#ifndef PROFILER_INCLUDED
#define PROFILER_INCLUDED

enum PF {
    PF_PIXELS,
    PF_EUCLIDIAN,
    PF_EUCLIDIAN_MUTATE,
    PF_HARMONY,
    PF_BPM,
    PF_ENVELOPES,
    PF_MIDI_OUT,
    PF_SCREEN,
    PF_INTEREST,
    PF_MAX
};

class profiler {

public:
    unsigned long time[PF::PF_MAX];
    unsigned long count[PF::PF_MAX];

    void reset() {
        for (int i = 0 ; i < PF::PF_MAX; i ++) {
           time[i] = 0;
           count[i] = 0;
        }
    }
    void l(int proftype, unsigned long length) {
        time[proftype] += length;
        count[proftype]++;
    }
    void output() {
        Serial.printf("profiler:[ ");
        Serial.printf("PIX:%i@%i | ", count[PF::PF_PIXELS], time[PF::PF_PIXELS]); 
        Serial.printf("EUC:%i@%i | ", count[PF::PF_EUCLIDIAN], time[PF::PF_EUCLIDIAN]); 
        Serial.printf("MUT:%i@%i | ", count[PF::PF_EUCLIDIAN_MUTATE], time[PF::PF_EUCLIDIAN_MUTATE]); 
        Serial.printf("HAR:%i@%i | ", count[PF::PF_HARMONY], time[PF::PF_HARMONY]); 
        Serial.printf("BPM:%i@%i | ", count[PF::PF_BPM], time[PF::PF_BPM]); 
        Serial.printf("ENV:%i@%i | ", count[PF::PF_ENVELOPES], time[PF::PF_ENVELOPES]); 
        Serial.printf("MDO:%i@%i | ", count[PF::PF_MIDI_OUT], time[PF::PF_MIDI_OUT]); 
        Serial.printf("SCR:%i@%i | ", count[PF::PF_SCREEN], time[PF_SCREEN]);
        Serial.printf("INT:%i@%i | ", count[PF::PF_INTEREST], time[PF::PF_INTEREST]); 
        Serial.printf(" ]\n");
    }
};

profiler pf = profiler();

#endif