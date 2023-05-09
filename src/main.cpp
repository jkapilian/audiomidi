#include <Arduino.h>
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "AudioLibs/AudioRealFFT.h"

#define COPY_LOG_OFF 1

AudioKitStream kit;
AudioRealFFT fft;
StreamCopy copier(fft, kit);
int cur = 0;

// display top 3 notes
void top3(AudioFFTBase &fft) {
  AudioFFTResult results[3];
  fft.resultArray(results);
  for (int i = 0; i < 3; i++) {
    AudioFFTResult result = results[i];
    if (result.magnitude > 3000000) {
      Serial.print(result.frequency);
      Serial.print(" ");
      Serial.print(result.magnitude);  
      Serial.print("\t");
    }
  }
  Serial.println("");
}

// display fft result
void fftResult(AudioFFTBase &fft){
    auto result = fft.result();
    if (result.magnitude> 5000000){
      // divide frequency by 2 because patch on keyboard that sounds closest to sine wave sounds an octave above
      int midi = round(12 * log(result.frequency/440/2)/log(2) + 69);
      // only accept MIDI messages w/in keyboard range to further reduce overtones
      if (midi != cur && midi >= 53 && midi <= 84) {
        Serial.println(midi);
        cur = midi;
      }
    } else if (cur != 0) {
      Serial.println(0);
      cur = 0;
    }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  // setup Audiokit as input device
  auto cfg = kit.defaultConfig(RX_MODE);
  cfg.input_device = AUDIO_HAL_ADC_INPUT_LINE2;
  kit.begin(cfg);

  // Setup FFT output
  auto tcfg = fft.defaultConfig();
  tcfg.length = 4096;
  tcfg.channels = 2;
  tcfg.sample_rate = 44100;
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);
}

void loop() {
  // put your main code here, to run repeatedly:
  // update FFT
  copier.copy();
}