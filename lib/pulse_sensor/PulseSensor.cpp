#include <Arduino.h>
#include <PulseSensor.h>

unsigned long PulseSensor::last_beat_analog_read = 0;
unsigned long PulseSensor::last_beat_time = 0;
int PulseSensor::BPM = 0;
bool PulseSensor::beat_detected = false;
uint8_t PulseSensor::beat_analog_sample_index = 0;
uint16_t PulseSensor::beat_analog_samples[PULSE_WINDOW_INTEGRATOR];

// Running variables
double PulseSensor::baseline = 0.0; // for DC removal (exponential)
double PulseSensor::lastFiltered = 0.0;

// integrator buffer (moving window)
double PulseSensor::integratorBuf[PULSE_WINDOW_INTEGRATOR];
int PulseSensor::integratorIdx = 0;
double PulseSensor::integratorSum = 0.0;

// adaptive thresholding / peak detection
double PulseSensor::noiseLevel = 0.0;
double PulseSensor::signalPeak = 0.0;

uint64_t PulseSensor::lastBeatTime = 0;
uint64_t PulseSensor::lastSampleMicros = 0;

int PulseSensor::bpmHistory[PULSE_BPM_HISTORY];
int PulseSensor::bpmIdx = 0;
int PulseSensor::bpmCount = 0;

void PulseSensor::setupADC() {
      analogReadResolution(12);
      analogSetPinAttenuation(PULSE_SENSOR_PIN, ADC_11db);
}

void PulseSensor::pushIntegrator(double v)
{
      PulseSensor::integratorSum -= PulseSensor::integratorBuf[PulseSensor::integratorIdx];
      PulseSensor::integratorBuf[PulseSensor::integratorIdx] = v;
      PulseSensor::integratorSum += PulseSensor::integratorBuf[PulseSensor::integratorIdx];
      PulseSensor::integratorIdx = (PulseSensor::integratorIdx + 1) % PULSE_WINDOW_INTEGRATOR;
}

double PulseSensor::getIntegrator()
{
      return PulseSensor::integratorSum / PULSE_WINDOW_INTEGRATOR;
}

double PulseSensor::readADCfloat()
{
      int raw = analogRead(PULSE_SENSOR_PIN);
      return (double)raw; // 0..4095
}

void PulseSensor::recordBeat(uint64_t now)
{
      if (PulseSensor::lastBeatTime == 0)
      {
            PulseSensor::lastBeatTime = now;
            return;
      }
      uint64_t dt = now - PulseSensor::lastBeatTime;
      PulseSensor::lastBeatTime = now;
      if (dt < PULSE_MIN_BEAT_MS || dt > PULSE_MAX_BEAT_MS)
            return; // ignore
      int instBPM = (int)round(60000.0 / dt);
      PulseSensor::bpmHistory[PulseSensor::bpmIdx] = instBPM;
      PulseSensor::bpmIdx = (PulseSensor::bpmIdx + 1) % PULSE_BPM_HISTORY;
      if (PulseSensor::bpmCount < PULSE_BPM_HISTORY)
            PulseSensor::bpmCount++;
}

int PulseSensor::getAvgBPM()
{
      if (PulseSensor::bpmCount == 0)
            return 0;
      long s = 0;
      for (int i = 0; i < PulseSensor::bpmCount; ++i)
            s += PulseSensor::bpmHistory[i];
      return (int)(s / PulseSensor::bpmCount);
}

void PulseSensor::init()
{
      Serial.println("[Pulse] Initializing Pulse Sensor");
      pinMode(PULSE_SENSOR_PIN, INPUT);

      PulseSensor::setupADC();

      for (int i = 0; i < PULSE_WINDOW_INTEGRATOR; ++i) PulseSensor::integratorBuf[i] = 0.0;
      PulseSensor::lastSampleMicros = micros();
}

void PulseSensor::loop()
{
      unsigned long nowMicros = micros();
      // maintain sampling interval
      if (nowMicros - PulseSensor::lastSampleMicros < PULSE_SAMPLE_INTERVAL_US)
      {
            // yield to not hog CPU
            yield();
            return;
      }
      PulseSensor::lastSampleMicros += PULSE_SAMPLE_INTERVAL_US; // keep steady sampling

      // 1) read raw
      double raw = PulseSensor::readADCfloat();

      // 2) DC removal (slow exponential moving average baseline)
      PulseSensor::baseline = PULSE_BASELINE_ALPHA * PulseSensor::baseline + (1.0 - PULSE_BASELINE_ALPHA) * raw;
      double ac = raw - PulseSensor::baseline; // centered AC component (can be negative)

      // 3) derivative (simple)
      static double prevAC = 0.0;
      double deriv = ac - prevAC;
      prevAC = ac;

      // 4) square to accentuate peaks
      double squared = deriv * deriv;

      // 5) moving-window integrator
      pushIntegrator(squared);
      double integrator = PulseSensor::getIntegrator();

      // 6) adaptively track peak & noise
      // decay peak & noise slowly
      PulseSensor::signalPeak *= PULSE_PEAK_DECAY;
      PulseSensor::noiseLevel *= PULSE_NOISE_DECAY;
      // update with current integrator
      if (integrator > PulseSensor::signalPeak)
            PulseSensor::signalPeak = integrator;
      else
            PulseSensor::noiseLevel = max(PulseSensor::noiseLevel, integrator * 0.5f);

      // 7) adaptive threshold: midway between peak and noise (tweak multiplier)
      double threshold = PulseSensor::noiseLevel + ((double) PULSE_TRESHOLD_FACTOR) * (PulseSensor::signalPeak - PulseSensor::noiseLevel);

      // 8) detect rising edge crossing threshold with refractory
      static bool lastOver = false;
      bool over = integrator > threshold;
      uint64_t now = millis();
      uint64_t lastDetectedMs = 0;

      if (over && !lastOver)
      {
            // rising edge
            if (millis() - lastDetectedMs > PULSE_MIN_BEAT_MS)
            {
                  // good beat
                  PulseSensor::recordBeat(now);
                  lastDetectedMs = millis();
            }
      }
      lastOver = over;

      // 9) Update the BPM value
      PulseSensor::BPM = PulseSensor::getAvgBPM();


      // 10) [Optional] Print values to serial plotter
#ifdef DEBUG
      Serial.printf("Beat:%d\n", over ? 1 : 0);
      Serial.printf("Raw:%d\n", (int)integrator);
      Serial.printf("Threshold:%d\n", threshold);
      Serial.printf("BPM:%d\n", PulseSensor::BPM);
#endif
      yield();
}
