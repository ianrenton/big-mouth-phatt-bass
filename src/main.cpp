// Big Mouth Phatt Bass control code
// by Ian Renton, 2024. CC Zero / Public Domain

// Debug mode - for lip sync testing - reduces volume to avoid annoying the family and autoplays chosen track
#define DEBUG false
#define DEBUG_VOLUME 10
#define DEBUG_AUTOPLAY_TRACK 1

// Button and sensor pins
#define BUTTON_PIN 4
#define LDR_PIN 33
#define LONG_PRESS_DURATION_MILLIS 500 // How long do you hold the button down to count as a long press?

// Motor control pins
#define HEADTAIL_MOTOR_PIN_1 12
#define HEADTAIL_MOTOR_PIN_2 14
#define HEADTAIL_MOTOR_PWM_PIN 13
#define MOUTH_MOTOR_PIN_1 27
#define MOUTH_MOTOR_PIN_2 26
#define MOUTH_MOTOR_PWM_PIN 25

// Motor PWM settings
#define PWM_FREQUENCY 1000
#define PWM_RESOLUTION 8
#define HEADTAIL_MOTOR_PWM_CHANNEL 0
#define MOUTH_MOTOR_PWM_CHANNEL 1
#define HEADTAIL_MOTOR_PWM_DUTY_CYCLE 255 // Proxy for motor speed, up to 2^resolution
#define MOUTH_MOTOR_PWM_DUTY_CYCLE 255    // Proxy for motor speed, up to 2^resolution

// Music player settings
#define TRACK_NUMBER_FOR_SENSOR_MODE 1 // In sensor mode you don't get to select track, use this one
#define MAX_TRACK_NUMBER 10
#define MUSIC_VOLUME 20 // Up to 30
#define ANNOUNCER_VOLUME 10 // Up to 30
#define MUSIC_FOLDER 1 // Corresponds to folder "01" on SD card
#define ANNOUNCER_FOLDER 2 // Corresponds to folder "02" on SD card
#define SENSOR_MODE_ANNOUNCER_TRACK_NUMBER 99 // Corresponds to file "02/099.mp3" on SD card
#define MP3_PLAYER_BAUD_RATE 9600


// Includes
#include <Arduino.h>

// Function defs
void indicateReady();
boolean isButtonPushed();
double getLightLevel();
void announceTrackName(int trackNumber);
void announceSensorMode();
void trigger(int trackNumber);
void lipsyncPhattBass();
void lipsyncAllAboutThatBass();
void lipsyncMrScruffFish();
void lipsyncChopSuey();
void lipsyncSmellsLikeTeenSpirit();
void lipsyncKillingInTheName();
void lipsyncEnterSandman();
void lipsyncCloser();
void lipsyncIAmJustAFish();
void lipsyncBasketCase();
void playTrack(int foldernum, int tracknum);
void flapHeadFor(int runtime, int interval);
void flapTailFor(int runtime, int interval);
void flapHead(int interval);
void flapTail(int interval);
void headOut();
void tailOut();
void headTailRest();
void flapMouthFor(int runtime, int interval);
void flapMouthAndTailTogetherFor(int runtime, int interval);
void mouthOpenFor(int runtime);
void mouthOpen();
void mouthClose();
void mouthRest();
void stop();
void changeVolume(int thevolume);
void sendCommandToMP3Player(byte command, int dataBytes);
void lightSleep(int timeMs);


// Variable defs
int trackNumber = 1;
bool sensorMode = false;
double lastSensorLightLevel = 0;


