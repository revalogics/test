/*
March 19, 2020
19-Channel Vocoder with Built-in Waveform Generator

gAEGNSRHMNSFHNMSR

Teensy 3.6
Serial + MIDI + Audio
256 MHz (overclock)
Optimize: Faster
Last tested on Arduino 1.8.12
with Teensyduino 1.51

This code does not require any hardware
Vocoding is done by passing audio in and out of Teensy, via USB

Coded by revalogics
*/

#define FREQ1               82.41   // E2
#define FREQ2               123.5   // B2 freq
#define WAVEFORM_TYPE       WAVEFORM_SAWTOOTH
#define SIBILANCE           0.7     // 0.0 to 1.0

#define FILTER_RESONANCE    5
#define ENV_ATTACK          0.995884
#define THRESHOLD           0.1

#include <Audio.h>

AudioInputI2S             INA;
AudioInputUSB             IN;
AudioSynthNoiseWhite      NOISE;
AudioSynthWaveform        WAVE[2];
AudioMixer4               MIX[9];
AudioFilterStateVariable  FILTER_V_A[19];
AudioFilterStateVariable  FILTER_V_B[19];
AudioFilterStateVariable  FILTER_I_A[19];
AudioFilterStateVariable  FILTER_I_B[19];
AudioFilterStateVariable  FILTER_S[2];
AudioAnalyzePeak          PEAK[19];
AudioOutputUSB            OUT;

