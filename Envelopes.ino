#include "SinTables.h"

#include "MidiOutput.hpp"



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
#if MUSO_MODE==MUSO_MODE_0B_AND_2A
  envelopes[ENV_PITCH_1].midi_cc    = MUSO_CC_CV_6;
  envelopes[ENV_PITCH_2].midi_cc    = MUSO_CC_CV_7;
  envelopes[ENV_PITCH_3].midi_cc    = MUSO_CC_CV_8;
  envelopes[ENV_PITCH_4].midi_cc    = MUSO_CC_CV_9;
  envelopes[ENV_PITCH_1].trigger_on = 18;
  envelopes[ENV_PITCH_2].trigger_on = 18;
  envelopes[ENV_PITCH_3].trigger_on = 19;
  envelopes[ENV_PITCH_4].trigger_on = 19;
#endif

}

void randomise_envelopes() {
  for (int i = 0 ; i < NUM_ENVELOPES ; i++) { // _EXTENDED if we want to randomise the extended envelopes too?
    //envelopes[i].trigger_on = random(0,4);
    envelopes[i].lfo_sync_ratio_hold_and_decay = random(0,127);
    envelopes[i].lfo_sync_ratio_sustain_and_release = random(0,127);
    envelopes[i].attack_length = random(0,127);
    envelopes[i].hold_length = random(0,127);
    envelopes[i].decay_length = random(0,127);
    envelopes[i].sustain_ratio = 1.0/(float)random(0,127);
  }
}

void kill_envelopes() {
  for (byte i = 0 ; i < NUM_ENVELOPES_EXTENDED; i++) {
    envelopes[i].stage = OFF;
    envelopes[i].stage_start_level = (byte)0;
    //MIDIOUT.sendControlChange(envelopes[i].midi_cc, (byte)0, (byte)1);
    midi_send_envelope_level(i, 0);
  }
}

// change to an envelope setting
bool handle_envelope_ccs(byte channel, byte number, byte value) {
  //NOISY_DEBUG(1000, number);
  int num_envelopes = 0;
  int env_offset = 0;
  
  if (channel==MIDI_CHANNEL_EXTENDED_ENVELOPES && MUSO_MODE==MUSO_MODE_0B_AND_2A) {
    num_envelopes = NUM_ENVELOPES_EXTENDED - NUM_ENVELOPES; //4;
    env_offset = NUM_ENVELOPES; //5;
  } else {
    num_envelopes = NUM_ENVELOPES;
    env_offset = 0;
  }

  if (number>=ENV_CC_START && number <= ENV_CC_START + (ENV_CC_SPAN*num_envelopes)) {
    number -= ENV_CC_START;
    int env_num = number / ENV_CC_SPAN; // which envelope are we dealing with?
    number = number % ENV_CC_SPAN;      // which control are we dealing with?
    env_num += env_offset;
    //NOISY_DEBUG(env_num*100 , number);
    //NUMBER_DEBUG(6, env_num, number);
    // TODO: switch() would be better, but wasted too much time with weird problem doing it that way so this'll do for now
    if (number==ATTACK-1) {
      envelopes[env_num].attack_length  = (ENV_MAX_ATTACK) * ((float)value/127.0f);
    } else if (number==HOLD-1) {
      envelopes[env_num].hold_length    = (ENV_MAX_HOLD) * ((float)value/127.0f);
    } else if (number==DECAY-1) {
      envelopes[env_num].decay_length   = (ENV_MAX_DECAY) * ((float)value/127.0f);
    } else if (number==SUSTAIN-1) {
      envelopes[env_num].sustain_ratio = (((float)value/127.0f) * (float)(128-SUSTAIN_MINIMUM)) / 127.0f;   // converted to range 0-96 so can use minimum
    } else if (number==RELEASE-1) {
      envelopes[env_num].release_length = (ENV_MAX_RELEASE) * ((float)value/127.0f);
    } else if (number==LFO_SYNC_RATIO_HOLD_AND_DECAY-1) {
      envelopes[env_num].lfo_sync_ratio_hold_and_decay = constrain(1+value,1,128);
    } else if (number==LFO_SYNC_RATIO_SUSTAIN_AND_RELEASE-1) {
      envelopes[env_num].lfo_sync_ratio_sustain_and_release = constrain(1+value,1,128);
    } else if (number==ASSIGN_HARMONY_OUTPUT-1) {
      if (envelopes[env_num].trigger_on!=TRIGGER_CHANNEL_OFF)
        update_envelope(env_num, 0, false);
      envelopes[env_num].loop = bitRead(value,5);          // +32 = loop on/off
      envelopes[env_num].invert = bitRead(value,6);        // +64 = invert on/off
      envelopes[env_num].trigger_on = value & 0b00011111;  // mask off the high bits to get the trigger number
      //Serial.printf("setting envelope %i to trigger on %i, loop %i invert %i\n", env_num, envelopes[env_num].trigger_on, envelopes[env_num].loop, envelopes[env_num].invert );
    }
    return true;
  }

  return false;
}

