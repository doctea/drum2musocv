#include "SinTables.h"

#define SUSTAIN_MINIMUM 32
#define ENV_MAX_ATTACK 32
#define ENV_MAX_HOLD   32
#define ENV_MAX_DECAY  32
#define ENV_MAX_RELEASE 64

//#define TEST_LFOS

enum stage : byte {
  OFF = 0,
  //DELAY,  // time
  ATTACK,
  HOLD, // time
  DECAY,
  SUSTAIN,
  RELEASE,
  //END = 0
  LFO_SYNC_RATIO
};
// above enums also used as the envelope CC offsets

//#define LFO_SYNC_RATIO (RELEASE+1)

typedef struct envelope_state {
//#ifndef TEST_LFOS
  byte stage = OFF;
/*#else
  byte stage = LFO_SYNC_RATIO;
#endif*/

  byte velocity;         // triggered velocity
  byte actual_level;          // right now, the level
  byte stage_start_level;     // level at start of current stage

  // TODO: int delay_length = 5;  //  D - delay before atack starts
  unsigned int  attack_length = 0;     //  A - attack  - length of stage
  unsigned int  hold_length = PPQN * 2; //48;  //  H - hold    - length to hold at end of attack before decay
  unsigned int  decay_length = PPQN * 4; //384; //512;     //  D - decay   - length of stage
  float         sustain_ratio = 0.90f;  //  S - sustain - level to drop to after decay phase
  unsigned int  release_length = PPQN * 16; //768;   //  R - release - length (time to drop to 0)

  byte lfo_sync_ratio = 0;

  unsigned long stage_triggered_at = 0;
  unsigned long triggered_at = 0; 
  unsigned long last_sent_at = 0;

  byte midi_cc;

  byte last_sent_lvl;
};

#define ENV_CC_SPAN   8   // how many CCs to reserve per-envelope
#define ENV_CC_START  64  // what number CC the envelope controls begin at

envelope_state envelopes[NUM_ENVELOPES];

void initialise_envelopes() {
  // set up the default envelope states
  envelopes[ENV_SPLASH].attack_length = PPQN / 2;
  envelopes[ENV_SPLASH].decay_length = 0;
  envelopes[ENV_SPLASH].sustain_ratio = 1.0f;
  envelopes[ENV_SPLASH].release_length = PPQN * 3;
  
  envelopes[ENV_CRASH].midi_cc      = MUSO_CC_CV_2;
  envelopes[ENV_SPLASH].midi_cc     = MUSO_CC_CV_3;
  envelopes[ENV_WOBBLY].midi_cc     = MUSO_CC_CV_4;
  envelopes[ENV_RIDE_BELL].midi_cc  = MUSO_CC_CV_1;
  envelopes[ENV_RIDE_CYMBAL].midi_cc       = MUSO_CC_CV_5;
}

void kill_envelopes() {
  for (byte i = 0 ; i < NUM_ENVELOPES; i++) {
    envelopes[i].stage = OFF;
    envelopes[i].stage_start_level = (byte)0;
    MIDI.sendControlChange(envelopes[i].midi_cc, (byte)0, (byte)1);
  }
}

// change to an envelope setting
bool handle_envelope_ccs(byte channel, byte number, byte value) {
  //NOISY_DEBUG(1000, number);

  if (number>=ENV_CC_START && number <= ENV_CC_START + (ENV_CC_SPAN*NUM_ENVELOPES)) {
    number -= ENV_CC_START;
    int env_num = number / ENV_CC_SPAN; // which envelope are we dealing with?
    number = number % ENV_CC_SPAN;      // which control are we dealing with?
    //NOISY_DEBUG(env_num*100 , number);
    //NUMBER_DEBUG(6, env_num, number);
    // TODO: switch() would be better, but wasted too much time with weird problem doing it that way so this'll do for now
    if (number==ATTACK-1) {
      envelopes[env_num].attack_length  = (PPQN * ENV_MAX_ATTACK) * ((float)value/127.0f);
    } else if (number==HOLD-1) {
      envelopes[env_num].hold_length    = (PPQN * ENV_MAX_HOLD) * ((float)value/127.0f);
    } else if (number==DECAY-1) {
      envelopes[env_num].decay_length   = (PPQN * ENV_MAX_DECAY) * ((float)value/127.0f);
    } else if (number==SUSTAIN-1) {
      //envelopes[env_num].sustain_ratio = 1.0f; //SUSTAIN_RATIO_MINIMUM;// + ((float)value/127.0f) * (SUSTAIN_RATIO_MINIMUM); ///127.0f));
      //envelopes[env_num].sustain_ratio = (log(((float)value))*127.0f)/127.0f; ///127.0f));
      //envelopes[env_num].sustain_ratio = 0.5 + ((value/127.0f)/2.0);
      //envelopes[env_num].sustain_ratio = (64 + (value/2))/127.0f;
      envelopes[env_num].sustain_ratio = (((float)value/127.0f) * (float)(128-SUSTAIN_MINIMUM)) / 127.0f;   // converted to range 0-96 so can use minimum
    } else if (number==RELEASE-1) {
      envelopes[env_num].release_length = (PPQN * ENV_MAX_RELEASE) * ((float)value/127.0f);
    } else if (number==LFO_SYNC_RATIO-1) {
      envelopes[env_num].lfo_sync_ratio = value;
    }
    return true;
  }
  if (/*number==0x7B || // intercept 'all notes off', 
      number==0x65 || // RPN MSB*/
      number==0x07) { // intercept 'volume' messages ..  this is the fucker interfering -- used for overall volume control, so DAW sends this, interferring with our control of the CC!
            // TODO: have i commented out the wrong lines here? ^^^
        kill_envelopes();
        return true;
  }
  
  return false;
}