AudioConnection wires[] = {
  {IN, 0, FILTER_V_A[0], 0},
  {IN, 0, FILTER_V_A[1], 0},
  {IN, 0, FILTER_V_A[2], 0},
  {IN, 0, FILTER_V_A[3], 0},
  {IN, 0, FILTER_V_A[4], 0},
  {IN, 0, FILTER_V_A[5], 0},
  {IN, 0, FILTER_V_A[6], 0},
  {IN, 0, FILTER_V_A[7], 0},
  {IN, 0, FILTER_V_A[8], 0},
  {IN, 0, FILTER_V_A[9], 0},
  {IN, 0, FILTER_V_A[10], 0},
  {IN, 0, FILTER_V_A[11], 0},
  {IN, 0, FILTER_V_A[12], 0},
  {IN, 0, FILTER_V_A[13], 0},
  {IN, 0, FILTER_V_A[14], 0},
  {IN, 0, FILTER_V_A[15], 0},
  {IN, 0, FILTER_V_A[16], 0},
  {IN, 0, FILTER_V_A[17], 0},
  {IN, 0, FILTER_V_A[18], 0},
  
  {FILTER_V_A[0], 0, FILTER_V_B[0], 0},
  {FILTER_V_A[1], 1, FILTER_V_B[1], 0},
  {FILTER_V_A[2], 1, FILTER_V_B[2], 0},
  {FILTER_V_A[3], 1, FILTER_V_B[3], 0},
  {FILTER_V_A[4], 1, FILTER_V_B[4], 0},
  {FILTER_V_A[5], 1, FILTER_V_B[5], 0},
  {FILTER_V_A[6], 1, FILTER_V_B[6], 0},
  {FILTER_V_A[7], 1, FILTER_V_B[7], 0},
  {FILTER_V_A[8], 1, FILTER_V_B[8], 0},
  {FILTER_V_A[9], 1, FILTER_V_B[9], 0},
  {FILTER_V_A[10], 1, FILTER_V_B[10], 0},
  {FILTER_V_A[11], 1, FILTER_V_B[11], 0},
  {FILTER_V_A[12], 1, FILTER_V_B[12], 0},
  {FILTER_V_A[13], 1, FILTER_V_B[13], 0},
  {FILTER_V_A[14], 1, FILTER_V_B[14], 0},
  {FILTER_V_A[15], 1, FILTER_V_B[15], 0},
  {FILTER_V_A[16], 1, FILTER_V_B[16], 0},
  {FILTER_V_A[17], 1, FILTER_V_B[17], 0},
  {FILTER_V_A[18], 2, FILTER_V_B[18], 0},
  
  {FILTER_V_B[0], 0, PEAK[0], 0},
  {FILTER_V_B[1], 1, PEAK[1], 0},
  {FILTER_V_B[2], 1, PEAK[2], 0},
  {FILTER_V_B[3], 1, PEAK[3], 0},
  {FILTER_V_B[4], 1, PEAK[4], 0},
  {FILTER_V_B[5], 1, PEAK[5], 0},
  {FILTER_V_B[6], 1, PEAK[6], 0},
  {FILTER_V_B[7], 1, PEAK[7], 0},
  {FILTER_V_B[8], 1, PEAK[8], 0},
  {FILTER_V_B[9], 1, PEAK[9], 0},
  {FILTER_V_B[10], 1, PEAK[10], 0},
  {FILTER_V_B[11], 1, PEAK[11], 0},
  {FILTER_V_B[12], 1, PEAK[12], 0},
  {FILTER_V_B[13], 1, PEAK[13], 0},
  {FILTER_V_B[14], 1, PEAK[14], 0},
  {FILTER_V_B[15], 1, PEAK[15], 0},
  {FILTER_V_B[16], 1, PEAK[16], 0},
  {FILTER_V_B[17], 1, PEAK[17], 0},
  {FILTER_V_B[18], 2, PEAK[18], 0},
  
  {WAVE[0], 0, MIX[0], 0},
  {WAVE[1], 0, MIX[0], 1},
  
  {MIX[0], 0, FILTER_I_A[0], 0},
  {MIX[0], 0, FILTER_I_A[1], 0},
  {MIX[0], 0, FILTER_I_A[2], 0},
  {MIX[0], 0, FILTER_I_A[3], 0},
  {MIX[0], 0, FILTER_I_A[4], 0},
  {MIX[0], 0, FILTER_I_A[5], 0},
  {MIX[0], 0, FILTER_I_A[6], 0},
  {MIX[0], 0, FILTER_I_A[7], 0},
  {MIX[0], 0, FILTER_I_A[8], 0},
  {MIX[0], 0, FILTER_I_A[9], 0},
  {MIX[0], 0, FILTER_I_A[10], 0},
  {MIX[0], 0, FILTER_I_A[11], 0},
  {MIX[0], 0, FILTER_I_A[12], 0},
  {MIX[0], 0, FILTER_I_A[13], 0},
  {MIX[0], 0, FILTER_I_A[14], 0},
  {MIX[0], 0, FILTER_I_A[15], 0},
  {MIX[0], 0, FILTER_I_A[16], 0},
  {MIX[0], 0, FILTER_I_A[17], 0},
  {MIX[0], 0, FILTER_I_A[18], 0},
  
  {FILTER_I_A[0], 0, FILTER_I_B[0], 0},
  {FILTER_I_A[1], 1, FILTER_I_B[1], 0},
  {FILTER_I_A[2], 1, FILTER_I_B[2], 0},
  {FILTER_I_A[3], 1, FILTER_I_B[3], 0},
  {FILTER_I_A[4], 1, FILTER_I_B[4], 0},
  {FILTER_I_A[5], 1, FILTER_I_B[5], 0},
  {FILTER_I_A[6], 1, FILTER_I_B[6], 0},
  {FILTER_I_A[7], 1, FILTER_I_B[7], 0},
  {FILTER_I_A[8], 1, FILTER_I_B[8], 0},
  {FILTER_I_A[9], 1, FILTER_I_B[9], 0},
  {FILTER_I_A[10], 1, FILTER_I_B[10], 0},
  {FILTER_I_A[11], 1, FILTER_I_B[11], 0},
  {FILTER_I_A[12], 1, FILTER_I_B[12], 0},
  {FILTER_I_A[13], 1, FILTER_I_B[13], 0},
  {FILTER_I_A[14], 1, FILTER_I_B[14], 0},
  {FILTER_I_A[15], 1, FILTER_I_B[15], 0},
  {FILTER_I_A[16], 1, FILTER_I_B[16], 0},
  {FILTER_I_A[17], 1, FILTER_I_B[17], 0},
  {FILTER_I_A[18], 2, FILTER_I_B[18], 0},
  
  {FILTER_I_B[0], 0, MIX[1], 0},
  {FILTER_I_B[1], 1, MIX[1], 1},
  {FILTER_I_B[2], 1, MIX[1], 2},
  {FILTER_I_B[3], 1, MIX[1], 3},
  {FILTER_I_B[4], 1, MIX[2], 0},
  {FILTER_I_B[5], 1, MIX[2], 1},
  {FILTER_I_B[6], 1, MIX[2], 2},
  {FILTER_I_B[7], 1, MIX[2], 3},
  {FILTER_I_B[8], 1, MIX[3], 0},
  {FILTER_I_B[9], 1, MIX[3], 1},
  {FILTER_I_B[10], 1, MIX[3], 2},
  {FILTER_I_B[11], 1, MIX[3], 3},
  {FILTER_I_B[12], 1, MIX[4], 0},
  {FILTER_I_B[13], 1, MIX[4], 1},
  {FILTER_I_B[14], 1, MIX[4], 2},
  {FILTER_I_B[15], 1, MIX[4], 3},
  {FILTER_I_B[16], 1, MIX[5], 0},
  {FILTER_I_B[17], 1, MIX[5], 1},
  {FILTER_I_B[18], 2, MIX[5], 2},
  
  {NOISE, 0, FILTER_S[0], 0},
  {FILTER_S[0], 2, FILTER_S[1], 0},
  {FILTER_S[1], 2, MIX[5], 3},
  
  {MIX[1], 0, MIX[6], 0},
  {MIX[2], 0, MIX[6], 1},
  {MIX[3], 0, MIX[6], 2},
  {MIX[4], 0, MIX[7], 0},
  {MIX[5], 0, MIX[7], 1},
  
  {MIX[6], 0, MIX[8], 0},
  {MIX[7], 0, MIX[8], 1},
  
  {MIX[8], 0, OUT, 0},
  {MIX[8], 0, OUT, 1}
};

