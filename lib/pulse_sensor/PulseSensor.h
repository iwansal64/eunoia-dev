#pragma once

#include <Arduino.h>

// Basic Information
#define PULSE_SENSOR_PIN 3            // ADC pin connected to pulse sensor signal

// Timing Information
#define PULSE_SAMPLE_RATE_HZ 200       // Target sample frequency

// Sensitivity Settings
#define PULSE_TRESHOLD_FACTOR 0.01    // Increase if its too sensitive
#define PULSE_WINDOW_INTEGRATOR 30 // Integration window in samples (~150 ms at 200Hz)

// Smooth Settings
#define PULSE_BPM_HISTORY 6        // average over last N beats for smooth BPM
#define PULSE_BASELINE_ALPHA 0.995 // alpha for baseline EMA (higher = slower)

// Outlier Handlers
#define PULSE_MIN_BEAT_MS 300      // Ignore beats shorter than this
#define PULSE_MAX_BEAT_MS 2000     // Ignore beats longer than this

// Noise Handlers
#define PULSE_PEAK_DECAY 0.9995    // decay for peaks
#define PULSE_NOISE_DECAY 0.9999   // decay for noise


// Helper for codes
#define PULSE_SAMPLE_INTERVAL_US (1000000 / PULSE_SAMPLE_RATE_HZ)


class PulseSensor
{
public:
      static unsigned long last_beat_analog_read;
      static unsigned long last_beat_time;
      static int BPM;
      static bool beat_detected;
      static uint16_t beat_analog_samples[PULSE_WINDOW_INTEGRATOR];
      static uint8_t beat_analog_sample_index;

      // Running variables
      static double baseline;           // for DC removal (exponential)
      static double lastFiltered;

      // integrator buffer (moving window)
      static double integratorBuf[PULSE_WINDOW_INTEGRATOR];
      static int integratorIdx;
      static double integratorSum;

      // adaptive thresholding / peak detection
      static double noiseLevel;
      static double signalPeak;

      static uint64_t lastBeatTime;
      static uint64_t lastSampleMicros;

      static int bpmHistory[PULSE_BPM_HISTORY];
      static int bpmIdx;
      static int bpmCount;


      // Functions
      static void setupADC();
      static void pushIntegrator(double v);
      static double getIntegrator();
      static double readADCfloat();
      static void recordBeat(uint64_t now);
      static int getAvgBPM();
      
      static void init();
      static void loop();
};
