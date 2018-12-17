#include <Adafruit_NeoPixel.h> // Include the adafruit Neopixel Library
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include "arduino_secrets.h" 

#define PIN        12
#define LED_COUNT  14

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;
unsigned long lastMillis = 0;
const int brightness = 8;
int colorR = 0;
int colorG = 255;
int colorB = 0;
uint16_t TotalSteps = 255;  // total number of steps in the pattern
uint16_t Index;  // current step within the pattern
uint32_t Color1, Color2;  // What colors are in use (used by Fade)


WiFiUDP Udp;
IPAddress outIp(192, 168, 178, 31); // your computer's private IP
const unsigned int outPort = 6448;
const unsigned int inPort = 12000;

void connect();  // <- predefine connect() for setup()

enum mode {modeFade, modeRain, modeSnake, modeSparkle, modeCarousel};
mode currentMode = modeFade;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strips = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  setColor();
  WiFi.setPins(8,7,4,2);
  strips.begin();
  strips.setBrightness(brightness);
  strips.show();
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  
  connect();
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("\nconnected!");
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // initialise UDP:
  Udp.begin(inPort);
}

void loop() {
  // receive OSC packet via UDP:
  OSCMessage msgIN;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msgIN.fill(Udp.read());
    }
    if (!msgIN.hasError()) {
      msgIN.dispatch("/wek/outputs", changeColor);
    }
  }
}

// convert value and change LED brightness: 
void changeColor(OSCMessage &msg) {
  float redValue = msg.getFloat(0);
//  float blueValue = msg.getFloat(1);
  colorR = (int) (redValue * 255);
  colorG = 0;
  colorB = 0;
//  colorB = (int) (blueValue * 255);
  delay(10);
  setColor();
}

void setColor() {
  for(uint16_t i=0; i<strips.numPixels(); i++) {
    strips.setPixelColor(i, strips.Color(colorR, colorG, colorB));        
  }
  strips.show();
}

// Fill the dots one after the other with a color
void runFade() {
  Index = 0;
  Color1 = strips.Color(colorR, colorG, colorB);
  Color2 = strips.Color(1,1,1);
  while(Index+50 <= TotalSteps) {
    fadeCycle(); // Fading darker
    delay(3);
    Index++;
  }
  while(Index > 50) {
    fadeCycle();  // Fading brighter
    delay(3);
    Index--;
  }
}
void fadeCycle() {
  uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
  uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
  uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
  for(uint16_t i=0; i<strips.numPixels(); i++) {
    strips.setPixelColor(i, strips.Color(red, green, blue));        
  }
  strips.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(int WheelPos) {
  if(WheelPos < 85) {
    return strips.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strips.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strips.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
 
// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color){
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color){
    return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color){
    return color & 0xFF;
}
