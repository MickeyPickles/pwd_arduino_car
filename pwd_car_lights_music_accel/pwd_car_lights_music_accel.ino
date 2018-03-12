#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 11
#define NUMPIXELS 16
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VS1053.h>

Adafruit_MMA8451 mma = Adafruit_MMA8451();

int ACCELEROMETER_THRESHOLD = 1;
int SHOW_LENGTH = 5000;
unsigned long lastShowTime = 0;
float restingXValue = 0.0;

float restingDifference = 0.0;
unsigned long lastAccelerometerSample = millis();
unsigned long ACCELEREROMETER_SAMPLE_RATE = 500;




//unsigned long elapsedTime;
bool isShowPlaying = false;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    6      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
// These are common pins between breakout and shield
#define CARDCS 5     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 1   
Adafruit_VS1053_FilePlayer musicPlayer =   Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  setupNeopixels();
  setupAccelerometer();
  setupSoundplayer();
}

void setupNeopixels() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


void setupAccelerometer() {
//  Serial.begin(9600);
//  while (!Serial) ;
  pinMode(13, OUTPUT);
  Serial.println("Adafruit MMA8451 test44");
  

  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
}


void setupSoundplayer() {
  //Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20,20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  // Play one file, don't return until complete
//  Serial.println(F("Playing track 001"));
//  musicPlayer.playFullFile("track001.mp3");
  // Play another file in the background, REQUIRES interrupts!
  Serial.println(F("Playing track 002"));
  musicPlayer.startPlayingFile("track001.mp3");
  musicPlayer.pausePlaying(true);
}

void loop() {



//
  if(isShowPlaying) {
//    unsigned long elapsedTime = millis() - lastShowTime;
//    if(elapsedTime >= SHOW_LENGTH) {
//      //time to stop show
//      musicPlayer.startPlayingFile("track001.mp3");
//      musicPlayer.pausePlaying(true);
//      isShowPlaying = false;
//    }
  } else {
      unsigned long elapsedTime = millis() - lastAccelerometerSample;
      if(elapsedTime >= ACCELEREROMETER_SAMPLE_RATE) {
        lastAccelerometerSample = millis();
        if(checkIfNeedToStartShow()) {
          restingXValue = 0.0;
          Serial.println("start the show!!!!");
          //isShowPlaying = true;
          musicPlayer.pausePlaying(false);
          lastShowTime = millis();
          theaterChaseRainbow(50);
          musicPlayer.startPlayingFile("track001.mp3");
          musicPlayer.pausePlaying(true);
        }
      } 
  }

  //delay(10);
}

boolean checkIfNeedToStartShow() {
  // Read the 'raw' data in 14-bit counts
  mma.read();


  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: \t"); Serial.print(event.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(event.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(event.acceleration.z); Serial.print("\t");
  Serial.println("m/s^2 ");

 
  if(restingXValue == 0.0) {
    restingXValue = abs(event.acceleration.x);
  }

  restingDifference = abs(event.acceleration.x) - restingXValue;

  Serial.print("restingDifference = ");
  Serial.println(restingDifference);

  return abs(restingDifference) > ACCELEROMETER_THRESHOLD;

//  if(abs(restingDifference) > ACCELEROMETER_THRESHOLD) 
//  {
//        int elapsedTime = millis() - lastShowTime;
//        if(elapsedTime > SHOW_DELAY) {
//
//          //theaterChaseRainbow(50);
//          restingXValue = 0.0;
//        }
//  }
}

void turnOffAllPixels() {
      for(int i=0;i<NUMPIXELS;i++){
            // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
               strip.setPixelColor(i, strip.Color(0,0,0)); // off       
      }      
      strip.show();
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j+=5) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
    turnOffAllPixels();
    lastShowTime = millis();
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