// received a message that the state of the envelope should change (note on/note off etc)
void update_envelope (byte env_num, byte velocity, bool state) {
  unsigned long now = clock_millis(); 
  if (state == true) { //&& envelopes[env_num].stage==OFF) {  // envelope told to be in 'on' state by note on
    envelopes[env_num].velocity = velocity;
    envelopes[env_num].actual_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
    envelopes[env_num].stage_start_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
    envelopes[env_num].stage = ATTACK;
    envelopes[env_num].triggered_at = now;
    envelopes[env_num].stage_triggered_at = now;
    envelopes[env_num].last_sent_at = 0;  // trigger immediate sending

    //NUMBER_DEBUG(7, envelopes[env_num].stage, envelopes[env_num].attack_length);
  } else if (state == false && envelopes[env_num].stage != OFF) { // envelope told to be in 'off' state by note off
    // note ended - fastforward to next envelope stage...?
    // if attack/decay/sustain, go straight to release at the current volume...?
    switch (envelopes[env_num].stage) {
      case RELEASE:
        // received note off while already releasing -- cut note short
        envelopes[env_num].stage_start_level = 0; 
        envelopes[env_num].stage = OFF;
        envelopes[env_num].stage_triggered_at = now;
        return;
      case OFF:
        // don't do anything if we're in this stage and we receive note off, since we're already meant to be stopping by now
        /*NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);*/
        return;

      // if in any other stage, jump straight to RELEASE stage at current volume
      case ATTACK:
      case HOLD:
        // TODO: continue to HOLD , but leap straight to RELEASE when reached end?
      case DECAY:
      case SUSTAIN:
      default:
        //NOISY_DEBUG(500, 2);
        //NUMBER_DEBUG(13, 13, 13);

        envelopes[env_num].stage = RELEASE;
        envelopes[env_num].stage_start_level = envelopes[env_num].actual_level;
        envelopes[env_num].stage_triggered_at = now;
        envelopes[env_num].last_sent_at = 0;  // trigger immediate sending
        break;
    }
  }
}

// process all the envelopes
void process_envelopes(unsigned long now, unsigned long delta) {
  for (byte i = 0 ; i < NUM_ENVELOPES ; i++) {
    process_envelope(i, now, delta);
  }
}