const float freqTable[19] = {
  110.0000000,  // A2   freqTable[0]
  138.5913155,  // C#3  freqTable[1]
  174.6141157,  // F3   freqTable[2]
  220.0000000,  // A3   freqTable[3]
  277.1826310,  // C#4  freqTable[4]
  349.2282314,  // F4   freqTable[5]
  440.0000000,  // A4   freqTable[6]
  554.3652620,  // C#5  freqTable[7]
  698.4564629,  // F5   freqTable[8]
  880.0000000,  // A5   freqTable[9]
  1108.730524,  // C#6  freqTable[10]
  1396.912926,  // F6   freqTable[11]
  1760.000000,  // A6   freqTable[12]
  2217.461048,  // C#7  freqTable[13]
  2793.825851,  // F7   freqTable[14]
  3520.000000,  // A7   freqTable[15]
  4434.922096,  // C#8  freqTable[16]
  5587.651703,  // F8   freqTable[17]
  7040.000000   // A8   freqTable[18]
};
float peaksRaw[19], peaksMem[19];

elapsedMillis ledT;
bool ledstatus;

void setup() {
  pinMode(13, OUTPUT);
  AudioMemory(128);
  NOISE.amplitude(SIBILANCE);
  for(int i = 0; i < 19; i++) {
    FILTER_V_A[i].frequency(freqTable[i]);
    FILTER_V_B[i].frequency(freqTable[i]);
    FILTER_I_A[i].frequency(freqTable[i]);
    FILTER_I_B[i].frequency(freqTable[i]);
    FILTER_V_A[i].resonance(FILTER_RESONANCE);
    FILTER_V_B[i].resonance(FILTER_RESONANCE);
    FILTER_I_A[i].resonance(FILTER_RESONANCE);
    FILTER_I_B[i].resonance(FILTER_RESONANCE);
    peaksMem[i] = 1.0;
  }
  FILTER_S[0].frequency(freqTable[18]);
  FILTER_S[1].frequency(freqTable[18]);
  FILTER_S[0].resonance(FILTER_RESONANCE);
  FILTER_S[0].resonance(FILTER_RESONANCE);
  MIX[6].gain(0, 0.33);
  MIX[6].gain(1, 0.33);
  MIX[6].gain(2, 0.33);
  MIX[7].gain(0, 0.33);
  MIX[7].gain(1, 0.33);
  MIX[8].gain(0, 0.5);
  MIX[8].gain(1, 0.5);
  WAVE[0].begin(0.5, FREQ1, WAVEFORM_TYPE);
  WAVE[1].begin(0.5, FREQ2, WAVEFORM_TYPE);
}