// Setup and run the program
void setup() {
  // Set up button and LDR pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT_PULLUP);

  // Set up motor control pins
  pinMode(HEADTAIL_MOTOR_PIN_1, OUTPUT);
  pinMode(HEADTAIL_MOTOR_PIN_2, OUTPUT);
  pinMode(HEADTAIL_MOTOR_PWM_PIN, OUTPUT);
  pinMode(MOUTH_MOTOR_PIN_1, OUTPUT);
  pinMode(MOUTH_MOTOR_PIN_2, OUTPUT);
  pinMode(MOUTH_MOTOR_PWM_PIN, OUTPUT);

  // Set up PWM
  ledcSetup(HEADTAIL_MOTOR_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(MOUTH_MOTOR_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(HEADTAIL_MOTOR_PWM_PIN, HEADTAIL_MOTOR_PWM_CHANNEL);
  ledcAttachPin(MOUTH_MOTOR_PWM_PIN, MOUTH_MOTOR_PWM_CHANNEL);
  ledcWrite(HEADTAIL_MOTOR_PWM_CHANNEL, HEADTAIL_MOTOR_PWM_DUTY_CYCLE);
  ledcWrite(MOUTH_MOTOR_PWM_CHANNEL, MOUTH_MOTOR_PWM_DUTY_CYCLE);

  // Set up serial comms to MP3 player
  Serial2.begin(MP3_PLAYER_BAUD_RATE);
  while (!Serial2);

  // Reset anything going on on the motor & MP3 boards
  stop();

  // If we are in debug mode to speed up lip-sync testing, autoplay the chosen track.
  if (DEBUG) {
    trigger(DEBUG_AUTOPLAY_TRACK);
    return;
  }

  // Check startup mode. If the button is held down at startup time, we go into
  // "sensor mode" to use the LDR to trigger the fish, otherwise we go into
  // normal mode where a button press triggers it.
  if (isButtonPushed()) {
    // Button held in on startup, so going into sensor mode.
    // Wait for button to be unpushed, then the mode/track announce sequence will give time for the user to move away
    sensorMode = true;
    trackNumber = TRACK_NUMBER_FOR_SENSOR_MODE;

    announceSensorMode();
    while (isButtonPushed()) {
      lightSleep(10);
    }
    lightSleep(2000);

    // Record the current light level, so we don't trigger immediately
    lastSensorLightLevel = getLightLevel();

  } else {
    // Normal mode, nothing else to do here
  }

  announceTrackName(trackNumber);
  lightSleep(1000);
}

// Main program loop
void loop() {
  // Wait for a trigger condition, either a change in light level or
  // a button push depending on our mode.
  if (sensorMode) {
    double lightLevel = getLightLevel();
    if (lightLevel < lastSensorLightLevel - 0.04 || lightLevel > lastSensorLightLevel + 0.04) {
      trigger(trackNumber);
    }
    lastSensorLightLevel = lightLevel;
    lightSleep(200);

  } else if (isButtonPushed()) {
    // Not in sensor mode, and the button was pushed. If it was pushed for less than half a second,
    // this is the sign to trigger and play the music. If it was pushed for more than half a second,
    // this is the sign to switch tracks.
    lightSleep(LONG_PRESS_DURATION_MILLIS);
    if (!isButtonPushed()) {
      trigger(trackNumber);
    } else {
      while (isButtonPushed());
      trackNumber++;
      if (trackNumber > MAX_TRACK_NUMBER) {
        trackNumber = 1;
      }
      // Announce the name of the new track that will play
      announceTrackName(trackNumber);
    }
  }
  lightSleep(50);
}

// Return true if button is pushed
boolean isButtonPushed() {
  return digitalRead(BUTTON_PIN) == 0;
}

// Return a normalised light level 0-1
double getLightLevel() {
  int measuredLevel = analogRead(LDR_PIN);
  return constrain(1 - measuredLevel / 2500.0, 0.0, 1.0);
}

// Play an "announcer" MP3 to say which song is playing
void announceTrackName(int tracknum) {
  changeVolume(DEBUG ? DEBUG_VOLUME : ANNOUNCER_VOLUME);
  playTrack(ANNOUNCER_FOLDER, tracknum);
}

// Play an "announcer" MP3 to say we are in sensor mode
void announceSensorMode() {
  changeVolume(DEBUG ? DEBUG_VOLUME : ANNOUNCER_VOLUME);
  playTrack(ANNOUNCER_FOLDER, SENSOR_MODE_ANNOUNCER_TRACK_NUMBER);
}

// Trigger a music playing & lip syncing action
void trigger(int trackNumber) {
  // Set volume. A lower volume is set in debug mode.
  changeVolume(DEBUG ? DEBUG_VOLUME : MUSIC_VOLUME);

  // Start playing MP3
  playTrack(MUSIC_FOLDER, trackNumber);

  // Lip-sync!
  switch(trackNumber) {
    case 1:
      lipsyncPhattBass();
      break;
    case 2:
      lipsyncAllAboutThatBass();
      break;
    case 3:
      lipsyncMrScruffFish();
      break;
    case 4:
      lipsyncChopSuey();
      break;
    case 5:
      lipsyncSmellsLikeTeenSpirit();
      break;
    case 6:
      lipsyncKillingInTheName();
      break;
    case 7:
      lipsyncEnterSandman();
      break;
    case 8:
      lipsyncCloser();
      break;
    case 9:
      lipsyncIAmJustAFish();
      break;
    case 10:
      lipsyncBasketCase();
      break;
  }

  // Stop once complete
  stop();
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Warp Brothers - Phatt Bass (track number 1)
void lipsyncPhattBass() {
  lightSleep(3000); // *sirens*
  headOut();
  lightSleep(1000);
  mouthOpenFor(1000); // Listen
  lightSleep(1000);
  mouthOpenFor(500); // to the
  lightSleep(300);
  mouthOpenFor(300); // phatt
  lightSleep(200);
  flapMouthFor(3500, 250); // bass... bass... bass... bass...
  tailOut();
  mouthOpenFor(300); // bass...
  headTailRest();
  mouthOpenFor(300); // bass...
  tailOut();
  mouthOpenFor(300); // bass...
  headTailRest();
  lightSleep(300);
  flapTailFor(10800, 200); // *early 2000s techno noises*
  headOut();
  lightSleep(200);
  mouthOpenFor(600); // phatt
  lightSleep(600);
  mouthOpenFor(600); // bass
  for (int i = 0; i < 10; i++) { // rest of music
    flapTailFor(800, 200);
    flapHeadFor(800, 200);
  }
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Meghan Trainor - All About that Bass (track number 2)
void lipsyncAllAboutThatBass() {
  lightSleep(300);
  headOut();
  lightSleep(1000);
  flapMouthFor(4500, 250); // Because you know I'm all about that bass, 'bout that bass, no treble
  headTailRest();
  flapMouthFor(3500, 250); // I'm all about that bass, 'bout that bass, no treble
  headOut();
  flapMouthFor(3500, 250); // I'm all about that bass, 'bout that bass, no treble
  headTailRest();
  flapMouthFor(2500, 250); // I'm all about that bass, 'bout that
  flapMouthFor(1000, 125); // bass bass bass bass
  lightSleep(500);
  for (int i = 0; i < 12; i++) { // Yeah, it's pretty clear, I ain't no size two, but I can shake it, shake it, like I'm supposed to do
    tailOut();
    mouthOpen();
    lightSleep(150);
    mouthClose();
    lightSleep(150);
    headTailRest();
    mouthOpen();
    lightSleep(150);
    mouthClose();
    lightSleep(150);
  }
  for (int i = 0; i < 10; i++) { // 'Cause I got that boom boom that all the boys chase, and all the right junk in all the right
    headOut();
    mouthOpen();
    lightSleep(150);
    mouthClose();
    lightSleep(150);
    headTailRest();
    mouthOpen();
    lightSleep(150);
    mouthClose();
    lightSleep(150);
  }
  for (int i = 0; i < 2; i++) { // basses
    tailOut();
    mouthOpen();
    lightSleep(150);
    mouthClose();
    lightSleep(150);
    headTailRest();
  }
  lightSleep(500);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Mr Scruff - Fish (track number 3)
void lipsyncMrScruffFish() {
  lightSleep(300);
  headOut();
  mouthOpenFor(2400); // Now listen to me young fellow
  lightSleep(300);
  mouthOpenFor(2400); // What need is there for fish to sing
  lightSleep(300);
  mouthOpenFor(3000); // When I can roar and bellow?
  headTailRest();
  lightSleep(1000);
  for (int i = 0; i < 4; i++) { // Fish x8
    tailOut();
    mouthOpenFor(340);
    lightSleep(100);
    headTailRest();
    mouthOpenFor(340);
    lightSleep(100);
  }
  mouthOpenFor(340); // Fish
  lightSleep(100);
  headOut();
  lightSleep(100);
  mouthOpenFor(1300); // Eating fish
  headTailRest();
  lightSleep(400);
  for (int i = 0; i < 2; i++) { // *ununtelligible noises*
    mouthOpenFor(700);
    lightSleep(300);
  }
  lightSleep(1400);
  for (int i = 0; i < 4; i++) { // Fish x8
    tailOut();
    mouthOpenFor(340);
    lightSleep(100);
    headTailRest();
    mouthOpenFor(340);
    lightSleep(100);
  }
  mouthOpenFor(340); // Fish
  lightSleep(100);
  headOut();
  lightSleep(100);
  mouthOpenFor(1300); // Eating fish
  headTailRest();
  lightSleep(3800);
  mouthOpenFor(2600); // Fish are really (something??)
  lightSleep(1800);
  mouthOpenFor(2600); // Fish are really (something??)
  lightSleep(2000);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// System of a Down - Chop Suey (track number 4)
void lipsyncChopSuey() {
  headOut();
  mouthOpenFor(300); // Wake up
  headTailRest();
  lightSleep(100);
  mouthOpenFor(300); // *whisper* Wake up
  lightSleep(100);
  headOut();
  mouthOpenFor(1500); // Grab a brush and put a little make-up
  headTailRest();
  lightSleep(600);
  headOut();
  mouthOpenFor(1320); // Hide the scars to fade away the shake-up
  headTailRest();
  lightSleep(50);
  mouthOpenFor(500); // *whisper* Hide the scars to fade away the
  lightSleep(50);
  headOut();
  mouthOpenFor(1320); // Why'd you leave the keys upon the table?
  headTailRest();
  lightSleep(550);
  headOut();
  mouthOpenFor(1320); // Here you go create another fable
  headTailRest();
  lightSleep(50);
  mouthOpenFor(500); // You wanted to
  lightSleep(50);
  headOut();
  mouthOpenFor(1250); // Grab a brush and put a little make-up
  headTailRest();
  lightSleep(50);
  mouthOpenFor(500); // You wanted to
  lightSleep(50);
  headOut();
  mouthOpenFor(1320); // Hide the scars to fade away the shake-up
  headTailRest();
  lightSleep(50);
  mouthOpenFor(500); // You wanted to
  lightSleep(50);
  headOut();
  mouthOpenFor(1320); // Why'd you leave the keys upon the table?
  headTailRest();
  lightSleep(50);
  mouthOpenFor(500); // You wanted to
  lightSleep(50);
  mouthOpenFor(1500); // I don't think you trust
  lightSleep(1500);
  mouthOpenFor(700); // in
  lightSleep(1200);
  mouthOpenFor(800); // my
  lightSleep(1100);
  mouthOpenFor(2800); // Self-righteous suicide
  lightSleep(1000);
  mouthOpenFor(700); // I
  lightSleep(1200);
  mouthOpenFor(900); // cry
  lightSleep(850);
  mouthOpenFor(1900); // when angels deserve to
  lightSleep(50);
  headOut();
  mouthOpenFor(3200); // DDDDIIIIIIEEEEE
  headTailRest();
  lightSleep(50);
  flapTailFor(4200, 125);
  lightSleep(50);
  headOut();
  mouthOpenFor(1800); // *roar*
  headTailRest();
  lightSleep(500);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Nirvana - Smells Like Teen Spirit (track number 5)
void lipsyncSmellsLikeTeenSpirit() {
  mouthOpenFor(500); // Hello
  lightSleep(500);
  mouthOpenFor(500); // Hello
  headOut();
  lightSleep(400);
  flapMouthFor(1400, 175); // With the lights out
  lightSleep(300);
  flapMouthFor(1400, 175); // It's less dangerous
  lightSleep(400);
  headTailRest();
  lightSleep(400);
  flapMouthAndTailTogetherFor(1400, 175); // Here we are now
  lightSleep(400);
  flapMouthAndTailTogetherFor(1400, 175); // Entertain us
  lightSleep(300);
  headOut();
  lightSleep(600);
  flapMouthFor(1400, 175); // I feel stupid
  lightSleep(600);
  flapMouthFor(1400, 175); // and contagious
  lightSleep(200);
  headTailRest();
  lightSleep(500);
  flapMouthAndTailTogetherFor(1400, 175); // Here we are now
  lightSleep(500);
  flapMouthAndTailTogetherFor(1400, 175); // Entertain us
  lightSleep(700);
  flapMouthAndTailTogetherFor(1400, 175); // A mulatto
  headOut();
  lightSleep(700);
  flapMouthFor(1400, 175); // An albino
  headTailRest();
  lightSleep(700);
  flapMouthAndTailTogetherFor(1400, 175); // A mosquito
  headOut();
  lightSleep(700);
  flapMouthFor(1400, 175); // My libido
  headTailRest();
  lightSleep(800);
  mouthOpenFor(800); // Yeah
  lightSleep(500);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Rage Against the Machine - Killing in the Name (track number 6)
void lipsyncKillingInTheName() {
  headOut();
  lightSleep(250);
  for (int i = 0; i < 8; i++) {
    flapMouthFor(2250, 125); // Fuck you I won't do what you tell me
    lightSleep(400);
  }
  flapMouthFor(2250, 125); // Fuck you I won't do what you tell me
  headTailRest();
  lightSleep(2000);
  headOut();
  lightSleep(250);
  mouthOpenFor(300); // Mother
  lightSleep(200);
  mouthOpenFor(1000); // Fuckeeerrrrr
  headTailRest();
  lightSleep(1200);
  mouthOpenFor(300); // Ugh
  flapTailFor(5500, 250);
  flapTailFor(3000, 125);
  flapTailFor(500, 250);
  flapHeadFor(500, 250);
  flapTailFor(500, 250);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Metallica - Enter Sandman (track number 7)
void lipsyncEnterSandman() {
  lightSleep(400);
  flapMouthFor(3000, 300); // Hush little baby, don't say a word
  lightSleep(900);
  flapMouthFor(3000, 300); // And never mind that noise you heard
  lightSleep(1100);
  flapMouthAndTailTogetherFor(3000, 300); // It's just the beast under your bed
  lightSleep(900);
  flapMouthAndTailTogetherFor(3000, 300); // In your closet, in your head
  headOut();
  lightSleep(1000);
  flapMouthFor(1200, 300); // Exit
  mouthOpenFor(1000); // light
  lightSleep(1700);
  flapMouthFor(1200, 300); // Enter
  mouthOpenFor(1000); // night
  lightSleep(1100);
  mouthOpenFor(1000); // Grain
  lightSleep(200);
  mouthOpenFor(200); // of
  lightSleep(200);
  mouthOpenFor(2000); // sand
  lightSleep(500);
  flapMouthFor(1200, 300); // Exit
  mouthOpenFor(1000); // light
  lightSleep(1600);
  flapMouthFor(1200, 300); // Enter
  mouthOpenFor(1000); // night
  lightSleep(1500);
  mouthOpenFor(1000); // Take
  lightSleep(200);
  mouthOpenFor(200); // my
  lightSleep(200);
  mouthOpenFor(2000); // hand
  headTailRest();
  lightSleep(200);
  flapMouthFor(1600, 200); // We're off to never never
  mouthOpenFor(1500); // laaaaand
  lightSleep(1000);
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// NIN - Closer (track number 8)
void lipsyncCloser() {
  headOut();
  lightSleep(200);
  flapMouthFor(3000, 165); // I wanna fuck you like an animal
  headTailRest();
  lightSleep(200);
  flapTailFor(1600, 200); // (instrumental)
  headOut();
  lightSleep(200);
  flapMouthFor(2700, 165); // I wanna feel you from the
  mouthOpenFor(500); // in
  lightSleep(100);
  mouthOpenFor(800); // side
  headTailRest();
  lightSleep(200);
  flapTailFor(1200, 200); // (instrumental)
  headOut();
  lightSleep(200);
  flapMouthFor(3000, 165); // I wanna fuck you like an animal
  headTailRest();
  lightSleep(200);
  flapTailFor(1600, 200); // (instrumental)
  lightSleep(400);
  headOut();
  lightSleep(200);
  flapMouthFor(1800, 150); // My whole existence is
  mouthOpenFor(800); // flawed
  headTailRest();
  lightSleep(200);
  flapTailFor(2000, 200); // (instrumental)
  lightSleep(400);
  headOut();
  lightSleep(200);
  flapMouthFor(1800, 150); // You get me closer to
  mouthOpenFor(1000); // God
  headTailRest();
  lightSleep(200);
  flapTailFor(6300, 350); // (instrumental)
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// "I am Just a Fish" (track number 9)
void lipsyncIAmJustAFish() {
  headOut();
  lightSleep(200);
  mouthOpenFor(700); // Don't
  lightSleep(500);
  mouthOpenFor(700); // Cry
  lightSleep(700);
  flapMouthFor(1200, 150); // I am just a
  mouthOpenFor(500); // Fish
  headTailRest();
  lightSleep(200);
  for (int i = 0; i < 2; i++) { // (instrumental)
    tailOut();
    lightSleep(500);
    headTailRest();
    lightSleep(700);
  }
  tailOut();
  lightSleep(500);
  headTailRest();
  lightSleep(100);
  for (int i = 0; i < 3; i++) {
    headOut();
    lightSleep(400);
    flapMouthFor(1200, 150); // I am just a
    mouthOpenFor(500); // Fish
    headTailRest();
    lightSleep(200);
    for (int i = 0; i < 2; i++) { // (instrumental)
      tailOut();
      lightSleep(500);
      headTailRest();
      lightSleep(700);
    }
    tailOut();
    lightSleep(500);
    headTailRest();
    lightSleep(100);
  }
  // Outro
  flapHeadFor(2400, 600);
  for (int i = 0; i < 5; i++) {
    tailOut();
    lightSleep(500);
    headTailRest();
    lightSleep(700);
  }
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// "Basket Case" (track number 10)
void lipsyncBasketCase() {
  headOut();
  lightSleep(400);
  mouthOpenFor(300); // Do
  lightSleep(100);
  flapMouthFor(900, 150); // you have the
  mouthOpenFor(400); // time
  flapTailFor(600, 100);
  headOut();
  lightSleep(400);
  mouthOpenFor(300); // To
  lightSleep(200);
  flapMouthFor(900, 150); // listen to me
  mouthOpenFor(400); // whine
  flapTailFor(600, 100);

  headOut();
  lightSleep(400);
  flapMouthFor(2560, 160); // About nothing and everything
  mouthOpenFor(600); // all at
  lightSleep(200);
  mouthOpenFor(200); // once

  flapTailFor(1800, 100);

  headOut();
  lightSleep(400);
  mouthOpenFor(300); // I
  lightSleep(100);
  flapMouthFor(900, 150); // am one of those
  mouthOpenFor(400); // those
  flapTailFor(600, 100);
  headOut();
  lightSleep(400);
  mouthOpenFor(300); // Me-
  lightSleep(200);
  flapMouthFor(900, 150); // lodromatic
  mouthOpenFor(400); // fools
  flapTailFor(600, 100);
  
  headOut();
  lightSleep(400);
  flapMouthFor(2560, 160); // Neurotic to the bone, no
  mouthOpenFor(600); // doubt about
  lightSleep(100);
  mouthOpenFor(100); // it

  flapTailFor(3000, 100);

  headOut();
  lightSleep(400);
  flapMouthFor(1500, 120); // Sometimes I give myself
  mouthOpenFor(600); // the
  lightSleep(200);
  mouthOpenFor(500); // creeps

  flapTailFor(2500, 100);

  headOut();
  lightSleep(400);
  flapMouthFor(1740, 120); // Sometimes my mind plays tricks
  mouthOpenFor(600); // on
  lightSleep(200);
  mouthOpenFor(500); // me

  flapTailFor(1600, 100);

  headOut();
  lightSleep(400);
  flapMouthFor(1800, 150); // At all keeps adding up

  flapTailFor(600, 100);

  headOut();
  lightSleep(300);
  flapMouthFor(1500, 150); // I think I'm cracking
  mouthOpenFor(800); // up
  lightSleep(500);
  mouthOpenFor(200); // Am
  lightSleep(200);
  flapMouthFor(1500, 150); // I just paranoid
  flapMouthFor(600, 100); // Or am I just
  mouthOpenFor(800); // stoned
  headTailRest();
  lightSleep(300);

  for (int i = 0; i < 3; i++) {
    tailOut();
    lightSleep(800);
    headTailRest();
    lightSleep(800);
  }
}

// Play a specific track number from a specific folder.
void playTrack(int foldernum, int tracknum) {
  // Disable repeat
  sendCommandToMP3Player(0x11, 0);
  // Play track
  int foldertrack = (foldernum << 8) | tracknum;
  sendCommandToMP3Player(0x0f, foldertrack);
}

// Flap the head in and out for a defined time (in millis), moving it at the defined interval (in millis).
// runtime should be a multiple of interval * 2, otherwise the number of mouth movements will be rounded down.
// Used to bop to music
void flapHeadFor(int runtime, int interval) {
  int runs = runtime / interval / 2.0;
  for (int i = 0; i < runs; i++) {
    headOut();
    lightSleep(interval);
    headTailRest();
    lightSleep(interval);
  }
}

// Flap the tail in and out for a defined time (in millis), moving it at the defined interval (in millis).
// runtime should be a multiple of interval * 2, otherwise the number of mouth movements will be rounded down.
// Used to bop to music
void flapTailFor(int runtime, int interval) {
  int runs = runtime / interval / 2.0;
  for (int i = 0; i < runs; i++) {
    tailOut();
    lightSleep(interval);
    headTailRest();
    lightSleep(interval);
  }
}

// Flap the head out for a defined time (in millis), then back in for the same time. Used to bop to music.
void flapHead(int interval) {
  headOut();
  lightSleep(interval);
  headTailRest();
  lightSleep(interval);
}

// Flap the tail out for a defined time (in millis), then back in for the same time. Used to bop to music.
void flapTail(int interval) {
  tailOut();
  lightSleep(interval);
  headTailRest();
  lightSleep(interval);
}

// Bring the fish's head out
void headOut() {
  digitalWrite(HEADTAIL_MOTOR_PIN_1, LOW);
  digitalWrite(HEADTAIL_MOTOR_PIN_2, HIGH);
}

// Bring the fish's tail out
void tailOut() {
  digitalWrite(HEADTAIL_MOTOR_PIN_1, HIGH);
  digitalWrite(HEADTAIL_MOTOR_PIN_2, LOW);
}

// Put the fish head and tail back to the neutral position
void headTailRest() {
  digitalWrite(HEADTAIL_MOTOR_PIN_1, LOW);
  digitalWrite(HEADTAIL_MOTOR_PIN_2, LOW);
}

// Flap the fish's mouth for a defined time (in millis), opening and closing it at the defined interval (in millis).
// runtime should be a multiple of interval * 2, otherwise the number of mouth movements will be rounded down.
// Used to simulate singing or rapid speech.
void flapMouthFor(int runtime, int interval) {
  int runs = runtime / interval / 2.0;
  for (int i = 0; i < runs; i++) {
    mouthOpen();
    lightSleep(interval);
    mouthClose();
    lightSleep(interval);
  }
}

// Flap the fish's mouth and tail together for a defined time (in millis), opening and closing the mouth and bringing
// the tail out/in at the defined interval (in millis).
// runtime should be a multiple of interval * 2, otherwise the number of mouth movements will be rounded down.
// Used to simulate singing or rapid speech.
void flapMouthAndTailTogetherFor(int runtime, int interval) {
  int runs = runtime / interval / 2.0;
  for (int i = 0; i < runs; i++) {
    mouthOpen();
    tailOut();
    lightSleep(interval);
    mouthClose();
    headTailRest();
    lightSleep(interval);
  }
}

// Open the fish's mouth for a defined time (in millis), then close it. Used to simulate speaking a word.
void mouthOpenFor(int runtime) {
  mouthOpen();
  lightSleep(runtime);
  mouthClose();
}

// Open the fish's mouth
void mouthOpen() {
  digitalWrite(MOUTH_MOTOR_PIN_1, LOW);
  digitalWrite(MOUTH_MOTOR_PIN_2, HIGH);
}

// Close the fish's mouth
void mouthClose() {
  digitalWrite(MOUTH_MOTOR_PIN_1, HIGH);
  digitalWrite(MOUTH_MOTOR_PIN_2, LOW);
}

// Rest the fish's mouth
void mouthRest() {
  digitalWrite(MOUTH_MOTOR_PIN_1, LOW);
  digitalWrite(MOUTH_MOTOR_PIN_2, LOW);
}

// Stop the motors & music
void stop() {
  headTailRest();
  mouthRest();
  sendCommandToMP3Player(0x16, 0);
}

// Set volume to specific value
void changeVolume(int thevolume) {
  sendCommandToMP3Player(0x06, thevolume);
}

// Send a command to the MP3-TF-16P. Some commands support one or two bytes of data
// Based on docs here: https://picaxe.com/docs/spe033.pdf
// Todo: replace with https://registry.platformio.org/libraries/makuna/DFPlayer%20Mini%20Mp3%20by%20Makuna/
void sendCommandToMP3Player(byte command, int dataBytes) {
  byte commandData[10];
  byte q;
  int checkSum;
  commandData[0] = 0x7E; //Start of new command
  commandData[1] = 0xFF; //Version information
  commandData[2] = 0x06; //Data length (not including parity) or the start and version
  commandData[3] = command; //The command
  commandData[4] = 0x01; //1 = feedback
  commandData[5] = highByte(dataBytes); //High byte of the data
  commandData[6] = lowByte(dataBytes); //low byte of the data
  checkSum = -(commandData[1] + commandData[2] + commandData[3] + commandData[4] + commandData[5] + commandData[6]);
  commandData[7] = highByte(checkSum); //High byte of the checkSum
  commandData[8] = lowByte(checkSum); //low byte of the checkSum
  commandData[9] = 0xEF; //End bit
  delay(50);
  for (q = 0; q < 10; q++) {
    Serial2.write(commandData[q]);
  }
  delay(50);
}

// Replacement for "delay" that uses the ESP32 "light sleep" mode to save power
void lightSleep(int timeMs) {
  esp_sleep_enable_timer_wakeup(timeMs * 1000);
  esp_light_sleep_start();
}