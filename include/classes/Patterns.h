#include <Arduino.h>

#define NOTE_OFF -1
#define DEFAULT_VELOCITY 127

class Pattern {
    public:

    int pattern_length_steps = 16;
    int ticks_per_step = 12;            // todo: calculate this from desired pattern length in bars, PPQN and steps

    // todo: ability to pass in step, offset, and bar number, like we have for the current euclidian...?
    //          or, tbf, we can derive this from the 'tick'
    virtual bool query_note_on_for_tick(unsigned int tick) = 0;
    virtual bool query_note_off_tick(unsigned int tick) = 0;

    virtual void set_event_for_tick(unsigned int tick, short note = 0, short velocity = 127, short channel = 0) = 0;

    virtual void set_pattern_length(int pattern_length) {
        this->pattern_length_steps = pattern_length;
    }
};

class SimplePattern : public Pattern {
    public:

    struct event {
        short note = NOTE_OFF;
        short velocity = DEFAULT_VELOCITY;
        short channel = 0;
    };

    event *events = nullptr;

    SimplePattern() : Pattern() {
        this->events = (event*)calloc(sizeof(event), pattern_length_steps);
    }

    virtual unsigned int get_step_for_tick(unsigned int tick) {
        return (tick / this->ticks_per_step) % pattern_length_steps;
    }

    virtual void set_event_for_tick(unsigned int tick, short note = 0, short velocity = DEFAULT_VELOCITY, short channel = 0) override {
        short step = get_step_for_tick(tick);
        this->events[step].note = note;
        this->events[step].velocity = velocity;
        this->events[step].channel = channel;
    }
    virtual void unset_event_for_tick(unsigned int tick) {
        short step = get_step_for_tick(tick);
        this->events[step].velocity = 0;
        //this->events[step].note = NOTE_OFF;
        //this->events[step].channel = channel;
    }

    virtual bool query_note_on_for_tick(unsigned int tick) override {
        short step = get_step_for_tick(tick);
        return (this->events[step].velocity>0);
    }
    virtual bool query_note_off_tick(unsigned int tick) override {
        short step = get_step_for_tick(tick);
        return (this->events[step].velocity==0);
    }
};
