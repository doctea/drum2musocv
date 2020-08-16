enum stage {
  OFF = 0, 
  //DELAY,  // time
  ATTACK = 1,
  //HOLD, // time 
  DECAY = 2, 
  SUSTAIN = 3 , 
  RELEASE = 4,
  END = 0
};

typedef struct envelope_state {
  byte stage = OFF;
  
  byte initial_level; // initial level during attack (todo: attack ramp)
  byte actual_level;          // right now, the level
  byte stage_start_level;     // level at start of current stage

  // int delay_length = 5;    //  D - delay before atack starts 
  int attack_length = 100;    //  A - attack  - length of stage
  // int hold_length = 50;    //  H - hold    - length to hold at end of attack before decay
  int decay_length = 500;     //  D - decay   - length of stage
  float sustain_ratio = 0.90f;//  S - sustain - level to drop to after decay phase
  int release_length = 1000;   //  R - release - length (time to drop to 0)

  unsigned long stage_triggered_at;
  unsigned long triggered_at;
  unsigned long last_sent_at;
};


envelope_state envelopes[2];

void update_envelope (byte env_num, byte velocity, bool state) {
  unsigned long now = millis();
  if (state==true) {
    envelopes[env_num].initial_level = velocity;
    envelopes[env_num].actual_level = velocity; // TODO: start this at 0 so it can ramp / offset level feature
    //envelopes[env_num].sustain_ratio = 0.90f * ((float)velocity/127.0f);  // todo: accent
    envelopes[env_num].stage = ATTACK;
    envelopes[env_num].triggered_at = now;
    envelopes[env_num].stage_triggered_at = now;
    envelopes[env_num].last_sent_at = 0;  // trigger immediate sending
  } else {  
    // note ended - fastforward to next envelope stage...?
    // if attack/decay/sustain, go straight to release at the current volume...?
    switch (envelopes[env_num].stage) {
      /*case ATTACK:
        envelopes[env_num].stage = DECAY;
        envelopes[env_num].stage_triggered_at = now;
        envelopes[env_num].last_sent_at = 0;  // trigger immediate sending
        break;*/
      case OFF:
      case RELEASE:
        return;
      default:
        /*for (int i = 0 ; i < 2000 ; i++) {
          MIDI.sendControlChange(7, i%127, 1);
        }*/

        //NOISY_DEBUG(500, 2);
        
        envelopes[env_num].stage = RELEASE;
        envelopes[env_num].stage_start_level = envelopes[env_num].actual_level;
        envelopes[env_num].stage_triggered_at = now;
        envelopes[env_num].last_sent_at = 0;  // trigger immediate sending
        break;
    }
  }
}

void process_envelopes(unsigned long now, unsigned long delta) {
  for (int i = 0 ; i < 1 ; i++) {
      if (envelopes[i].stage!=OFF) {
        if (envelopes[i].last_sent_at==0 || abs(now - envelopes[i].last_sent_at)>=CONFIG_THROTTLE_MS) {
          long elapsed = now - envelopes[i].stage_triggered_at;
          byte lvl;
          //NOISY_DEBUG(100, 30);
          switch(envelopes[i].stage) {
            case ATTACK:
              //NOISY_DEBUG(500, 60);
              //NOISY_DEBUG(100, 15);
              // length of time to ramp up to level
              //MIDI.sendControlChange(7, envelopes[i].level, 1);
              lvl = envelopes[i].initial_level;
              if (elapsed >= envelopes[i].attack_length) {
                envelopes[i].stage = DECAY;
                envelopes[i].stage_triggered_at = now;
                envelopes[i].stage_start_level = lvl;
              }
              break;
            case DECAY:
              /*NOISY_DEBUG(100, 1);
              NOISY_DEBUG(100, 2);
              NOISY_DEBUG(250, 1);*/
              // length of time to decay down to sustain_level
              // TODO: decay should actually be aiming to reach sustain_ratio * level, not 0
              float f_sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].stage_start_level/127.0f);
              float f_original_level = envelopes[i].stage_start_level / 127.0f;

              float decay = f_sustain_level + 
                            (f_original_level-f_sustain_level * 
                              (1.0f - (float)elapsed / (float)envelopes[i].decay_length)
                            );

              // this code is actually for RELEASE stage i think... real one should move on to SUSTAIN stage
              //float decay = 1.0f - (float)elapsed / (float)envelopes[i].decay_length; /// decay_length; // reversing this gives cool effect?
              //MIDI.sendControlChange(7, (byte)((float)envelopes[i].level * decay), 1); //decay_1 * 127, 1);
              //lvl = (byte)((float)envelopes[i].stage_start_level * decay);
              lvl = (byte) decay * 127.0f;
              if (elapsed >= envelopes[i].decay_length) {
                //NOISY_DEBUG(500, 10);
                envelopes[i].stage = SUSTAIN;
                envelopes[i].stage_triggered_at = now;
                envelopes[i].stage_start_level = lvl;
              }
              break;
            case SUSTAIN:
              NOISY_DEBUG(100, 3);
              NOISY_DEBUG(100, 5);
              NOISY_DEBUG(250, 1);
              //NOISY_DEBUG(100, 10);
            
              // volume to remain at until note is released
              float sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].stage_start_level/127.0f);
              //MIDI.sendControlChange(7, (byte)(sustain_level*127.0f), 1); //decay_1 * 127, 1);

              //NOISY_DEBUG(500, 120);
              
              lvl = (byte)(sustain_level*127.0f);
              break;
            case RELEASE:
              // length of time to decay down to 0
              // immediately jump here if note off during any other stage (than OFF)
              // this code is actually for RELEASE stage i think... real one should move on to SUSTAIN stage
              float release = (float)elapsed / (float)envelopes[i].release_length; /// decay_length; // reversing this gives cool effect?
              //float f_sustain_level = envelopes[i].sustain_ratio * ((float)envelopes[i].stage_start_level/127.0f);

              //NOISY_DEBUG(5000, 1);
                            
              //MIDI.sendControlChange(7, (byte)(f_sustain_level * decay), 1); //decay_1 * 127, 1);
              lvl = (byte)((float)envelopes[i].stage_start_level * release);
              //lvl = (byte)random(0,127);
              if (elapsed >= envelopes[i].release_length) {
                //MIDI.sendControlChange(7, 0, 1);
                lvl = 0;
                envelopes[i].stage_triggered_at = now;
                envelopes[i].stage = OFF;
              }
              break;
            case OFF: // wont be reached?
              lvl = 0;
              break;
            default:
              return;
          }
          envelopes[i].actual_level = lvl;
          MIDI.sendControlChange(7, lvl, 1);
          envelopes[i].last_sent_at = now; //millis(); 
        }
      }
  }
}
