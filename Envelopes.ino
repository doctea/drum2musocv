#include "SinTables.h"

#define SUSTAIN_MINIMUM 32
#define ENV_MAX_ATTACK 32
#define ENV_MAX_HOLD   32
#define ENV_MAX_DECAY  32
#define ENV_MAX_RELEASE 64

enum stage : byte {
  OFF = 0,
  //DELAY,  // time
  ATTACK,
  HOLD, // time
  DECAY,
  SUSTAIN,
  RELEASE,
  //END = 0
};
// above enums also used as the envelope CC offsets

#define LFO_SYNC_RATIO (RELEASE+1)

typedef struct envelope_state {
  byte stage = OFF;

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

#define ENV_CC_SPAN 8
#define ENV_CC_START 64

envelope_state envelopes[3];

void initialise_envelopes() {
  envelopes[ENV_SPLASH].attack_length = PPQN / 2; //12;
  envelopes[ENV_SPLASH].decay_length = 0;
  envelopes[ENV_SPLASH].sustain_ratio = 1.0f; //.3f;
  envelopes[ENV_SPLASH].release_length = PPQN * 3; //72;
  envelopes[ENV_SPLASH].midi_cc = 11; //release_length = 50;

  envelopes[ENV_CRASH].midi_cc = 7;

  envelopes[ENV_WOBBLY].midi_cc = 71;
}

void kill_envelopes() {
  for (byte i = 0 ; i < 3; i++) {
    envelopes[i].stage = OFF;
    envelopes[i].stage_start_level = (byte)0;
    MIDI.sendControlChange(envelopes[i].midi_cc, (byte)0, (byte)1);
  }
}

bool handle_envelope_ccs(byte channel, byte number, byte value) {
  //NOISY_DEBUG(1000, number);

  if (number>=ENV_CC_START && number <= ENV_CC_START + (ENV_CC_SPAN*NUM_ENVELOPES)) {
    //NOISY_DEBUG(100, number);
    number -= ENV_CC_START;
    int env_num = number / ENV_CC_SPAN;
    number = number % ENV_CC_SPAN;
    //NOISY_DEBUG(env_num*100 , number);
    NUMBER_DEBUG(6, env_num, number);
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
      envelopes[env_num].sustain_ratio = (((float)value/127.0f)*(float)(128-SUSTAIN_MINIMUM)) /127.0f;   // converted to range 0-96 so can use minimum
    } else if (number==RELEASE-1) {
      envelopes[env_num].release_length = (PPQN * ENV_MAX_RELEASE) * ((float)value/127.0f);
    } else if (number==LFO_SYNC_RATIO-1) {
      envelopes[env_num].lfo_sync_ratio = value;
    }
    return true;
  }
  if (number==0x7B || // intercept 'all notes off', 
      number==0x65 || // RPN MSB
      number==0x07) { // volume messages ..  this is the fucker !
    kill_envelopes();
    return true;
  }
  
  return false;
}

