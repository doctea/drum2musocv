#include "SinTables.h"

void initialise_envelopes() {
  // set up the default envelope states
  /*envelopes[ENV_SPLASH].attack_length = PPQN / 2;
  envelopes[ENV_SPLASH].decay_length = 0;
  envelopes[ENV_SPLASH].sustain_ratio = 1.0f;
  envelopes[ENV_SPLASH].release_length = PPQN * 3;*/
  
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
    MIDIOUT.sendControlChange(envelopes[i].midi_cc, (byte)0, (byte)1);
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
    } else if (number==LFO_SYNC_RATIO_HOLD_AND_DECAY-1) {
      envelopes[env_num].lfo_sync_ratio_hold_and_decay = value;
    } else if (number==LFO_SYNC_RATIO_SUSTAIN_AND_RELEASE-1) {
      envelopes[env_num].lfo_sync_ratio_sustain_and_release = value;
    }
    return true;
  }
  if (number==0x7B) {// || // intercept 'all notes off', 
        // TODO: have i commented out the wrong lines here? ^^^
        kill_envelopes();
        return true;
  } else if (number==0x07) {
      //number==0x65 || // RPN MSB
      //number==0x07*/) { // intercept 'volume' messages ..  this is the fucker interfering -- used for overall volume control, so DAW sends this, interferring with our control of the CC!
      //TODO: do i need to also ignore the others (1,7,11,71,74)?
      //TODO: or... use them as offsets so can modulate...?
      return true;
  }  
  return false;
}

