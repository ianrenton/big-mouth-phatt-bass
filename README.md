# Big Mouth Phatt Bass

A project by Ian Renton. For more details, see https://ianrenton.com/projects/big-mouth-phatt-bass

## Toolchain

This project uses the Platform.io environment, and is designed to run on a DOIT ESP32 Devkit (or clone), using the Arduino toolkit.

## Operation

To use "normal mode", power on the Billy Bass without the front button held down. From that point, a quick button press starts the selected song. A long button press (>1 sec) cues up the next track. The ESP32 Devkit LED blinks to show the track number.

To use "sensor mode", power on the Billy Bass with the front button held down. The LED will blink 10 times, giving you time to remove your hand. From that point onwards, the LDR sensor will be used to trigger playing a song.

## Songs

The following songs are supported. I *think* the MP3s are "fair use" to share as they are heavily cut and modified. The first two are modified to crudely replace "bass" (music) with "bass" (fish). The others are just funny things for a Billy Bass to sing.

1. Warp Brothers - Phatt Bass
2. Meghan Trainor - All About that Bass
3. Mr Scruff - Fish
4. System of a Down - Chop Suey
5. Nirvana - Smells Like Teen Spirit (Work in Progress)
6. Rage Against the Machine - Killing in the Name (Work in Progress)
7. Metallica - Enter Sandman (Work in Progress)

You can download the contents of the SD card used in the project [here](https://ianrenton.com/projects/big-mouth-phatt-bass/sdcard.zip). This contains the song sections plus announcer voices.