void update_envelope (byte env_num, byte velocity, bool state) {
  unsigned long now = clock_millis(); 
  if (state == true) { //&& envelopes[env_num].stage==OFF) {
    envelopes[env_num].velocity = velocity;
    envelopes[env_num].actual_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
    envelopes[env_num].stage_start_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
    envelopes[env_num].stage = ATTACK;
    envelopes[env_num].triggered_at = now;
    envelopes[env_num].stage_triggered_at = now;
    envelopes[env_num].last_sent_at = 0;  // trigger immediate sending

    //NUMBER_DEBUG(7, envelopes[env_num].stage, envelopes[env_num].attack_length);
  } else if (state == false && envelopes[env_num].stage != OFF) {
    // note ended - fastforward to next envelope stage...?
    // if attack/decay/sustain, go straight to release at the current volume...?
    switch (envelopes[env_num].stage) {
      // don't do anything if we're in this stage and we receive note off, since we're already meant to be stopping by now
      case RELEASE:
        envelopes[env_num].stage_start_level = 0; // received note off while releasing -- cut note short
        envelopes[env_num].stage = OFF;
        envelopes[env_num].stage_triggered_at = now;
        return;
      case OFF:
        /*NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);
        NUMBER_DEBUG(15, 15, 15);*/
        return;

      // if in any other stage, jump straight to RELEASE
      case ATTACK:
      case HOLD:
        // TODO: continue to HOLD , but leap straight to RELEASE when reached end
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

void process_envelopes(unsigned long now, unsigned long delta) {
  for (byte i = 0 ; i < 3 ; i++) {
    process_envelope(i, now, delta);
  }
}

void process_envelope(byte i, unsigned long now, unsigned long delta) {
    //if (envelopes[i].stage!=OFF) {
    //if (envelopes[i].last_sent_at==0 || abs(now - envelopes[i].last_sent_at)>=CONFIG_THROTTLE_MS) {
    unsigned long elapsed = now - envelopes[i].stage_triggered_at;
    byte lvl = envelopes[i].stage_start_level;
    //NOISY_DEBUG(100, 30);
    //NUMBER_DEBUG(13, envelopes[i].stage, elapsed/16); //lvl);

    byte s = envelopes[i].stage ;
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
        // TODO: decay should actually be aiming to reach sustain_ratio * level, not 0
        float f_sustain_level = SUSTAIN_MINIMUM + envelopes[i].sustain_ratio * (float)envelopes[i].stage_start_level;
        float f_original_level = envelopes[i].stage_start_level;

        if (envelopes[i].decay_length>0) {
          float decay_position = ((float)elapsed / (float)(envelopes[i].decay_length));
  
          /*float decay = (float) f_sustain_level +
                        ((f_original_level - f_sustain_level) *
                         decay_position
                        );
  
          //lvl = (byte)((float)envelopes[i].stage_start_level * decay);  // not right but cool effect?
          lvl = //f_sustain_level + decay
            (byte) decay * f_sustain_level;*/
  
          // we start at stage_start_level
          // so 
          //float diff = (f_original_level - f_sustain_level) * (1.0f-decay_position);
          float diff = (f_original_level - (f_sustain_level));
          // and over decay_length time
          // want to scale down to f_sustain_level at minimum
  
          //lvl = f_sustain_level + (diff - (decay_position * diff));*/
  
          //lvl = (byte) (f_sustain_level + (diff * decay_position));
          lvl = f_original_level - (diff *decay_position);
        } else {
          lvl = f_sustain_level; //envelopes[i].stage_start_level;
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

        // volume to remain at until note is released
        //float sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].inital_level);

        byte sustain_level = envelopes[i].stage_start_level;
        //sustain_level = random(0, 127);

        lvl = (byte)(sustain_level);
        
        // go straight to RELEASE if sustain is zero
        //if (envelopes[i].sustain_ratio==0.0f) {
        if (envelopes[i].sustain_ratio==0.0f) {
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
          envelopes[i].stage++; // = RELEASE;
        }
    } else if (s==RELEASE) {
        // length of time to decay down to 0
        // immediately jump here if note off during any other stage (than OFF)
        // this code is actually for RELEASE stage i think... real one should move on to SUSTAIN stage
        float eR = (float)elapsed / (float)envelopes[i].release_length; /// decay_length; // reversing this gives cool effect?
        //float f_sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].stage_start_level/127.0f);

        //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);

        lvl = (byte)((float)envelopes[i].stage_start_level * (1.0f-eR));

        if (elapsed > envelopes[i].release_length || envelopes[i].release_length==0) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          //lvl = 0;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage = OFF;
        }
        //break;
    } else if (s==OFF) {  // may have stopped or something so mute
        lvl = 0;
    }

    if (envelopes[i].lfo_sync_ratio>=16 && envelopes[i].stage!=OFF) {
      // modulate the level
      //lvl = (lvl*(0.5+isin(elapsed * ((envelopes[i].lfo_sync_ratio / 16) * PPQN)))); 
      //lvl = (lvl * (0.5 + isin(elapsed * (((envelopes[i].lfo_sync_ratio) / 16 ))))); // * PPQN)))); ///((float)(cc_value_sync_modifier^2)/127.0))));  // TODO: find good parameters to use here, cc to adjust the modulation level etc

      //NUMBER_DEBUG(12, 0, 127 * isin(elapsed 
      lvl = constrain(/*lvl + */((127-lvl) * (0.5+isin( (envelopes[i].lfo_sync_ratio/PPQN) * elapsed))), 0, 127);

    }

    envelopes[i].actual_level = lvl;
    if (envelopes[i].last_sent_lvl != lvl) {
      if (envelopes[i].stage==OFF) lvl = 0;
      NUMBER_DEBUG(12, envelopes[i].stage, lvl);

      //NUMBER_DEBUG(3, envelopes[i].stage, elapsed/16);
      MIDI.sendControlChange(envelopes[i].midi_cc, lvl, 1);
      envelopes[i].last_sent_at = now;
      envelopes[i].last_sent_lvl = lvl;
    }

}
