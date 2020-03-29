#include <SPI.h>
#include <SD.h>
#include <MIDIPlayer.h>

MIDIPlayer midi;
String songName = "2.MID";
File file;
int pin = 9;

void setup()
{
  Serial.begin(9600);
  SD.begin(4);
  file = SD.open(songName);
  midi.playMIDI(file, pin);
  pinMode(3, OUTPUT);
}

void loop()
{
  delay(1000);
  digitalWrite(3, HIGH);
  delay(200);
  digitalWrite(3, LOW);
}
