# Arduino-MIDI-File-Sequencer
A library for Arduino devices to sequence and play MIDI files in the "background" from an SD card on a specified pin. All notes are played using the tone() function. Simplified music playing with pure tones. Tested on Arduino Uno and Nano.
-------------------------------
DEPENDENCIES:
SPI.h
SD.h
-------------------------------
PUBLIC FUNCTIONS:
void playMIDI(FILE file, int pin) -> Start playing the song in the specified MIDI file on the specified pin
void stop()                       -> Stops the song from playing, but it does not close the file. The song can still be resumed after
void start()                      -> Resumes the song from where it stopped after stop() had been invoked
