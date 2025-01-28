/*
    ESP32 NTP Nixie Tube Clock Program

    NixieTubeShield.h - Interface to Nixie Tube Shield
*/

#ifndef NIXIE_TUBE_SHIELD_H
#define NIXIE_TUBE_SHIELD_H

#include <ClickButton.h>
#include <Wire.h>
#include "LEDControl.h"
#include "Tone.h"

// ***************************************************************
// ESP32 Pin Configuration
// ***************************************************************

//#define ESP32_WEMOS_D1_R32

#ifdef ESP32_WEMOS_D1_R32
    // Wemos D1 R32 Pin Definitions
    #define LED_GREEN    27
    #define LED_RED      13
    #define LED_BLUE     25
    #define LATCH_ENABLE SS  // SS (GPIO5)
    #define MODE_BUTTON  2 // Can cause a boot loop since GPIO2 can trigger at boot which fires up the if with SHIELD.isSetButtonLongClicked() which causes esp_restart(); - can be commented out as workaround 
    #define UP_BUTTON     35
    #define DOWN_BUTTON  4
    #define BUZZER_PIN    26

    // Not used in this Sctipt
    // 14 - TMP
    // 16 - SHDN
    // 17 - IR
    // 34 - LS
#else
    // Normal ESP32 Pin Definitions
    #define LED_GREEN    A16
    #define LED_RED      A14
    #define LED_BLUE     A13
    #define HV_ENABLE    17  //TODO
    #define LATCH_ENABLE SS
    #define NEON_DOTS    A18
    #define MODE_BUTTON  A15
    #define UP_BUTTON     16
    #define DOWN_BUTTON  A10
    #define BUZZER_PIN    A4
#endif

#define DS1307_ADDRESS 0x68

// ***************************************************************
// Digit Data Definitions
// Each digit, except blank, has a single active low bit in a
// field of 10 bits
// ***************************************************************
#define DIGIT_0      0
#define DIGIT_1      1
#define DIGIT_2      2
#define DIGIT_3      3
#define DIGIT_4      4
#define DIGIT_5      5
#define DIGIT_6      6
#define DIGIT_7      7
#define DIGIT_8      8
#define DIGIT_9      9
#define DIGIT_BLANK  0xFF

#define BLANK_DIGIT  10

#define UpperDotsMask 0x80000000
#define LowerDotsMask 0x40000000

ClickButton setButton(MODE_BUTTON, LOW, CLICKBTN_PULLUP);
ClickButton upButton(UP_BUTTON, LOW, CLICKBTN_PULLUP);
ClickButton downButton(DOWN_BUTTON, LOW, CLICKBTN_PULLUP);

// Class Definition
class NixieTubeShield : public LEDControl {
  unsigned int SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

  public:

    // Class Constructor
    NixieTubeShield() : LEDControl(LED_RED, LED_GREEN, LED_BLUE) {

      // Check if we're using Wemos D1 R32 or normal ESP32
      #ifndef ESP32_WEMOS_D1_R32
          // Normal ESP32 specific setup
          // Setup output pins
          pinMode(HV_ENABLE,    OUTPUT);
          pinMode(NEON_DOTS,    OUTPUT);
          //pinMode(NEON_DOTS_LOWER,    OUTPUT);
        
          // Set default outputs
          digitalWrite(HV_ENABLE,    LOW);
          digitalWrite(NEON_DOTS,    LOW);
      #endif

      // Setup output pins
      pinMode(LATCH_ENABLE, OUTPUT);
      pinMode(MODE_BUTTON,  INPUT_PULLUP);
      pinMode(UP_BUTTON,  INPUT_PULLUP);
      pinMode(DOWN_BUTTON,  INPUT_PULLUP);
        
      // Setup output pins
      digitalWrite(LATCH_ENABLE, LOW);

      setButton.debounceTime   = 20;   // Debounce timer in ms
      setButton.multiclickTime = 30;  // Time limit for multi clicks
      setButton.longClickTime  = 2000; // time until "held-down clicks" register

      upButton.debounceTime   = 20;   // Debounce timer in ms
      upButton.multiclickTime = 30;  // Time limit for multi clicks
      upButton.longClickTime  = 2000; // time until "held-down clicks" register

      downButton.debounceTime   = 20;   // Debounce timer in ms
      downButton.multiclickTime = 30;  // Time limit for multi clicks
      downButton.longClickTime  = 2000; // time until "held-down clicks" register

      // Initialize digit storage to all ones so all digits are off
      for (int i = 0; i < 6; i++) {
        digits[i] = DIGIT_BLANK;
      }
    }

