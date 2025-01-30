#define LDR_PIN 35

#define HISTORY_FRAMES 4

class SunriseSiren3000LDR {
  private:
    int history[HISTORY_FRAMES]; // contains MODIFIED values! index 0 contains newest one

    void updateHistory(int newValue) {
      for (int i=HISTORY_FRAMES-1; i>=0; i--) {
        this->history[i] = (i > 0) ? this->history[i - 1] : newValue;
      }
    }

    double getAverage() {
      int sum = 0;
      for (int i=0; i<HISTORY_FRAMES; i++) sum += this->history[i];

      return sum / HISTORY_FRAMES;
    }

  public:
    unsigned int minValue;
    unsigned int maxValue;

    int rawValue;
    int modifiedValue;
    int averagedValue;

    void init() {
      pinMode(LDR_PIN, INPUT);
      for (int i=0; i<HISTORY_FRAMES; i++) this->history[i] = analogRead(LDR_PIN);
    }

    void update() {
      this->rawValue = analogRead(LDR_PIN);
      // old: 10-2200
      int constrainedValue = constrain(this->rawValue, this->minValue, this->maxValue);
      int mappedValue = map(constrainedValue, this->maxValue, this->minValue, 0, MAX_BRIGHTNESS) / 4 * 4;
      this->modifiedValue = max(mappedValue, 2);

      this->updateHistory(this->modifiedValue);
      this->averagedValue = this->getAverage();
    }
};
