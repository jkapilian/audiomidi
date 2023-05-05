#include <Arduino.h>
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "AudioLibs/AudioRealFFT.h"

#define COPY_LOG_OFF 1

AudioKitStream kit;
AudioRealFFT fft;
StreamCopy copier(fft, kit);
int cur = 0;

// void printBuffer(int len){
//   // by default we get int16_t values on 2 channels = 4 bytes per frame
//   int16_t *value_ptr = (int16_t*) buffer;
//   for (int j=0;j<len/4;j++){
//     Serial.print(*value_ptr++);
//     Serial.print(",");
//     Serial.println(*value_ptr++);
//   }
// }

// void process_data() {
//   int16_t *value_ptr = (int16_t*) buffer;
//   for (int j=0;j<BUFFER_SIZE/4;j++){
//     dub_buf[j] = (double)*value_ptr++;
//     value_ptr++;
//   }
// }

// void run_fft() {
//   FFT.Windowing(dub_buf, BUFFER_SIZE/4, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
//   FFT.Compute(FFT_FORWARD);
//   // FFT.ComplexToMagnitude()
//   double x = FFT.MajorPeak(dub_buf, BUFFER_SIZE/4, 16000);
//   Serial.println(x, 6);
// }

// display fft result
void fftResult(AudioFFTBase &fft){
    float diff;
    auto result = fft.result();
    if (result.magnitude>1000000){
      // Serial.println(result.magnitude);
      /*Serial.print(result.frequency);
      Serial.print(" ");
      Serial.print(result.magnitude);  
      Serial.print(" => ");
      Serial.print(result.frequencyAsNote(diff));
      Serial.print( " diff: ");
      Serial.println(diff);*/
      result.frequencyAsNote(diff);
      if (abs(diff) < 10) {
        // Serial.println(12 * log(result.frequency/440)/log(2) + 69);
        int midi = round(12 * log(result.frequency/440)/log(2) + 69);
        if (midi != cur) {
          Serial.println(midi);
          cur = midi;
        }
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
  tcfg.length = 1024;
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