// received a message that the state of the envelope should change (note on/note off etc)
void update_envelope (byte env_num, byte velocity, bool state) {
  unsigned long now = bpm_clock(); //clock_millis(); 
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
void process_envelopes(unsigned long now) {
  static unsigned long last_processed = 0;
  if (now==last_processed) return;
  for (byte i = 0 ; i < NUM_ENVELOPES ; i++) {
    process_envelope(i, now);
  }
  last_processed = now;
}

// process an envelope (ie update its stage and send updated CC to the midimuso if appropriate)
void process_envelope(byte i, unsigned long now) {
    //if (envelopes[i].stage!=OFF) {
    //if (envelopes[i].last_sent_at==0 || abs(now - envelopes[i].last_sent_at)>=CONFIG_THROTTLE_MS) {
    unsigned long elapsed = now - envelopes[i].stage_triggered_at;
    unsigned long real_elapsed = elapsed;
    // elapsed is currently the number of REAL ticks that have passed
    //elapsed *= PPQN;
    //elapsed *= ((1.0+(float)cc_value_sync_modifier)*0.75);
    //elapsed = ((1.0+(float)cc_value_sync_modifier)*0.75)/6;
    elapsed *= (((1.0f+(float)cc_value_sync_modifier)) / 0.75); // * PPQN);
    byte lvl = envelopes[i].stage_start_level;
    //NOISY_DEBUG(100, 30);
    //NUMBER_DEBUG(13, envelopes[i].stage, elapsed/16); //lvl);

    byte s = envelopes[i].stage ;

    if (s>0) {
      Serial.printf("process_envelope(%i, %u) in stage %i: sync'd elapsed is %u, ", i, now, s, elapsed);
      Serial.printf("real elapsed is %u, ", real_elapsed);
      Serial.printf("cc_value_sync_modifier is %u\r\n", cc_value_sync_modifier);
    }
    
    
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
          lvl = (byte) ((float)envelopes[i].velocity * ((float)elapsed / (0.1+(float)envelopes[i].attack_length )));
          
        if (elapsed >= envelopes[i].attack_length) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          envelopes[i].stage++; // = HOLD;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
        }
    } else if (s==HOLD) {
        lvl = envelopes[i].velocity; //stage_start_level;
        if (elapsed >= envelopes[i].hold_length || envelopes[i].hold_length == 0) {
          envelopes[i].stage++; // = DECAY;
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
        }
    } else if (s==DECAY) {
        //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);
        // length of time to decay down to sustain_level
        float f_sustain_level = SUSTAIN_MINIMUM + (envelopes[i].sustain_ratio * (float)envelopes[i].stage_start_level);
        float f_original_level = envelopes[i].stage_start_level;

        if (envelopes[i].decay_length>0) {
          float decay_position = ((float)elapsed / (float)(0.1+envelopes[i].decay_length));
  
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
          Serial.printf("Leaving SUSTAIN stage with lvl at %i\r\n", lvl);
          envelopes[i].stage++; // = RELEASE;
        }
    } else if (s==RELEASE) {
        if (envelopes[i].sustain_ratio==0.0f) {
          envelopes[i].stage_start_level = envelopes[i].velocity;
        }

        // the length of time to decay down to 0
        // immediately jump here if note off during any other stage (than OFF)
        if (envelopes[i].release_length>0) {
          //float eR = (float)elapsed / (float)(0.1+envelopes[i].release_length); 
          float eR = (float)elapsed / (float)(envelopes[i].release_length); 
          eR = constrain(eR, 0.0d, 1.0d);
  
          //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);
          Serial.printf("in RELEASE stage, eR is %3.3f, lvl is %i ....", eR, lvl);
          lvl = (byte)((float)envelopes[i].stage_start_level * (1.0f-eR));
          Serial.printf(".... lvl changed to %i\r\n", lvl);
        } else {
          lvl = 0;
        }
  
        if (elapsed > envelopes[i].release_length || envelopes[i].release_length==0) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage = OFF;
          Serial.printf("Leaving RELEASE stage with lvl at %i\r\n", lvl);
        } else {
          Serial.printf("RELEASE not finished because %u is less than %i?\r\n", elapsed, envelopes[i].release_length);
        }
    } else if (s==OFF) {  // may have stopped or something so mute
        lvl = 0; //64;
    }

    // if lfo_sync_ratio is >=16 for this envelope then apply lfo modulation to the level
    // TODO: make this actually more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage
    if (envelopes[i].stage!=OFF) {  // this is where we would enable them for constant LFO i think?
      // modulate the level
      //lvl = (lvl*(0.5+isin(elapsed * ((envelopes[i].lfo_sync_ratio / 16) * PPQN)))); 
      //lvl = (lvl * (0.5 + isin(elapsed * (((envelopes[i].lfo_sync_ratio) / 16 ))))); // * PPQN)))); ///((float)(cc_value_sync_modifier^2)/127.0))));  // TODO: find good parameters to use here, cc to adjust the modulation level etc

      
      int sync = (envelopes[i].stage==DECAY || envelopes[i].stage==HOLD) 
                    ?
                    envelopes[i].lfo_sync_ratio_hold_and_decay
                    :
                  (envelopes[i].stage==SUSTAIN || envelopes[i].stage==RELEASE)
                    ?
                    envelopes[i].lfo_sync_ratio_sustain_and_release : 
                   -1;
                    
      //NUMBER_DEBUG(12, 0, 127 * isin(elapsed
      if (sync>=0) {
         lvl = constrain(
          //((127-lvl) * (0.5+isin( (envelopes[i].lfo_sync_ratio/PPQN) * elapsed))), 
          lvl + ((lvl/4.0) * isin( PI*(sync/PPQN) * elapsed)),
          0,
          127
        );
        Serial.printf("sync of %i resulted in lvl %i\r\n", sync, lvl);
      }
    }

    envelopes[i].actual_level = lvl;
    
    if (envelopes[i].last_sent_lvl != lvl) {  // only send a new value if its different to the last value sent for this envelope
      //if (envelopes[i].stage==OFF) lvl = 0;   // force level to 0 if the envelope is meant to be OFF
      //NUMBER_DEBUG(12, envelopes[i].stage, lvl);
      //NUMBER_DEBUG(3, envelopes[i].stage, elapsed/16);
      /*static int cc = 0;
      Serial.printf("sending lvl %i to envelope %i on midi_cc %i!\r\n", lvl, i, cc); //envelopes[i].midi_cc);
      cc++;
      cc%=127;*/
      //MIDIOUT.sendControlChange(cc, lvl, 1); // send message to midimuso
      midi_send_envelope_level(i, lvl);
      
      envelopes[i].last_sent_at = now;
      envelopes[i].last_sent_lvl = lvl;
    }

}
