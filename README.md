# USB MIDI to CV

Project inspired by [elkayem/midi2cv](https://github.com/elkayem/midi2cv), but using MIDI over USB communication.

## Requirements

- Arduino Leonardo or any board based on ATmega32u4 or ARM ([MIDIUSB compatibility](https://docs.arduino.cc/libraries/midiusb/#Compatibility))  
- MCP4728 (PCBs are designed for the generic GY-MCP4728 module)  
- LM324N  
- Some resistors and connectors

> ⚠️ The resistors connected to the A output of the LM324N don't need to match exact values, as long as they maintain an amplification ratio of approximately 1.77. Inaccurate values may affect 1V/Oct tuning.

## How It Works

- When a MIDI note is received (regardless of channel), it is converted to CV in 1V/Oct format, with C0 = 0V, C1 = 1V, C2 = 2V... covering 88 notes total.
- When a second note is played, it replaces the previous one, which is kept in a buffer.
- **Velocity** is converted to a voltage ranging from 0V to 4V.
- A **Trigger Pulse** of 5V is sent for 20ms every time a note is triggered.
- The **Gate** stays high (5V) while any note is being played.
- **Pitch Bend** is centered at 0.5V and ranges from 0V to 1V.
- Any **Control Change (CC)** message is converted to a voltage from 0V to 4V.

## Schematic

![Schematic](https://github.com/dioubernardo/usbmidi2cv/blob/main/diagram.png)

The file [`usbmidi2cv.sch`](https://github.com/dioubernardo/usbmidi2cv/blob/main/usbmidi2cv.sch) contains the schematic and can be edited using Eagle.

## PCB

Designed to work as an Arduino shield, this PCB can be easily etched on a single-sided copper board for DIY builds.

The file [`usbmidi2cv.brd`](https://github.com/dioubernardo/usbmidi2cv/blob/main/usbmidi2cv.brd) contains the PCB layout, also editable in Eagle.