    #ifndef ESP32_WEMOS_D1_R32
        // High voltage control
        void hvEnable(boolean state) {
          digitalWrite(HV_ENABLE, state ? HIGH : LOW);
        }
    #endif

    // Neon lamp control
    void dotsEnable(boolean state) {
      #ifndef ESP32_WEMOS_D1_R32
        digitalWrite(NEON_DOTS, state ? HIGH : LOW);
      #endif
      dotsEnabled = state;
    }

    // Set the NX1 (most significant) digit
    void setNX1Digit(int d) {
      digits[5] = NUMERIC_DIGITS[d];
    }

    // Set the NX2 digit
    void setNX2Digit(int d) {
      digits[4] = NUMERIC_DIGITS[d];
    }

    // Set the NX3 digit
    void setNX3Digit(int d) {
      digits[3] = NUMERIC_DIGITS[d];
    }

    // Set the NX4 digit
    void setNX4Digit(int d) {
      digits[2] = NUMERIC_DIGITS[d];
    }

    // Set the NX5 digit
    void setNX5Digit(int d) {
      digits[1] = NUMERIC_DIGITS[d];
    }

    // Set the NX6 (least significant) digit
    void setNX6Digit(int d) {
      digits[0] = NUMERIC_DIGITS[d];
    }

    void show() {
      digitalWrite(LATCH_ENABLE, LOW);    // allow data input (Transparent mode)
      unsigned long Var32=0;
       
      //-------- REG 1 ----------------------------------------------- 
      Var32=0;
     
      Var32|=(unsigned long)(SymbolArray[digits[0]])<<20; // s2
      Var32|=(unsigned long)(SymbolArray[digits[1]])<<10; //s1
      Var32|=(unsigned long) (SymbolArray[digits[2]]); //m2
    
      if (dotsEnabled) Var32|=LowerDotsMask;
      else  Var32&=~LowerDotsMask;
  
      if (dotsEnabled) Var32|=UpperDotsMask;
      else Var32&=~UpperDotsMask;  

      SPI.transfer(Var32>>24);
      SPI.transfer(Var32>>16);
      SPI.transfer(Var32>>8);
      SPI.transfer(Var32);
    
     //-------------------------------------------------------------------------
    
     //-------- REG 0 ----------------------------------------------- 
      Var32=0;
      
      Var32|=(unsigned long)(SymbolArray[digits[3]])<<20; // m1
      Var32|=(unsigned long)(SymbolArray[digits[4]])<<10; //h2
      Var32|=(unsigned long)SymbolArray[digits[5]]; //h1
    
      if (dotsEnabled) Var32|=LowerDotsMask;
      else  Var32&=~LowerDotsMask;
  
      if (dotsEnabled) Var32|=UpperDotsMask;
      else Var32&=~UpperDotsMask;  

      SPI.transfer(Var32>>24);
      SPI.transfer(Var32>>16);
      SPI.transfer(Var32>>8);
      SPI.transfer(Var32);
    
      digitalWrite(LATCH_ENABLE, HIGH);     // latching data 
    }

