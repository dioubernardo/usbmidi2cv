/*
  Monophonic - Last note, with 10 notes in buffer

  Note CV output (88 keys, 1V/octave)                         DAC A
  Velocity CV output (0 to 4V)                                DAC B

  Pitch bend CV output (0 to 1V, witch 0.5 central possition) DAC C
  Control Change CV out (0 to 4V)                             DAC D

  Trigger output (5V, pulso de 20 msec)                       D4
  Gate output (5V quando alguma tecla estiver precionada)     D5
*/

#include "MIDIUSB.h"
#include <Adafruit_MCP4728.h>

#define GATE_PIN    4
#define TRIGGER_PIN 5

const byte bufferSize = 10;
const byte lastIndex = bufferSize - 1;

byte bufferNotes[bufferSize] = {};
byte notes[88] = {};
unsigned long triggerInit = 0;

Adafruit_MCP4728 mcp;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);

  Serial.begin(115200);
  delay(200);

  for (int i = 0; i <= lastIndex; i++) {
    bufferNotes[i] = 255;
  }

  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    bool on = 0;
    while (1) {
      on = !on;
      digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
      delay(500);
    }
  }

  Serial.println("OK");

  mcp.setChannelValue(MCP4728_CHANNEL_A, 0);
  mcp.setChannelValue(MCP4728_CHANNEL_B, 0);

  // Set initial pitch bend voltage to 0.5V (mid point)
  mcp.setChannelValue(MCP4728_CHANNEL_C, 1023, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  mcp.setChannelValue(MCP4728_CHANNEL_D, 0);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {

  byte note;
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      switch (rx.header) {

        case 0x09: // Note ON

          if (rx.byte2 < 12 || rx.byte2 > 99) // only 88 keys from C0
            break;

          note = rx.byte2 - 12;
          for (int i = (lastIndex - 1); i >= -1; i--) {
            if (bufferNotes[i] != 255 || i == -1) {
              notes[note] = rx.byte3;
              bufferNotes[i + 1] = note;
              emitNote(note);
              break;
            }
          }

          break;

        case 0x08: // Note OFF

          if (rx.byte2 < 12 || rx.byte2 > 99) // only 88 keys, From C0 to D7#
            break;

          note = rx.byte2 - 12;

          int i;
          for (i = lastIndex; i >= 0; i--) {
            if (bufferNotes[i] == note) {
              bufferNotes[i] = 255;
              break;
            }
          }

          for (i = lastIndex; i >= 0; i--) {
            if (bufferNotes[i] != 255) {
              emitNote(bufferNotes[i]);
              break;
            }
          }

          if (i == -1) {
            digitalWrite(GATE_PIN, LOW);
          }

          break;

        case 0x0E: // Pitch Bend

          // Pitch Bend range from 0 to 16383
          // Right shift by 3 to scale from 0 to 2047
          unsigned int val = (rx.byte3 << 7) | rx.byte2;
          mcp.setChannelValue(MCP4728_CHANNEL_C, val >> 3, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

          break;

        case 0x0B: // Control Change
          // CC range from 0 to 127
          // Left shift d2 by 5 to scale from 0 to 4095
          mcp.setChannelValue(MCP4728_CHANNEL_D, rx.byte2 << 5, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);

          break;

      }
      /*
            Serial.print(rx.header, HEX);
            Serial.print(" ");
            Serial.print(rx.byte1, HEX);
            Serial.print(" ");
            Serial.print(rx.byte2, HEX);
            Serial.print(" ");
            Serial.print(rx.byte3, HEX);
            Serial.println(" ");
      */

    }
  } while (rx.header != 0);

  if (triggerInit > 0 && (millis() - triggerInit) > 20) {
    triggerInit = 0;
    digitalWrite(TRIGGER_PIN, LOW);
  }
}

void emitNote(byte note) {

  /*
    0 -> 0V
    4095 -> VREF_INTERNAL MCP4728_GAIN_2X -> 4,096V
    Volts per note = 4095 / (88 - 1) = 47.06896551724138
    The OPAMP need amplifier 1,77x
  */
  mcp.setChannelValue(MCP4728_CHANNEL_A, note * 47.06896551724138f, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);

  // Velocity range from 0 to 4095 mV
  // Left shift by 5 to scale from 0 to 4095
  mcp.setChannelValue(MCP4728_CHANNEL_B, notes[note] << 5, MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);

  digitalWrite(GATE_PIN, HIGH);
  digitalWrite(TRIGGER_PIN, HIGH);
  triggerInit = millis();
}
