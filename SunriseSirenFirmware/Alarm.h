#define ALARM_ACTIVITY_PERIOD 1000 // in ms

class SunriseSiren3000Alarm {
  private:
    unsigned long timePassedAt;
    bool timePassed;
    bool stopped; // false by default, true once button has been pressed

  public:
    String time;
    bool tripping; // whether the alarm is active/beeping
    bool activity; // every other second or so, whether the LEDs and buzzer should flash/beep

    bool update(String now) {
      bool justTripped = false;

      if (now == this->time && !this->timePassed) {
        this->timePassed = true;
        this->timePassedAt = millis();
        justTripped = true;
      }

      unsigned long trippingSince = millis() - this->timePassedAt;
      this->tripping = (this->timePassed && !this->stopped);
      this->activity = (this->tripping && trippingSince % ALARM_ACTIVITY_PERIOD < ALARM_ACTIVITY_PERIOD / 2);

      return justTripped;
    }

    void stop() {
      this->stopped = true;
    }

    void reset() {
      this->timePassed = false;
      this->tripping = false;
      this->activity = false;
      this->stopped = false;
    }
};