    // Do anti-poisoning routine and then turn off all tubes
    void doAntiPoisoning(int flickerMode) {
      bool dots = false;
      dotsEnable(false);

      if(flickerMode){
        // Arrays to track current and target digits for each tube
        byte currentDigits[6] = {0, 0, 0, 0, 0, 0};
        byte targetDigits[6] = {0, 0, 0, 0, 0, 0};

        // Randomize target digits for each tube
        for (int i = 0; i < 6; i++) {
            targetDigits[i] = random(0, 10); // Target random digit from 0-9
        }

        for (int iteration = 0; iteration < 60; iteration++) { // Run for 3 seconds (60 x 50ms)
            for (int i = 0; i < 6; i++) {
                // Increment current digit towards the target digit, with wrap-around
                if (currentDigits[i] != targetDigits[i]) {
                    currentDigits[i] = (currentDigits[i] + 1) % 10;
                } else {
                    // Assign a new random target digit when the current matches the target
                    targetDigits[i] = random(0, 10);
                }
            }

            // Set each digit of the nixie tubes
            setNX1Digit(currentDigits[0]);
            setNX2Digit(currentDigits[1]);
            setNX3Digit(currentDigits[2]);
            setNX4Digit(currentDigits[3]);
            setNX5Digit(currentDigits[4]);
            setNX6Digit(currentDigits[5]);

            // Make the changes visible
            show();

            // Small delay for flickering effect
            delay(50);

            // Alternate the dots
            dots = !dots;
            dotsEnable(dots);
        }
      } else {
        for (int j = 0; j < 4; j++) {
          for (int i = 0; i < 11; i++) {
            setNX1Digit(i);
            setNX2Digit(i);
            setNX3Digit(i);
            setNX4Digit(i);
            setNX5Digit(i);
            setNX6Digit(i);

            // Make the changes visible
            show();

            // Small delay
            delay(500);

            setLEDColor(i%3==0?255:0, i%3==1?255:0, i%3==2?255:0);
            dots = !dots;
            dotsEnable(dots);
          }
        }
      }
    }

    void processButtons() {
        setButton.Update();
        upButton.Update();
        downButton.Update();
    }

    bool isSetButtonClicked() {
      return (setButton.clicks > 0);
    }

    bool isSetButtonLongClicked() {
      return (setButton.clicks < 0);
    }

    bool isUpButtonClicked() {
      return (upButton.clicks > 0);
    }

    bool isUpButtonLongClicked() {
      return (upButton.clicks < 0);
    }

    bool isDownButtonClicked() {
      return (downButton.clicks > 0);
    }

    bool isDownButtonLongClicked() {
      return (downButton.clicks < 0);
    }

    void getRTCTime(tmElements_t &m) {
      Wire.beginTransmission(DS1307_ADDRESS);
      Wire.write(zero);
      Wire.endTransmission();
    
      Wire.requestFrom(DS1307_ADDRESS, 7);

      m.Second = bcdToDec(Wire.read());
      m.Minute = bcdToDec(Wire.read());
      m.Hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
      m.Wday = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
      m.Day = bcdToDec(Wire.read());
      m.Month = bcdToDec(Wire.read());
      m.Year = bcdToDec(Wire.read());
    }

    void setRTCDateTime(const tmElements_t &m) {
      Wire.beginTransmission(DS1307_ADDRESS);
      Wire.write(zero); //stop Oscillator
    
      Wire.write(decToBcd(m.Second));
      Wire.write(decToBcd(m.Minute));
      Wire.write(decToBcd(m.Hour));
      Wire.write(decToBcd(m.Wday));
      Wire.write(decToBcd(m.Day));
      Wire.write(decToBcd(m.Month));
      Wire.write(decToBcd(m.Year));
    
      Wire.write(zero); //start
    
      Wire.endTransmission();
    }

  private:
    byte decToBcd(byte val) {
      // Convert normal decimal numbers to binary coded decimal
      return ( (val / 10 * 16) + (val % 10) );
    }
    
    byte bcdToDec(byte val)  {
      // Convert binary coded decimal to normal decimal numbers
      return ( (val / 16 * 10) + (val % 16) );
    }

    // Private data
    // Array of codes for each nixie digit
    int NUMERIC_DIGITS[11] = {
      DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4,
      DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9,
      DIGIT_BLANK
    };

    byte zero = 0x00; //workaround for issue #527
    int RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year, RTC_day_of_week;

    // Storage for codes for each nixie digit
    // Digit order is: NX6, NX5, NX4, NX3, NX2, NX1
    uint16_t digits[6];

    bool dotsEnabled = false;
};

#endif
