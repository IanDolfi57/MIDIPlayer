/*
 * MIDIPlayer.h - Library for playing MIDI files
 * Outputs tone() on specified pin
 * Created by Ian J. Dolfi, January 20, 2020
 * Released into the poublic domain
 */

#ifndef MIDIPlayer_h
#define MIDIPlayer_h

#include "SPI.h"
#include "SD.h"
#include "Arduino.h"

class MIDIPlayer
{
  public:
    void playMIDI(File SONG, int OutputPin);
	void interruptFunction();
	void stop();
	void start();
	bool finished;
  private:
    bool tickdiv;
    int ppqn; //Pulses or Ticks per quarter note  |
    long tempo; //Microseconds per quarter note   |> metrical timing
    float timeUnits; //Miliseconds per tick       |
    int fps; //Frames per second        |
    int subres; //Sub-frame resolution  |> timecode
    int note;
    File song;
    int outputPin = 9;
	float noteToFrequency(int NOTE);
	void readSection(char *buf, int num);
	void seekTrack(unsigned long &CHUNKLEN);
	void sequenceMIDI();
	unsigned long readVarLengthNum();
	bool readEvent();
};

static MIDIPlayer *PLAYER;

#endif