#define BUTTON_PIN 26

#define HOLD_THRESHOLD 750 // in ms

class SunriseSiren3000Button {
  private:
    bool lastState = false;
    bool releaseActionBlocked = false;

    unsigned long pressedAt = 0;

  public:
    bool state = false;
    bool released = false;
    bool held = false;

    void init() {
      pinMode(BUTTON_PIN, INPUT_PULLUP);

      this->lastState = digitalRead(BUTTON_PIN);
    }

    void update() {
      this->released = false;
      this->held = false;
      this->state = digitalRead(BUTTON_PIN);

      if (!this->state && this->lastState) {
        this->pressedAt = millis();
        this->releaseActionBlocked = false;
      } else if (this->state && !this->lastState && !releaseActionBlocked) {
        this->released = true;
      }

      unsigned long pressedFor = (this->state) ? 0 : millis() - this->pressedAt;
      if (pressedFor >= HOLD_THRESHOLD && !this->releaseActionBlocked) {
        this->held = true;
        this->releaseActionBlocked = true;
      }

      this->lastState = this->state;
    }
};
