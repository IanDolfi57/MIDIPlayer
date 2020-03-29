#include <SPI.h>
#include <SD.h>
#include <MIDIPlayer.h>

MIDIPlayer midi;
String songName = "2.MID";
File file;
int pin = 9;

void setup()
{
  SD.begin(4);
  file = SD.open(songName);
  midi.playMIDI(file, pin);
  pinMode(3, OUTPUT);
}

void loop()
{
  //Basic HIGH/LOW loop on a different pin to show its "multitasking" interrupt abilities
  //The song and this loop do not typically affect each other
  delay(1000);
  digitalWrite(3, HIGH);
  delay(200);
  digitalWrite(3, LOW);
}
