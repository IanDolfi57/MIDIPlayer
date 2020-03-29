/*
 * MIDIPlayer.h - Library for playing MIDI files
 * Outputs tone() on specified pin
 * Created by Ian J. Dolfi, January 20, 2020
 * Released into the poublic domain
 * 
 * ------------------------------------------------------------------
 * 
 * NOTE: All of the serial debugging statements are disabled
 * because it greatly slows down the program,
 * causing the song to play much slower than intended
 * 
 * Contact ijiflod@gmail.com for questions
 */


#include "SPI.h"
#include "SD.h"
#include "Arduino.h"
#include "MIDIPlayer.h"


bool tickdiv;
int ppqn; //Pulses or Ticks per quarter note  |
long tempo; //Microseconds per quarter note   |> metrical timing
float timeUnits; //Miliseconds per tick       |

int fps; //Frames per second        |
int subres; //Sub-frame resolution  |> timecode

unsigned long delta; //Target in tics
float deltaTime; //In miliseconds
bool finished = true;

int note;
File song;

int outputPin = 9;


float MIDIPlayer::noteToFrequency(int NOTE) //MIDI note number to coresponding frequency
{
  float FREQUENCY = pow(2, (float)(((float)(NOTE) - (float)(69))/12)) * 440;
  //Serial.println("Note: " + (String)NOTE);
  //Serial.println("Frequency: " + (String)FREQUENCY);
  return FREQUENCY;
}


bool MIDIPlayer::readEvent()
{
  byte status;
  status = song.read();
  
  if ((status >= 0x80 && status <= 0xEF)) //MIDI Event
  {
    //Serial.println("MIDI Event");
    byte nib1 = (status & 0b11110000) >> 4;
    byte nib2 = status & 0b00001111;

    /*
    if (nib2 != 0x0)
    {
      //return 1;
    }
    */
    
    if (nib1 == 0x8) //Note off
    {
      //Serial.println("Stop Note");
        noTone(outputPin);
        song.read();
      if (song.read() == note) //Only turn off the tone if this message is reffering to the currently playing note
      {
        //noTone(9);
        //song.read();
      }
      return 1;
    }
    else if (nib1 == 0x9) //Note on
    {
      //Serial.println("Play Note");
      noTone(outputPin);
      note = song.read();
      tone(outputPin, noteToFrequency(note));
      song.read();
      return 1;
    }
    else if (nib1 == 0xC || nib1 == 0xD)
    {
      //Serial.print(status, HEX);
      //Serial.print(" ");
      //Serial.print(song.read(), HEX);
      //Serial.println();
      return 1;
    }
    else
    {
      //Serial.print(status, HEX);
      //Serial.print(" ");
      //Serial.print(song.read(), HEX);
      //Serial.print(" ");
      //Serial.print(song.read(), HEX);
      //Serial.println();
      return 1;
    }
    return 0;
  }
  
  
  if ((status == 0xF0 || status == 0xF7)) //SysEx Event
  {
    //Skip over this event
    //Serial.println("SysEx Event");
    long len = readVarLengthNum();
    song.seek(song.position() + len);
    return 1;
  }
  
  if (status == 0xFF) //Meta Event
  {
    //Serial.println("Meta Event");
    byte BYTE = song.read();
    if (BYTE == 0x2F) //End of Track
    {
      if (song.read() == 0x00)
      {
        return 0;
      }
      else
      {
        //Serial.println("ERROR: POSSIBLE END OF TRACK, NOT REALLY SURE!");
        return 0;
      }
    }
    else if (BYTE == 0x51) //Set Tempo
    {
      if (song.read() == 0x03)
      {
        //Serial.println("Tempo set");
        byte TEMP = 0x0;
        tempo = 0x0000;
        TEMP = song.read();
        tempo = (tempo | (TEMP & 0xFFFF)) << 8;
        TEMP = song.read();
        tempo = (tempo | (TEMP & 0xFFFF)) << 8;
        TEMP = song.read();
        tempo = tempo | (TEMP & 0xFFFF);
        //Serial.println(tempo);
        if (!tickdiv)
        {
          timeUnits = (((float)tempo)/((float)ppqn))/1000.0;
          //Serial.println("Pulse per Quarter Note");
          //Serial.println("Pulses Per Quarter Note: " + (String)ppqn);
          //Serial.println("Tempo: " + (String)tempo);
          //Serial.println("Time Units: " + (String)timeUnits);
        }
        else
        {
          timeUnits = (1/fps)/subres * 1000; //In miliseconds
          //Serial.println("Time Code");
          //Serial.println("Time Units: " + (String)timeUnits);
        }
        return 1;
      }
      else
      {
        //Serial.println("ERROR: POSSIBLE TEMPO CHANGE, NOT REALLY SURE!");
        return 0;
      }
    }
    else
    {
      //Serial.println(BYTE, HEX);
      unsigned long len = readVarLengthNum();
      //Serial.println("Skipping Next " + (String)len + " Bytes");
      song.seek(song.position() + len);
    }
    return 1;
  }
  
  //Serial.println("INVALID MESSAGE TYPE");
  return 1;
}

void MIDIPlayer::readSection(char *buf, int num)
{
  buf = new char[num]; 
  for (int i = 0; i < num; i++)
  {
    buf[i] = song.read();
  }
}

