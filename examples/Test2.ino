#include <RTC_RX8025T.h>      // https://github.com/marcinsaj/RTC_RX8025T
#include <TimeLib.h>          // https://github.com/PaulStoffregen/Time
#include <Wire.h>             // https://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <OneButton.h>        // https://github.com/mathertel/OneButton
//#include <EEPROM.h>           // https://www.arduino.cc/en/Reference/EEPROM (included with Arduino IDE)

#include <SoftwareSerial.h>


// Analog voltage monitor pins
#define ADC_VBUS_PIN  PIN_PC1
#define ADC_CS_PIN    PIN_PC0

// Pin declaration
#define EN_PIN     PIN_PB2  // SPI - SS
#define DIN_PIN    PIN_PB3  // SPI - MOSI
#define CLK_PIN    PIN_PB5  // SPI - SCK
#define CLR_PIN    PIN_PB4  // SPI - MISO - ~~~~~~~~~~~~~~~~~TO DO DESCRIPTION

// Buttons
#define B1_PIN     PIN_PC3  // Right button
#define B2_PIN     PIN_PC2  // Left button

// RTC
#define RTC_PIN    PIN_PD2  // RTC interrupt input


// LED
// Used to indicate potential hardware issues during the boot process
#define LVCC_PIN   PIN_PD7  // VCC
#define LVFD_PIN   PIN_PD6  // VFD
#define LDIAG_PIN  PIN_PD5  // DIAG

// Power switches
#define ENA_PIN   PIN_PB1  // High side power switch TPS1H100B
#define ENB_PIN   PIN_PB0  // Last stand power switch


int adcVbusValue = 0;
int adcCsValue = 0;


/////////////////////////////////////////////////////////////////////////
#define RX_PIN PIN_PD0   // nieużywany
#define TX_PIN PIN_PB2   // PB1

SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX
/////////////////////////////////////////////////////////////////////////





void setup() 
{
  pinMode(ADC_VBUS_PIN, INPUT);
  pinMode(ADC_CS_PIN, INPUT);

  pinMode(RTC_PIN, INPUT_PULLUP);

  pinMode(LVCC_PIN, OUTPUT);
  pinMode(LVFD_PIN, OUTPUT);
  pinMode(LDIAG_PIN, OUTPUT);

  digitalWrite(LVCC_PIN, LOW);
  digitalWrite(LVFD_PIN, LOW);
  digitalWrite(LDIAG_PIN, LOW);

  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);



  mySerial.begin(9600);   // prędkość transmisji
  delay(3000);
}

void loop() {
  mySerial.println(F("Hello from ATmega328!"));
  adcVbusValue = analogRead(ADC_VBUS_PIN);
  mySerial.print(F("ADC_VBUS:"));
  mySerial.println(adcVbusValue);
  adcCsValue = analogRead(ADC_CS_PIN);
  mySerial.print(F("ADC_CS:"));
  mySerial.println(adcCsValue);



  delay(1000); // wysyłaj co 1s
}
