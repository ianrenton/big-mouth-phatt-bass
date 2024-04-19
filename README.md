# Big Mouth Phatt Bass

A project by Ian Renton. For more details, see https://ianrenton.com/projects/big-mouth-phatt-bass

## Toolchain

This project uses the Platform.io environment, and is designed to run on a DOIT ESP32 Devkit (or clone), using the Arduino toolkit.

## Operation

To use "normal mode", power on the Billy Bass without the front button held down. From that point, a quick button press starts the selected song. A long button press (>500ms) cues up the next track. The announcer voice MP3s will tell you which track will play.

To use "sensor mode", power on the Billy Bass with the front button held down. The announcer voice will tell you that Sensor Mode is enabled, giving you time to remove your hand. From that point onwards, the LDR sensor will be used to trigger playing a song.

## Songs

The following songs are supported. I *think* the MP3s are "fair use" to share for parody purposes as they are heavily cut and some are modified. The first two are modified to crudely replace "bass" (music) with "bass" (fish). The others are just funny things for a Billy Bass to sing.

1. Warp Brothers - Phatt Bass
2. Meghan Trainor - All About that Bass
3. Mr Scruff - Fish
4. System of a Down - Chop Suey
5. Nirvana - Smells Like Teen Spirit
6. Rage Against the Machine - Killing in the Name
7. Metallica - Enter Sandman

You can download the contents of the SD card used in the project [here](https://ianrenton.com/projects/big-mouth-phatt-bass/sdcard.zip). This contains the song sections plus announcer voices.