// process an envelope (ie update its stage and send updated CC to the midimuso if appropriate)
void process_envelope(byte i, unsigned long now, unsigned long delta) {
    //if (envelopes[i].stage!=OFF) {
    //if (envelopes[i].last_sent_at==0 || abs(now - envelopes[i].last_sent_at)>=CONFIG_THROTTLE_MS) {
    unsigned long elapsed = now - envelopes[i].stage_triggered_at;
    byte lvl = envelopes[i].stage_start_level;
    //NOISY_DEBUG(100, 30);
    //NUMBER_DEBUG(13, envelopes[i].stage, elapsed/16); //lvl);

    byte s = envelopes[i].stage ;
    // TODO: switch() would be nicer than if-else blocks, but ran into weird problems (like breakpoints never being hit) when approached it that way?!
    /*if (s==LFO_SYNC_RATIO) {
      lvl = random(0,127); //(int) (127.0 * (0.5+isin( (envelopes[i].lfo_sync_ratio/PPQN) * elapsed)));
    } else */
    if (s==ATTACK) {
        //NUMBER_DEBUG(8, envelopes[i].stage, elapsed/16);
        // length of time to ramp up to level
        //MIDI.sendControlChange(7, envelopes[i].level, 1);
        //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);

        if (envelopes[i].attack_length==0) 
          lvl = envelopes[i].velocity; // immediately start at desired velocity
        else
          lvl = (byte) ((float)envelopes[i].velocity * ((float)elapsed / (float)envelopes[i].attack_length));
          
        if (elapsed >= envelopes[i].attack_length) {
          NUMBER_DEBUG(9, envelopes[i].stage, 1);
          envelopes[i].stage++; // = HOLD;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
        }
    } else if (s==HOLD) {
        lvl = envelopes[i].stage_start_level;
        if (elapsed >= envelopes[i].hold_length || envelopes[i].hold_length == 0) {
          envelopes[i].stage++; // = DECAY;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
        }
    } else if (s==DECAY) {
        //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);
        // length of time to decay down to sustain_level
        float f_sustain_level = SUSTAIN_MINIMUM + envelopes[i].sustain_ratio * (float)envelopes[i].stage_start_level;
        float f_original_level = envelopes[i].stage_start_level;

        if (envelopes[i].decay_length>0) {
          float decay_position = ((float)elapsed / (float)(envelopes[i].decay_length));
  
          // we start at stage_start_level
          float diff = (f_original_level - (f_sustain_level));
          // and over decay_length time
          // we want to scale down to f_sustain_level at minimum
  
          lvl = f_original_level - (diff * decay_position);
        } else {
          // if there's no decay stage then set level to the sustain level
          lvl = f_sustain_level; 
        }
          
        if (elapsed >= envelopes[i].decay_length) {// || envelopes[i].decay_length==0 || lvl < f_sustain_level) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          //if (envelopes[i].decay_length==0) lvl = envelopes[i].stage_start_level;

          envelopes[i].stage++; // = SUSTAIN;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
        }
    } else if (s==SUSTAIN) {
        //NUMBER_DEBUG(8, envelopes[i].stage, elapsed/16); //envelopes[i].stage_start_level);
        //float sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].inital_level);
        // the volume level to remain at until the note is released
        byte sustain_level = envelopes[i].stage_start_level;
        //sustain_level = random(0, 127);

        lvl = (byte)(sustain_level);
        
        // go straight to RELEASE if sustain is zero
        if (envelopes[i].sustain_ratio==0.0f) {
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
          envelopes[i].stage++; // = RELEASE;
        }
    } else if (s==RELEASE) {
        // the length of time to decay down to 0
        // immediately jump here if note off during any other stage (than OFF)
        float eR = (float)elapsed / (float)envelopes[i].release_length; 

        //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);

        lvl = (byte)((float)envelopes[i].stage_start_level * (1.0f-eR));

        if (elapsed > envelopes[i].release_length || envelopes[i].release_length==0) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage = OFF;
        }
    } else if (s==OFF) {  // may have stopped or something so mute
        lvl = 0; //64;
    }

    // if lfo_sync_ratio is >=16 for this envelope then apply lfo modulation to the level
    // TODO: make this actually more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage
    if (envelopes[i].lfo_sync_ratio>=16 && envelopes[i].stage!=OFF) {
      // modulate the level
      //lvl = (lvl*(0.5+isin(elapsed * ((envelopes[i].lfo_sync_ratio / 16) * PPQN)))); 
      //lvl = (lvl * (0.5 + isin(elapsed * (((envelopes[i].lfo_sync_ratio) / 16 ))))); // * PPQN)))); ///((float)(cc_value_sync_modifier^2)/127.0))));  // TODO: find good parameters to use here, cc to adjust the modulation level etc

      //NUMBER_DEBUG(12, 0, 127 * isin(elapsed 
      lvl = constrain(
        //((127-lvl) * (0.5+isin( (envelopes[i].lfo_sync_ratio/PPQN) * elapsed))), 
        lvl + (32 * isin( PI*(envelopes[i].lfo_sync_ratio/PPQN) * elapsed)),
        0,
        127
      );
    }

    envelopes[i].actual_level = lvl;
    
    if (envelopes[i].last_sent_lvl != lvl) {  // only send a new value if its different to the last value sent for this envelope
      //if (envelopes[i].stage==OFF) lvl = 0;   // force level to 0 if the envelope is meant to be OFF
      //NUMBER_DEBUG(12, envelopes[i].stage, lvl);
      //NUMBER_DEBUG(3, envelopes[i].stage, elapsed/16);
      
      MIDI.sendControlChange(envelopes[i].midi_cc, lvl, 1); // send message to midimuso
      envelopes[i].last_sent_at = now;
      envelopes[i].last_sent_lvl = lvl;
    }

}
