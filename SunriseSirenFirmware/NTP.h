#include <NTPClient.h>
#include <WiFiUdp.h>

int tzOffset = 3600;

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "europe.pool.ntp.org", tzOffset, 60000);

class SunriseSiren3000NTP {
  public:
    void init() {
      ntpClient.begin();
      this->update();
    }

    void update() {
      ntpClient.update();
    }

    bool isTimeSet() {
      return ntpClient.isTimeSet();
    }

    String getTime() {
      char time[4];
      sprintf(time, "%02d%02d", ntpClient.getHours(), ntpClient.getMinutes());
      return String(time);
    }

    int getDay() {
      return ntpClient.getDay();
    }

    int getNextDay() {
      int nextDay = this->getDay() + 1;
      if (nextDay > 6) nextDay = 0;

      return nextDay;
    }

    void setDST(bool on) {
      tzOffset = on ? 7200 : 3600;
      ntpClient.setTimeOffset(tzOffset);
      ntpClient.forceUpdate();
    }
};