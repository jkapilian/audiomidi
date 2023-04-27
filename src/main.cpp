#include <Arduino.h>
#include <arduinoFFT.h>
#include "AudioKitHal.h"
#include "MIDIUSB.h"

AudioKit kit;
const int BUFFER_SIZE = 1024;
uint8_t buffer[BUFFER_SIZE];
double dub_buf[BUFFER_SIZE/4];
arduinoFFT FFT = arduinoFFT();

void printBuffer(int len){
  // by default we get int16_t values on 2 channels = 4 bytes per frame
  int16_t *value_ptr = (int16_t*) buffer;
  for (int j=0;j<len/4;j++){
    Serial.print(*value_ptr++);
    Serial.print(", ");
    Serial.println(*value_ptr++);
  }
}

void process_data() {
  int16_t *value_ptr = (int16_t*) buffer;
  for (int j=0;j<BUFFER_SIZE/4;j++){
    dub_buf[j] = (double)*value_ptr++;
    value_ptr++;
  }
}

void run_fft() {
  FFT.Windowing(dub_buf, BUFFER_SIZE/4, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  // FFT.ComplexToMagnitude()
  double x = FFT.MajorPeak(dub_buf, BUFFER_SIZE/4, 16000);
  Serial.println(x, 6);
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  auto cfg = kit.defaultConfig(AudioInput);
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE1;
  cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
  kit.begin(cfg);
}

void loop() {
  // put your main code here, to run repeatedly:
  // size_t len = kit.read(buffer, BUFFER_SIZE);
  // process_data();
  // run_fft();
  Serial.println("Sending note on");
  noteOn(0, 48, 64);   // Channel 0, middle C, normal velocity
  MidiUSB.flush();
  delay(500);
  Serial.println("Sending note off");
  noteOff(0, 48, 64);  // Channel 0, middle C, normal velocity
  MidiUSB.flush();
  delay(1500);
}