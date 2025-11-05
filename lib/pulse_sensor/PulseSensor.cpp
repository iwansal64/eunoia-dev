#include <Arduino.h>
#include <PulseSensor.h>

unsigned long PulseSensor::last_beat_analog_read = 0;
unsigned long PulseSensor::last_beat_time = 0;
int PulseSensor::BPM = 0;
bool PulseSensor::beat_detected = false;
uint8_t PulseSensor::beat_analog_sample_index = 0;

void PulseSensor::init()
{
      Serial.println("[Pulse] Initializing Pulse Sensor");
      pinMode(PULSE_SENSOR_PIN, INPUT);

      analogReadResolution(12);
      analogSetPinAttenuation(PULSE_SENSOR_PIN, ADC_11db);
}

void PulseSensor::loop()
{
      // Get the current time
      unsigned long current_time = millis();

      // Update the analog read of pulse sensor intervally
      if (current_time - last_beat_analog_read > PULSE_ANALOG_READ_INTERVAL)
      {
            // Get the current analog read of heart rate sensor
            uint16_t current_beat_analog = analogRead(PULSE_SENSOR_PIN);
            Serial.printf("%d,%d\n", current_beat_analog, BPM);

            // Detect outliers and convert all of it flat to 0
            if (current_beat_analog < PULSE_ANALOG_OUTLIER_MIN || current_beat_analog > PULSE_ANALOG_OUTLIER_MAX)
            {
                  current_beat_analog = 0;
            }

            // Update the heart beat analog samples
            beat_analog_samples[beat_analog_sample_index] = current_beat_analog;
            beat_analog_sample_index++;

            // If the analog sample index more than the size of the pulse analog samples array
            if (beat_analog_sample_index > PULSE_ANALOG_SCOUNT)
            {
                  beat_analog_sample_index = 0; // Reset the index back to zero
            }

            // Update the last analog reading for the pulse sensor
            last_beat_analog_read = current_time;
      }

      // Gather all of the signal samples and get the average of it
      uint16_t signal = 0;
      uint8_t valid_samples_count = 0;
      for (uint8_t i = 0; i < PULSE_ANALOG_SCOUNT; i++)
      {
            signal += beat_analog_samples[i];
            valid_samples_count += beat_analog_samples[i] != 0 ? 1 : 0; // If it's an outlier, adjust the valid signal counts
      }

      if (valid_samples_count > 0)
      {                                    // Prevent zero division error!
            signal /= valid_samples_count; // Divide the signal by the total of valid samples
      }

      // If the signal is above treshold and the beat is not detected just yet
      if (signal > PULSE_ANALOG_THRESHOLD && !beat_detected)
      {
            // Detect beat
            beat_detected = true;
            unsigned long beatInterval = current_time - last_beat_time;

            // If the interval of last beat and current beat is more than 300 (avoid noise) and less than 2000 (avoid inaccurate reading at first detected heartbeat)
            if (beatInterval > 300 && beatInterval < 2000)
            {
                  // BPM is beat per minute right?
                  // So, we should divide 60000 miliseconds (which is a minute)
                  // with the interval of the time of last beat and current time (current beat) which the interval between beats
                  // For example if the interval is just 1000 miliseconds (1 second) the BPM should be 60 BPM because if our heart beats every second in a minute there should be 60 beats. makes sense right? :>
                  BPM = 60000 / beatInterval;

                  Serial.print("❤️ Heartbeat detected! BPM = ");
                  Serial.println(BPM);
            }

            // Update the last time beat to current time :D
            last_beat_time = current_time;
      }

      if (signal < PULSE_ANALOG_THRESHOLD - 50)
      {
            beat_detected = false; // Reset detection
      }

      digitalWrite(LED_BUILTIN, beat_detected);

      // For debugging
      // Serial.print("Signal: ");
      // Serial.println(signal);
}