void update_envelopes_for_trigger(int trigger, int velocity, bool state) {
  for (int i = 0 ; i < NUM_ENVELOPES_EXTENDED ; i++) {
    if (envelopes[i].trigger_on==trigger) {
      //Serial.printf("update_envelopes_for_trigger(trigger %i, env %i, state %i)\n", trigger, i, state);
      update_envelope(i, (byte)velocity, state);
    }
  }
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
  for (byte i = 0 ; i < NUM_ENVELOPES_EXTENDED ; i++) {
    process_envelope(i, now);
  }
  last_processed = now;
}

//#define DEBUG_ENVELOPES

// process an envelope (ie update its stage and send updated CC to the midimuso if appropriate)
void process_envelope(byte i, unsigned long now) {
  //unsigned long envelope_time = millis();
    //if (envelopes[i].stage!=OFF) {
    //if (envelopes[i].last_sent_at==0 || abs(now - envelopes[i].last_sent_at)>=CONFIG_THROTTLE_MS) {
    unsigned long elapsed = now - envelopes[i].stage_triggered_at;
    unsigned long real_elapsed = elapsed;    // elapsed is currently the number of REAL ticks that have passed

    byte lvl = envelopes[i].stage_start_level;
    byte s = envelopes[i].stage ;
    
    float ratio = (float)PPQN / (float)cc_value_sync_modifier;  // calculate ratio of real ticks : pseudoticks
    elapsed = (float)elapsed * ratio;   // convert real elapsed to pseudoelapsed
    
#ifdef DEBUG_ENVELOPES
    static byte last_stage;
    if (s>0 && last_stage!=s && envelopes[i].trigger_on_channel==TRIGGER_CHANNEL_LFO) {
      Serial.printf("process_envelope(%i, %u, trig %i) in stage %i: sync'd elapsed is %u, ", i, now, envelopes[i].trigger_on_channel, elapsed);
      Serial.printf("real elapsed is %u, lvl is %i, ", real_elapsed, envelopes[i].last_sent_lvl);
      Serial.printf("cc_value_sync_modifier is %u\r\n", cc_value_sync_modifier);

      /*Serial.printf("Ratio is PPQN %u / %u = %3.3f, so therefore", PPQN, cc_value_sync_modifier, ratio);
      Serial.printf("converting real elapsed %u to ", real_elapsed);
      Serial.printf("%u\r\n", elapsed);*/
    } 
    last_stage = s;
    
#endif
    
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
          lvl = (byte) ((float)envelopes[i].velocity * ((float)elapsed / ((float)envelopes[i].attack_length )));
          
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
        
        // go straight to RELEASE if sustain is zero or we're in lfo mode
        if (envelopes[i].sustain_ratio==0.0f || envelopes[i].loop) { //trigger_on>=TRIGGER_CHANNEL_LFO) {
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage_start_level = lvl;
          OUT_printf("Leaving SUSTAIN stage with lvl at %i\r\n", lvl);
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
          //float eR = (float)elapsed / (float)(envelopes[i].release_length); 
          float eR = (float)elapsed / (float)(envelopes[i].release_length); 
          eR = constrain(eR, 0.0d, 1.0d);
  
          //NUMBER_DEBUG(8, envelopes[i].stage, envelopes[i].stage_start_level);
          //Serial.printf("in RELEASE stage, release_length is %u, elapsed is %u, eR is %3.3f, lvl is %i ....", envelopes[i].release_length, elapsed, eR, lvl);
          lvl = (byte)((float)envelopes[i].stage_start_level * (1.0f-eR));
          //Serial.printf(".... lvl changed to %i\r\n", lvl);
        } else {
          lvl = 0;
        }
  
        if (elapsed > envelopes[i].release_length || envelopes[i].release_length==0) {
          //NUMBER_DEBUG(9, envelopes[i].stage, 1);
          envelopes[i].stage_triggered_at = now;
          envelopes[i].stage = OFF;
          //Serial.printf("Leaving RELEASE stage with lvl at %i\r\n", lvl);
        } /*else {
          Serial.printf("RELEASE not finished because %u is less than %i?\r\n", elapsed, envelopes[i].release_length);
        }*/
    } else if (s==OFF) {  // may have stopped or something so mute
        lvl = 0; //64;
    }

    // if lfo_sync_ratio is >=16 for this envelope then apply lfo modulation to the level
    // TODO: make this actually more useful... set upper/lower limits to modulation, elapsed-based scaling of modulation, only modulate during eg RELEASE stage
    if (envelopes[i].stage!=OFF) {  // this is where we would enable them for constant LFO i think?
      // modulate the level
      // TODO: FIX THIS SO RATIO WORKS !
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
        
         sync *= 4; // multiply sync value so that it gives us possibility to modulate much faster

         float mod_amp = (float)lvl/4.0f; //32.0; // modulation amplitude is a quarter of the current level

         float mod_result = mod_amp * isin((float)elapsed * PPQN * ((float)sync/127.0f));
         //Serial.printf("mod_result is %3.1f, elapsed is %i, sync is %i\r\n", mod_result, elapsed, sync);
        
         lvl = constrain(
          lvl + mod_result,
          0,
          127
        );
        //Serial.printf("sync of %i resulted in lvl %i\r\n", sync, lvl);
      } 
    } else {
        // envelope is stopped - restart it if in lfo mode!
        if (envelopes[i].loop) { //trigger_on>=TRIGGER_CHANNEL_LFO) {
          OUT_printf("envelope %i is stopped, restarting\n", i);
          update_envelope(i, 127, true);
        }
    }

    /*if (envelopes[i].trigger_on==TRIGGER_CHANNEL_LFO_MODULATED || envelopes[i].trigger_on==TRIGGER_CHANNEL_LFO_MODULATED_AND_INVERTED ) {
      int modulating_envelope = (i-1==-1) ? NUM_ENVELOPES_EXTENDED-1 : i-1;
      lvl = ((float)lvl) * ((float)envelopes[modulating_envelope].last_sent_lvl/127);
    }*/
    if (envelopes[i].invert) {
      lvl = 127-lvl;
    }


    envelopes[i].actual_level = lvl;
    
    if (envelopes[i].last_sent_lvl != lvl) {  // only send a new value if its different to the last value sent for this envelope
      //if (envelopes[i].stage==OFF) lvl = 0;   // force level to 0 if the envelope is meant to be OFF

      //Serial.printf("sending value %i for envelope %i\n", lvl, i);
      midi_send_envelope_level(i, lvl); // send message to midimuso
      
      envelopes[i].last_sent_at = now;
      envelopes[i].last_sent_lvl = lvl;
    }

  //Serial.printf("envelope processed in %ims\n", millis()-envelope_time);

}

// trigger any envelopes that have been told to respond to given channel
void fire_envelope_for_channel(int channel, int velocity) {
  if (channel==0) return;
  //Serial.printf("checking fire_envelope_for_channel %i\r\n", channel);

  for (int i = 0 ; i < NUM_ENVELOPES_EXTENDED ; i++) {
    if (envelopes[i].trigger_on==channel) {
      //Serial.printf("got fire_for_envelope %i on channel %i\r\n", i, channel);
      update_envelope(i, velocity, true);
    }
  }
}

void douse_envelope_for_channel(int channel, int velocity) {
  if (channel==0) return;

  for (int i = 0 ; i < NUM_ENVELOPES_EXTENDED ; i++) {
    if (envelopes[i].trigger_on==channel) {
      //Serial.printf("got douse_for_envelope %i on channel %i\r\n", i, channel);
      update_envelope(i, velocity, false);
    }
  }
}

