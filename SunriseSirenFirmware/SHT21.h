#include <Wire.h>
#include <SHT2x.h>

#define SDA_PIN 21
#define SCL_PIN 22

class SunriseSiren3000SHT21 {
  private:
    SHT2x sht;

  public:
    int rawTemperature;
    int rawHumidity;
    float temperature;
    float humidity;

    void init() {
      Wire.begin(SDA_PIN, SCL_PIN);
      this->sht.begin();

      this->update();
    }

    void update() {
      this->sht.read();

      this->rawTemperature = this->sht.getRawTemperature();
      this->rawHumidity = this->sht.getRawHumidity();

      this->temperature = this->sht.getTemperature();
      this->humidity = this->sht.getHumidity();
    }
};
