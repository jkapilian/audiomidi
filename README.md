# Audio-MIDI Converter
## Background
Like many low-end keyboards, the Casio SA-46 has a 3.5mm audio out but no MIDI output. A portable MIDI controller of similar size would likely cost multiple times the SA-46's cost. See more of the artistic motivation [here](https://jkapilian.github.io/COMS3930-portfolio/Final/final.html).

## Materials
* Keyboard with 3.5mm audio out (ex: Casio SA-46)
* 3.5mm audio cable
* AIThinker AudioKit v2.2
* USB Cable
* Foam for audio insulation
* 1/4 inch basswood (any other enclosure material is welcome)
* Computer with Mainstage or equivalent

## Technical Outline
### Ideal Scenario
As the AudioKit board has a built in 3.5mm line in, signal could simply be read from the input into one of the many FFT libraries written for Arduino boards. While MIDIUSB seemed to be a good candidate for sending MIDI messages to a computer, it was not compatable with this board in addition to the TTGO T1 board, the other one I had access to. While certain drivers such as [Hairless MIDI](http://projectgus.github.io/hairless-midiserial/#getting_started) are available to allow serial messages to be converted into MIDI signals, a custom Python script could accomplish the same while being more lightweight and flexible for this project's specific needs.

### General Information Flow
* Audio from the keyboard travel as an analog signal over the 3.5mm to the AudioKit.
* As discovered by [Phil Schatzmann](https://www.pschatzmann.ch/home/2021/12/15/the-ai-thinker-audiokit-audio-input-bug/) and others that follow him, the AIThinker has a hardware bug where input from both the microphone and line in are selected together but can not be individually separated. Fortunately, the magnitude of signals from the line in are orders of magnitude higher than most sounds into the microphone when the volume on the keyboard is at its highest level, so an aggressive magnitude cutoff is applied to the signal to effectively only listen to line in. (The Casio SA-46 does not have velocity detection so this does not impede the ability to detect softer key presses). Insulating foam around the microphones inside of the enclosure helps to emphasize this effect.
* While ArduinoFFT was initially favored, Phil Schatzmann also wrote an [FFT library](https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-audiokit/streams-audiokit-fft/streams-audiokit-fft.ino) that interacts well with audio input data. An FFT is continuously calculated over the incoming audio stream.
* When the most present magnitude is above a certain threshold, if the corresponding note is not currently being pressed, the corresponding MIDI note is calculated and sent over serial as a "note on" message. Similarly, if the threshold is not reached a "note off" message is sent.
* The Python script listens to the serial line and uses the `mido` package to send an actual MIDI message over the IAC Driver based on the custom message sent from the AudioKit. See more about the IAC Driver from a prior project [here](https://jkapilian.github.io/COMS3930-portfolio/Module%202/mod2-tech.html#iac).
* If any programs are running that listen to MIDI input such as Mainstage or Garageband, the MIDI signals sent will be played.

## Instructions to Run
* Install [PlatformIO](http://platformio.org) in VSCode.
* Clone this repository.
* Plug in the AudioKit and open the code in VSCode. Press the Upload button to flash the code onto the board.
* Plug your piano into the line in port and turn the volume to its maximum to ensure that it will exceed the threshold and will be greater than the signal from the microphone.
  - If you are using a different keyboard, you may have to experiment and set a different threshold. For the Casio-SA46, I found sound #68 was the most successful as it most closely resembled a sine wave and as such had fewer overtones but feel free to play around with other sounds.
* Enable the [IAC Driver](https://support.apple.com/guide/audio-midi-setup/transfer-midi-information-between-apps-ams1013/mac) if you have a Mac.
* Navigate to the directory of the repository in your Terminal and run `python src/main.py`.
* Open the MIDI software of choice and play notes on the keyboard.

## Enclosure Creation
* The enclosure shown was laser cut using 1/4 inch basswood. If you are using this thickness of material, this Illustrator file can be used to laser cut a corresponding box. Feel free to experiment with other enclosures!

## Notes on Parameter Choices
### Latency
As shown in the demo, the latency on this device is still fairly large (up to 100ms). This is because the FFT has to be computed in chunks and divided into bins. Because of the Nyquist frequency, the number of frequency bins must be half of the length in audio samples of the chunk. While experimenting with different values of chunk length, I found that any value lower than 4096 samples was too low to be able to distinguish between lower notes on the keyboard as the lower frequencies would get binned together. However, with a sample rate of 44.1kHz, that leads to a maximum latency due to binning of 92.9ms, in addition to any latency induced by the board's processing. Efforts to improve the latency through an increase of sampling rate were also unsuccessful as the line in appears to only work with the default 44.1kHz, although the board's maximum rate is 48kHz which still would have led to latencies on the same order of magnitude.

### Detecting Polyphony
When looking at more than frequency in each time chunk, I found that the combination of sound #66 on the keyboard with a magnitude cutoff of `30000000` was sufficient to eliminate overtones from being detected when a single note was pressed while correctly detecting when multiple notes were played. However, as shown in the image below, when multiple notes are pressed, they intermittently jump below the cutoff. Lowering the magnitude cutoff quickly reintroduced overtones. If these were mapped to MIDI on and off messages, they would cause many false MIDI off messages to be sent, making it sound as if you were jumping between two notes rather than true polyphony. While a grace period could be introduced (i.e. don't send a MIDI off signal until N timesteps without a frequency being above the cutoff), this would create even more latency than the existing setup. As such, the `top3` function is present for the sake of experimentation but should probably be reserved for boards that can sample at a higher rate or for pianos that can output pure sine tones, reducing the need for such a high cutoff.