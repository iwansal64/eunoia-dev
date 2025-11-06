#include <Arduino.h>
#include <PulseSensor.h>

#define READ_INTERVAL 100

uint64_t last_time_read;

void setup()
{
      Serial.begin(9600);
      PulseSensor::init();

      last_time_read = millis();
}

void loop()
{
      PulseSensor::loop();

      uint64_t current_time = millis();

      if (current_time - last_time_read > READ_INTERVAL)
      {
            // Serial.println(PulseSensor::BPM);
            last_time_read = current_time;
      }
}
