
#include <Adafruit_GFX.h>   
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <SPI.h>
#include <mySD.h>
#include "BluetoothSerial.h"

//define pins of TFT screen
#define TFT_CS     5
#define TFT_RST    4  
#define TFT_DC     2
#define TFT_SCLK   18
#define TFT_MOSI   23
//define pins of SD card slot
#define SD_CS    26
#define SD_MOSI  14
#define SD_MISO  13
#define SD_SCK   27

//low range of the sensor (this will be blue on the screen)
#define MINTEMP 22

//high range of the sensor (this will be red on the screen)
#define MAXTEMP 34

//funtion prototype
void printDirectory(File dir, int numTabs);

//the colors we will be using
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

// initial tft display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// initial Bluetooth
BluetoothSerial ESP_BT;

//global variable of TFT Display
Adafruit_AMG88xx amg;
unsigned long delayTime;
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
uint16_t displayPixelWidth, displayPixelHeight;

//global variable of SD card
File root;

//global variable of bluetooth
int incoming;

//global variable
long lastSave;
long saveDelay=2000;

void setup() {
  Serial.begin(115200);

  ESP_BT.begin("ESP32_NSC21_PT1"); 

  tft.initR(INITR_144GREENTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);

  displayPixelWidth = tft.width() / 8;
  displayPixelHeight = tft.height() / 8;

  tft.setRotation(3);
    
  bool status;
    
  // default settings
  status = amg.begin();
  if (!status) {
      Serial.println("Could not find a valid AMG88xx sensor, check wiring!");
      while (1);
  }
  delay(100); // let sensor boot up

  // begin SD card
  if (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_SCK)) {
    Serial.println("initialization SD failed!");
    return;
  }
  Serial.println("initialization SD done.");
  root = SD.open("/");
  printDirectory(root, 0);
  Serial.println("done!");
  root = SD.open("test.txt", FILE_WRITE);
  root.println("start new record");
  root.close();
  lastSave = millis();
}

void loop() {
  //read all the pixels
  amg.readPixels(pixels);

  if (ESP_BT.available()) {
    incoming = ESP_BT.read(); 
    if(incoming==49){
      if((millis()-lastSave)>=saveDelay){
        root = SD.open("test.txt", FILE_WRITE);
        if(root){
          for(int j=1; j<=AMG88xx_PIXEL_ARRAY_SIZE; j++){
            root.print(String(pixels[j-1]));
            root.print(",");
            if(j % 8 == 0){
              root.println("");
            }
          }
          root.println("");
          root.close();
          Serial.println("SD Saved");
          ESP_BT.println("SD Saved");
          lastSave = millis();
        }
        else{
          Serial.println("SD save failed");
          ESP_BT.println("SD save failed");
        }
      }
    }
  }
  
  // Display draw
  for(int i=0; i<AMG88xx_PIXEL_ARRAY_SIZE; i++){
    uint8_t colorIndex = map(pixels[i], MINTEMP, MAXTEMP, 0, 255);
    colorIndex = constrain(colorIndex, 0, 255);
    tft.fillRect(displayPixelHeight * floor(i / 8), displayPixelWidth * (i % 8),
        displayPixelHeight, displayPixelWidth, camColors[colorIndex]);
  }
}

void printDirectory(File dir, int numTabs) {
  // Begin at the start of the directory
  dir.rewindDirectory();
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');   // we'll have a nice indentation
     }
     // Print the 8.3 name
     Serial.print(entry.name());
     // Recurse for directories, otherwise print the file size
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