void loop() {
  for(int i = 0; i < 19; i++) {
    if(PEAK[i].available()) {
      peaksRaw[i] = PEAK[i].read();
    }
  }
  for(int i = 0; i < 4; i++) {
    if((peaksRaw[i] * THRESHOLD) > peaksMem[i]) {
      peaksMem[i] /= ENV_ATTACK;
      MIX[1].gain(i, peaksMem[i]);
    }
    if((peaksRaw[i] * THRESHOLD) < peaksMem[i]) {
      peaksMem[i] *= ENV_ATTACK;
      MIX[1].gain(i, peaksMem[i]);
    }
  }
  for(int i = 0; i < 4; i++) {
    if((peaksRaw[i+4] * THRESHOLD) > peaksMem[i+4]) {
      peaksMem[i+4] /= ENV_ATTACK;
      MIX[2].gain(i, peaksMem[i+4]);
    }
    if((peaksRaw[i+4] * THRESHOLD) < peaksMem[i+4]) {
      peaksMem[i+4] *= ENV_ATTACK;
      MIX[2].gain(i, peaksMem[i+4]);
    }
  }
  for(int i = 0; i < 4; i++) {
    if((peaksRaw[i+8] * THRESHOLD) > peaksMem[i+8]) {
      peaksMem[i+8] /= ENV_ATTACK;
      MIX[3].gain(i, peaksMem[i+8]);
    }
    if((peaksRaw[i+8] * THRESHOLD) < peaksMem[i+8]) {
      peaksMem[i+8] *= ENV_ATTACK;
      MIX[3].gain(i, peaksMem[i+8]);
    }
  }
  for(int i = 0; i < 4; i++) {
    if((peaksRaw[i+12] * THRESHOLD) > peaksMem[i+12]) {
      peaksMem[i+12] /= ENV_ATTACK;
      MIX[4].gain(i, peaksMem[i+12]);
    }
    if((peaksRaw[i+12] * THRESHOLD) < peaksMem[i+12]) {
      peaksMem[i+12] *= ENV_ATTACK;
      MIX[4].gain(i, peaksMem[i+12]);
    }
  }
  for(int i = 0; i < 2; i++) {
    if((peaksRaw[i+16] * THRESHOLD) > peaksMem[i+16]) {
      peaksMem[i+16] /= ENV_ATTACK;
      MIX[5].gain(i, peaksMem[i+16]);
    }
    if((peaksRaw[i+16] * THRESHOLD) < peaksMem[i+16]) {
      peaksMem[i+16] *= ENV_ATTACK;
      MIX[5].gain(i, peaksMem[i+16]);
    }
  }
  if((peaksRaw[18] * THRESHOLD) > peaksMem[18]) {
    peaksMem[18] /= ENV_ATTACK;
    MIX[5].gain(2, peaksMem[18]);
    MIX[5].gain(3, peaksMem[18]);
  }
  if((peaksRaw[18] * THRESHOLD) < peaksMem[18]) {
    peaksMem[18] *= ENV_ATTACK;
    MIX[5].gain(2, peaksMem[18]);
    MIX[5].gain(3, peaksMem[18]);
  }
  
  if(ledT > 250) {
    ledT = 0;
    ledstatus = !digitalRead(13);
    digitalWrite(13, ledstatus);
  }
}