void MIDIPlayer::seekTrack(unsigned long &CHUNKLEN)
{
  //Serial.println("Seeking Track");
  char header[8];
  
  while (true)
  {
    //Serial.println("Now reading from byte " + (String)song.position());
    song.read(header, 8);
    
    CHUNKLEN = (((((header[4] & 0xFFFF) << 24) | header[5]) & 0xFFFF) << 16) | ((header[6] & 0xFFFF) << 8) | header[7];
  
    if (!(header[2] == 'r' && header[3] == 'k'))
    {
      //Serial.println("No track at byte " + (String)(song.position() - 8));
      //Serial.print((int)header[0]);
      //Serial.print((int)header[1]);
      //Serial.print((int)header[2]);
      //Serial.print((int)header[3]);
      //Serial.println();
      song.seek(song.position() + CHUNKLEN);
      if (song.position() + 8 >= song.size())
      {
        //Serial.println("NO TRACK FOUND IN FILE!!!");
        break;
      }
    }
    else
    {
      break;
    }
  }
  //Serial.println("Track Found");
}

unsigned long MIDIPlayer::readVarLengthNum()
{
  unsigned long num = 0;
  byte BYTE;
  int n = 0;
  while (true)
  {
    n++;
    BYTE = song.read();
    if (bitRead(BYTE, 7))
    {
      num = (num | (BYTE & 0b01111111)) << 7;
    }
    else
    {
      num = num | (BYTE & 0b01111111);
      //Serial.println("Bytes in Variable Length Number: " + (String)n);
      break;
    }
  }
  song.seek(song.position() - n);
  byte temp;
  for (int i = 0; i < n; i++)
  {
    temp = song.read();
    //Serial.print(temp, BIN);
    //Serial.println();
  }
  return num;
}

void MIDIPlayer::sequenceMIDI()
{
  byte temp;
  /*
  for (int i = 0; i < 64; i++)
  {
    temp = song.read();
    Serial.print((String)i + " - " + (String)(char)temp + " - 0x");
    Serial.print(temp, HEX);
    Serial.print(" - ");
    Serial.print(temp, BIN);
    Serial.println();
  }
  */
  Serial.println("-----------------------------------------------------------------------------");
  song.seek(0);
  char HEADER[8];
  unsigned long chunklen;
  int pointer = 0;
  
  song.read(HEADER, 8);
  String name = (String)song.name();
  if (!((name).substring(name.length() - 4) == ".MID"))
  {
    //Serial.println(name + " IS NOT A VALID FILE TYPE!");
    return;
  }
  
  //Read Header information
  if (!(HEADER[2] == 'h' && HEADER[3] == 'd'))
  {
    //Serial.println("NO HEADER IN " + (String)song.name());
    //Serial.println("Header is: " + (String)HEADER[0] + (String)HEADER[1] + (String)HEADER[2] + (String)HEADER[3]);
    //Serial.println((String)HEADER[0]);
    //Serial.println((String)HEADER[1]);
    //Serial.println((String)HEADER[2]);
    //Serial.println((String)HEADER[3]);
   
    return;
  }
  
  chunklen = (((((HEADER[4] & 0xFFFF) << 24) | HEADER[5]) & 0xFFFF) << 16) | ((HEADER[6] & 0xFFFF) << 8) | HEADER[7];
  
  //Serial.println((String)chunklen);
  
  char INFO[chunklen];
  song.read(INFO, chunklen);
  if (INFO[pointer++] != 0)
  {
    //Serial.println("WRONG MIDI FORMAT: " + (String)INFO[pointer]);
    return;
  }
  pointer += 3;
  
  //Determine timing type, ugh
  tickdiv = ((INFO[pointer] & 0b10000000) >> 7);
  
  if (!tickdiv)
  {
    //Serial.println("Pulse per Quarter Note");
    tempo = 500000;
    byte ppqnTEMP = 0x00;
    ppqn = 0x0000;
    ppqnTEMP = INFO[pointer];
    //Serial.println(ppqn, HEX);
    //Serial.println(ppqnTEMP, HEX);
    ppqn = (ppqn | (ppqnTEMP & 0xFFFF)) << 8;
    //Serial.println(ppqn, HEX);
    pointer++;
    ppqnTEMP = INFO[pointer];
    //Serial.println(ppqnTEMP, HEX);
    ppqn = ppqn | (ppqnTEMP & 0xFFFF);
    //Serial.println(ppqn, HEX);
    timeUnits = (((float)tempo)/ppqn)/1000;
    //Serial.println("Pulses Per Quarter Note: " + (String)ppqn);
    //Serial.println("Tempo: " + (String)tempo);
    //Serial.println("Time Units: " + (String)timeUnits);
  }
  else
  {
    fps = (int)INFO[pointer] * -1;
    subres = INFO[pointer++];
    timeUnits = (1/fps)/subres * 1000; //In miliseconds
    //Serial.println("Time Code");
    //Serial.println("Time Units: " + (String)timeUnits);
  }
  
  //------------------------------------------------------
  //Read track information
  seekTrack(chunklen); //Find the actual track chunk, in case metadata exists
  
  pointer = 0;
  //Serial.println("Now reading from byte " + (String)song.position());
  delta = readVarLengthNum();
}

void MIDIPlayer::playMIDI(File SONG, int OutputPin)
{
	PLAYER = this;
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
	song = SONG;
	outputPin = OutputPin;
	finished = false;
	sequenceMIDI();
}

SIGNAL(TIMER0_COMPA_vect) 
{
	PLAYER->interruptFunction();
} 

void MIDIPlayer::interruptFunction()
{
	if (!finished)
	{
		 deltaTime++;
		if (deltaTime >= ( delta *  timeUnits))
		{
			deltaTime = 0;
  			if(!readEvent())
			{
				 finished = true;
			}
			delta =  readVarLengthNum();
		}
	}
}

void MIDIPlayer::stop()
{
	finished = true;
}

void MIDIPlayer::start()
{
	finished = false;
}