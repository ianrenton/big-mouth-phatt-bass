// Big Mouth Phatt Bass control code
// by Ian Renton, 2024. CC Zero / Public Domain

// Debug LED pin
#define LED_PIN 2

// Button and sensor pins
#define BUTTON_PIN 4
#define LDR_PIN 33

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
#define MAX_TRACK_NUMBER 5
#define VOLUME 10 // Up to 30
#define MP3_PLAYER_BAUD_RATE 9600


// Includes
#include <Arduino.h>

// Function defs
void flashLED(int times);
void indicateReady();
boolean isButtonPushed();
double getLightLevel();
void trigger();
void lipsyncPhattBass();
void lipsyncAllAboutThatBass();
void lipsyncMrScruffFish();
void lipsyncChopSuey();
void lipsyncSmellsLikeTeenSpirit();
void playTrack(int tracknum);
void flapHeadFor(int runtime, int interval);
void flapTailFor(int runtime, int interval);
void flapHead(int interval);
void flapTail(int interval);
void headOut();
void tailOut();
void headTailRest();
void flapMouthFor(int runtime, int interval);
void mouthOpenFor(int runtime);
void mouthOpen();
void mouthClose();
void mouthRest();
void stop();
void changeVolume(int thevolume);
void sendCommandToMP3Player(byte command, int dataBytes);
void lightSleep(int timeMs);


// Variable defs
int trackNumber = 2;
bool sensorMode = false;
double lastSensorLightLevel = 0;


// Setup and run the program
void setup() {
  // Set up debug LED pin
  pinMode(LED_PIN, OUTPUT);

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

  // Check startup mode. If the button is held down at startup time, we go into
  // "sensor mode" to use the LDR to trigger the fish, otherwise we go into
  // normal mode where a button press triggers it.
  if (isButtonPushed()) {
    // Button held in on startup, so going into sensor mode.
    // Wait for button to be unpushed, then wait 5 seconds for the user to move away
    sensorMode = true;
    trackNumber = TRACK_NUMBER_FOR_SENSOR_MODE;
    while (isButtonPushed());

    // Record the current light level, so we don't trigger immediately
    lastSensorLightLevel = getLightLevel();

  } else {
    // Normal mode, nothing else to do here
  }

  indicateReady();
}

// Main program loop
void loop() {
  // Wait for a trigger condition, either a change in light level or
  // a button push depending on our mode.
  if (sensorMode) {
    double lightLevel = getLightLevel();
    if (lightLevel < lastSensorLightLevel - 0.04 || lightLevel > lastSensorLightLevel + 0.04) {
      trigger();
    }
    lastSensorLightLevel = lightLevel;
    lightSleep(200);

  } else if (isButtonPushed()) {
    // Not in sensor mode, and the button was pushed. If it was pushed for less than a second,
    // this is the sign to trigger and play the music. If it was pushed for more than a second,
    // this is the sign to switch tracks.
    lightSleep(1000);
    if (!isButtonPushed()) {
      trigger();
    } else {
      while (isButtonPushed());
      trackNumber++;
      if (trackNumber > MAX_TRACK_NUMBER) {
        trackNumber = 1;
      }
      flashLED(trackNumber);
    }
  }
  lightSleep(50);
}

// Indicates the program is ready by flashing the LED. 10 flashes for sensor mode, 1 in normal mode
void indicateReady() {
  flashLED(sensorMode ? 10 : trackNumber);
}

// Flash the LED high for 300ms then low for 300ms, repeated as many times as requested
void flashLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    lightSleep(300);
    digitalWrite(LED_PIN, LOW);
    lightSleep(300);
  }
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

// Trigger a music playing & lip syncing action
void trigger() {

  // Start playing MP3
  changeVolume(VOLUME);
  playTrack(trackNumber);

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
  flapMouthFor(1750, 250); // bass... bass... bass... bass...
  tailOut();
  mouthOpenFor(300); // bass...
  headTailRest();
  mouthOpenFor(300); // bass...
  tailOut();
  mouthOpenFor(300); // bass...
  headTailRest();
  lightSleep(300);
  flapTailFor(5400, 200); // *early 2000s techno noises*
  headOut();
  lightSleep(200);
  mouthOpenFor(600); // phatt
  lightSleep(600);
  mouthOpenFor(600); // bass
  for (int i = 0; i < 10; i++) { // rest of music
    flapTailFor(400, 200);
    flapHeadFor(400, 200);
  }
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Meghan Trainor - All About that Bass (track number 2)
void lipsyncAllAboutThatBass() {
  lightSleep(300);
  headOut();
  lightSleep(1000);
  flapMouthFor(2250, 250); // Because ou know I'm all about that bass, 'bout that bass, no treble
  headTailRest();
  flapMouthFor(1750, 250); // I'm all about that bass, 'bout that bass, no treble
  headOut();
  flapMouthFor(1750, 250); // I'm all about that bass, 'bout that bass, no treble
  headTailRest();
  flapMouthFor(1250, 250); // I'm all about that bass, 'bout that
  flapMouthFor(500, 125); // bass bass bass bass
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
  tailOut();
  mouthOpen(); // basses
  lightSleep(150);
  mouthClose();
  lightSleep(150);
  headTailRest();
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Mr Scruff - Fish (track number 3)
void lipsyncMrScruffFish() {
  // @todo
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// System of a Down - Chop Suey (track number 4)
void lipsyncChopSuey() {
  // @todo
}

// Lip-sync function, operating the motors in time to music. The music is already playing at
// this point so we just have to move motors accordingly. This version of the function is for:
// Nirvana - Smells Like Teen Spirit (track number 5)
void lipsyncSmellsLikeTeenSpirit() {
  // @todo
}

// Play a specific track number
void playTrack(int tracknum) {
  sendCommandToMP3Player(0x03, tracknum);
}

// Flap the head in and out for a defined time (in millis), moving it at the defined interval (in millis).
// runtime should be a multiple of interval * 2, otherwise the number of mouth movements will be rounded down.
// Used to bop to music
void flapHeadFor(int runtime, int interval) {
  int runs = runtime / interval;
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
  int runs = runtime / interval;
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
  int runs = runtime / interval;
  for (int i = 0; i < runs; i++) {
    mouthOpen();
    lightSleep(interval);
    mouthClose();
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