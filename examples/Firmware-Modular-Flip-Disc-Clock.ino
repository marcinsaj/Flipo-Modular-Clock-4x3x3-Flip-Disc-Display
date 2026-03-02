/* Modular Flip-Disc Clock – Programming and Firmware

MiniCore Configuration
Before burning the bootloader, install MiniCore using the following Boards Manager URL:
https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json

In Arduino IDE → Tools, configure the settings as follows:
Board: ATmega328P / ATmega328PA
Clock: External 12 MHz
Bootloader: Yes (UART0)
BOD: 2.7V
EEPROM: Retained
Compiler LTO: Enabled
Baud Rate: Default
Variant: 328P / 328PA

⚠ Make sure the clock is set to External 12 MHz. ⚠

Programming Procedure:
1. Select the correct Programmer: AVR ISP MKII (recommended).
2. Click Burn Bootloader to set the fuse bits and configure the microcontroller.
3. After completion, select Upload Using Programmer to flash the firmware. */

#include <RTC_RX8025T.h>  // https://github.com/marcinsaj/RTC_RX8025T
#include <TimeLib.h>      // https://github.com/PaulStoffregen/Time
#include <Wire.h>         // https://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <OneButton.h>    // https://github.com/mathertel/OneButton
#include <EEPROM.h>       // https://www.arduino.cc/en/Reference/EEPROM (included with Arduino IDE)
#include <SPI.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>


#define DEBUG 0

#if DEBUG
#include <SoftwareSerial.h>

const int RX_PIN = PIN_PD0;
const int TX_PIN = PIN_PD1;

SoftwareSerial mySerial(RX_PIN, TX_PIN);
#endif

// ============================================================================
// FLIP-DISC DISPLAY DATA (PROGMEM)
// ============================================================================
/* Refers to 4x3x3 display.
 * The array contains the addresses of the control outputs corresponding
 * to the setting of the discs to the "color" side.
 * 7,6,5,4,3,2,1,0  15,14,13,12,11,10,9,8  23,22,21,20,19,18,17,16
 *
 *    36 35 34   27 26 25
 *    33 32 31   24 23 22
 *    30 29 28   21 20 19
 *
 *    18 17 16    9  8  7
 *    15 14 13    6  5  4
 *    12 11 10    3  2  1 */
static const uint8_t resetDisc[36][3] PROGMEM =
{
  { 0b00000000, 0b00000000, 0b10000001 },
  { 0b00000000, 0b00000000, 0b00100001 },
  { 0b00000000, 0b00000010, 0b00000001 },

  { 0b00000001, 0b00000000, 0b10000000 },
  { 0b00000001, 0b00000000, 0b00100000 },
  { 0b00000001, 0b00000010, 0b00000000 },

  { 0b00000000, 0b10000000, 0b10000000 },
  { 0b00000000, 0b10000000, 0b00100000 },
  { 0b00000000, 0b10000010, 0b00000000 },


  { 0b00000000, 0b00010000, 0b00000001 },
  { 0b00010000, 0b00000000, 0b00000001 },
  { 0b00000010, 0b00000000, 0b00000001 },

  { 0b00000001, 0b00010000, 0b00000000 },
  { 0b00010001, 0b00000000, 0b00000000 },
  { 0b00000011, 0b00000000, 0b00000000 },

  { 0b00000000, 0b10010000, 0b00000000 },
  { 0b00010000, 0b10000000, 0b00000000 },
  { 0b00000010, 0b10000000, 0b00000000 },


  { 0b00000000, 0b00100000, 0b10000000 },
  { 0b00000000, 0b00100000, 0b00100000 },
  { 0b00000000, 0b00100010, 0b00000000 },

  { 0b00000000, 0b00000000, 0b10000100 },
  { 0b00000000, 0b00000000, 0b00100100 },
  { 0b00000000, 0b00000010, 0b00000100 },

  { 0b00100000, 0b00000000, 0b10000000 },
  { 0b00100000, 0b00000000, 0b00100000 },
  { 0b00100000, 0b00000010, 0b00000000 },


  { 0b00000000, 0b00110000, 0b00000000 },
  { 0b00010000, 0b00100000, 0b00000000 },
  { 0b00000010, 0b00100000, 0b00000000 },

  { 0b00000000, 0b00010000, 0b00000100 },
  { 0b00010000, 0b00000000, 0b00000100 },
  { 0b00000010, 0b00000000, 0b00000100 },

  { 0b00100000, 0b00010000, 0b00000000 },
  { 0b00110000, 0b00000000, 0b00000000 },
  { 0b00100010, 0b00000000, 0b00000000 }
};

static const uint8_t setDisc[36][3] PROGMEM =
{
  { 0b00000000, 0b00000000, 0b01000010 },
  { 0b00000000, 0b00000000, 0b00010010 },
  { 0b00000000, 0b00000100, 0b00000010 },

  { 0b10000000, 0b00000000, 0b01000000 },
  { 0b10000000, 0b00000000, 0b00010000 },
  { 0b10000000, 0b00000100, 0b00000000 },

  { 0b00000000, 0b00000001, 0b01000000 },
  { 0b00000000, 0b00000001, 0b00010000 },
  { 0b00000000, 0b00000101, 0b00000000 },


  { 0b00000000, 0b00001000, 0b00000010 },
  { 0b00001000, 0b00000000, 0b00000010 },
  { 0b00000100, 0b00000000, 0b00000010 },

  { 0b10000000, 0b00001000, 0b00000000 },
  { 0b10001000, 0b00000000, 0b00000000 },
  { 0b10000100, 0b00000000, 0b00000000 },

  { 0b00000000, 0b00001001, 0b00000000 },
  { 0b00001000, 0b00000001, 0b00000000 },
  { 0b00000100, 0b00000001, 0b00000000 },


  { 0b00000000, 0b01000000, 0b01000000 },
  { 0b00000000, 0b01000000, 0b00010000 },
  { 0b00000000, 0b01000100, 0b00000000 },

  { 0b00000000, 0b00000000, 0b01001000 },
  { 0b00000000, 0b00000000, 0b00011000 },
  { 0b00000000, 0b00000100, 0b00001000 },

  { 0b01000000, 0b00000000, 0b01000000 },
  { 0b01000000, 0b00000000, 0b00010000 },
  { 0b01000000, 0b00000100, 0b00000000 },


  { 0b00000000, 0b01001000, 0b00000000 },
  { 0b00001000, 0b01000000, 0b00000000 },
  { 0b00000100, 0b01000000, 0b00000000 },

  { 0b00000000, 0b00001000, 0b00001000 },
  { 0b00001000, 0b00000000, 0b00001000 },
  { 0b00000100, 0b00000000, 0b00001000 },

  { 0b01000000, 0b00001000, 0b00000000 },
  { 0b01001000, 0b00000000, 0b00000000 },
  { 0b01000100, 0b00000000, 0b00000000 }
};

/* Compressed array of all bits corresponding to discs states for selected dice symbol 0-9.
 * 4x3x3 display contains 4 sections 3x3, the discs in each section are numbered in the same way
 * so the array only needs to contain the disc numbers of one section.
 * Disc numbers in individual bytes: 7,6,5,4,3,2,1,0  15,14,13,12,11,10,9,8 */
static const uint8_t diceArray[12][2] PROGMEM =
{
  {0b00000000, 0b00000000}, // 0
  {0b00010000, 0b00000000}, // 1
  {0b00000001, 0b00000001}, // 2
  {0b01010100, 0b00000000}, // 3
  {0b01000101, 0b00000001}, // 4
  {0b01010101, 0b00000001}, // 5
  {0b01101101, 0b00000001}, // 6
  {0b01111101, 0b00000001}, // 7
  {0b11101111, 0b00000001}, // 8
  {0b11111111, 0b00000001}, // 9
  {0b00000000, 0b00000000}, // 10/CAD - clear all discs
  {0b11111111, 0b00000001}  // 11/SAD - set all discs
};

/* Compressed array of all bits corresponding to discs states for selected digits symbol 0-9.
 * 4x3x3 display contains 4 sections 3x3, the discs in each section are numbered in the same way
 * so the array only needs to contain the disc numbers of one section.
 * Disc numbers in individual bytes: 7,6,5,4,3,2,1,0  15,14,13,12,11,10,9,8 */
static const uint8_t numbArray[12][2] PROGMEM =
{
  {0b11101111, 0b00000001}, // 0
  {0b10010111, 0b00000001}, // 1
  {0b10010011, 0b00000001}, // 2
  {0b11011111, 0b00000001}, // 3
  {0b01111001, 0b00000001}, // 4
  {0b11010110, 0b00000000}, // 5
  {0b00111111, 0b00000001}, // 6
  {0b11001001, 0b00000001}, // 7
  {0b11111111, 0b00000000}, // 8
  {0b11111001, 0b00000001}, // 9
  {0b00000000, 0b00000000}, // 10/CAD - clear all discs
  {0b11111111, 0b00000001}  // 11/SAD - set all discs
};

// ============================================================================
// CONSTANTS
// ============================================================================

// Delay between flipping consecutive discs
// Best visual effect in range 0-50ms, maximum value 255
static const uint8_t flip_disc_delay = 30;

// Codenames for the flip-disc display
static const uint8_t CAD = 10; // Clear all discs
static const uint8_t SAD = 11; // Set all discs

// Voltage monitoring thresholds
const int LOWER_THRESHOLD = 200;    // 180 for 10V lower threshold (in ADC units)
const int UPPER_THRESHOLD = 250;   // upper threshold (in ADC units)
const int REQUIRED_OK_SAMPLE = 5;  // number of consecutive in-range readings required
const int SAMPLE_DELAY_MS = 10;    // delay between measurements

// Error / state codes
const int ERROR_NONE = 0;
const int ERROR_VBUS_LOW = 1;   // Vbus <= LOWER_THRESHOLD
const int ERROR_VBUS_HIGH = 2;  // Vbus >= UPPER_THRESHOLD
const int ERROR_VFD_NO_DISPLAY = 3;
const int ERROR_VFD_OVER_CURRENT = 4;

// Aliases for individual option settings
static const uint8_t HR12 = 12;  // Display time in 12 hour format
static const uint8_t HR24 = 24;  // Display time in 24 hour format
static const uint8_t NUMB = 0;   // Display digits in normal mode
static const uint8_t DICE = 1;   // Display digits in dice mode
static const uint8_t E00M = 0;   // Random Effect turn off
static const uint8_t E01M = 1;   // Random Effect every 1 minute
static const uint8_t E60M = 2;   // Random Effect every full hour

// Number of middle sequence cycles (random ON -> random OFF) depending on mode:
// - Every minute (E01M): 1 middle cycle
// - Every hour (E60M): 2 middle cycles
//
// Full effect sequence:
// 1. Randomly turn OFF only currently-ON discs (old time disappears)
// 2. Middle sequence: random ON all -> random OFF all (repeated N times)
// 3. Randomly turn ON only discs needed to display the new time
static const uint8_t EFFECT_MINUTE = 1;
static const uint8_t EFFECT_HOUR = 2;

// Eeprom addresses where settings are stored
static const uint16_t ee_time_format_address = 0;       // Time format 12/24 hour
static const uint16_t ee_symbol_type_address = 1;       // Digits or Dice symbols
static const uint16_t ee_effect_interval_address = 2;   // Random Effect 1min/1hour interval

// ============================================================================
// PIN DECLARATIONS
// ============================================================================

// Analog voltage monitor pins
const int ADC_VBUS_PIN = A7;
const int ADC_VOUT_PIN = A6;

// SPI pins
const int DIN_PIN = PIN_PB3;  // SPI - MOSI
const int EN_PIN  = PIN_PB2;  // SPI - SS
const int CLK_PIN = PIN_PB5;  // SPI - SCK
const int CLR_PIN = PIN_PB1;  // SPI - MISO

// Buttons
const int BT1_PIN = PIN_PD4;  // Right button
const int BT2_PIN = PIN_PC3;  // Left button

// RTC
const int INT_RTC = PIN_PD2;  // RTC interrupt input

// Fault input
const int INT_FLT = PIN_PD3;  // Diagnostic input

// LED - Used to indicate potential hardware issues during the boot process
const int LVCC_PIN = PIN_PD6;   // VCC
const int LVFD_PIN = PIN_PD5;   // VFD
const int LDIAG_PIN = PIN_PD7;  // DIAG

// Power switches
const int ENA_PIN = PIN_PC0;  // High side power switch TPS1H100B
const int ENB_PIN = PIN_PB0;  // Last stand power switch

// Discharge
const int DIS_PIN = PIN_PC1;

// Power ON
const int PWR_PIN = PIN_PC2;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Time structure and variables
tmElements_t tm;
uint8_t currentTimeDigit[4] = {0, 0, 0, 0};
uint16_t hour_time = 0;
uint16_t minute_time = 0;

// OneButton instances
OneButton button1(BT1_PIN, true, true);
OneButton button2(BT2_PIN, true, true);

// Button press status flags
bool shortPressButton1Status = false;
bool shortPressButton2Status = false;
bool longPressButton1Status = false;
bool longPressButton2Status = false;
bool doubleClickButton1Status = false;

// Settings and status flags
bool powerClockStatus = false;
bool timeSettingsStatus = false;
bool effectSettingsStatus = false;

// Settings values (stored in eeprom, read during setup)
uint8_t time_format = 12;     // HR12 - Time format
uint8_t symbol_type = 0;      // NUMB - Numbers
uint8_t effect_interval = 1;  // Random Effect every 1 minute

// Interrupt flags
volatile bool interruptFltStatus = false;
volatile bool interruptRtcStatus = false;

// Boolean state arrays for all 36 discs (true = ON, false = OFF).
// current_state[] - current display state (old time, before update)
// target_state[]  - target display state (new time, after update)
// Populated by ComputeDisplayState(), used by the visual effect.
bool current_state[36];
bool target_state[36];

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

void fltInterruptISR(void)
{
  interruptFltStatus = true;
  // Feature planned for future implementation
}

void rtcInterruptISR(void)
{
  interruptRtcStatus = true;
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void ShortPressButton1(void)
{
  shortPressButton1Status = true;
}

void ShortPressButton2(void)
{
  shortPressButton2Status = true;
}

void LongPressButton1(void)
{
  longPressButton1Status = true;
}

void LongPressButton2(void)
{
  longPressButton2Status = true;
}

void DoubleClickButton1(void)
{
  doubleClickButton1Status = true;
}

void ClearPressButtonFlags(void)
{
  shortPressButton1Status = false;
  shortPressButton2Status = false;
  longPressButton1Status = false;
  longPressButton2Status = false;
  doubleClickButton1Status = false;
}

// ============================================================================
// SETUP
// ============================================================================

void setup()
{
  wdt_disable();

  #if DEBUG
  mySerial.begin(9600);  // baud rate
  #endif

  // Set pin modes
  pinMode(PWR_PIN, OUTPUT);
  pinMode(DIS_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(CLR_PIN, OUTPUT);
  pinMode(LVCC_PIN, OUTPUT);
  pinMode(LVFD_PIN, OUTPUT);
  pinMode(LDIAG_PIN, OUTPUT);
  pinMode(ADC_VBUS_PIN, INPUT);
  pinMode(ADC_VOUT_PIN, INPUT);
  pinMode(INT_RTC, INPUT_PULLUP);
  pinMode(INT_FLT, INPUT_PULLUP);

  // Set initial pin states
  digitalWrite(PWR_PIN, LOW);
  digitalWrite(DIS_PIN, LOW);
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
  digitalWrite(EN_PIN, LOW);
  digitalWrite(DIN_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(CLR_PIN, LOW);
  digitalWrite(LVCC_PIN, LOW);
  digitalWrite(LVFD_PIN, LOW);
  digitalWrite(LDIAG_PIN, LOW);

  // Link the button functions
  button1.attachClick(ShortPressButton1);
  button1.attachLongPressStart(LongPressButton1);
  button1.attachDoubleClick(DoubleClickButton1);
  button1.setDebounceMs(50);
  button1.setPressMs(800);

  button2.attachClick(ShortPressButton2);
  button2.attachLongPressStart(LongPressButton2);
  button2.setDebounceMs(50);
  button2.setPressMs(2000);

  // Active 2-second wait — poll buttons to detect long press
  {
    unsigned long startWait = millis();
    while(millis() - startWait < 2000)
    {
      button1.tick();
    }
  }

  powerClockStatus = true;
  digitalWrite(PWR_PIN, HIGH);
  digitalWrite(LVCC_PIN, HIGH);

  delay(100);
  WaitForUsbPowerStable();

  // SPI initialization
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  delay(100);

  ClearOutputs();

  digitalWrite(ENA_PIN, HIGH);
  delay(500);

  digitalWrite(ENB_PIN, HIGH);
  delay(500);

  // If button1 was long-pressed during the 2s wait — enter test mode
  if(longPressButton1Status == true)
  {
    ClearPressButtonFlags();
    HardFlipDiscCleaning();
    PowerOffClock();
  }

  ClearPressButtonFlags();

  attachInterrupt(digitalPinToInterrupt(INT_FLT), fltInterruptISR, FALLING);

  // RTC RX8025T initialization
  RTC_RX8025T.init();

  // Time update interrupt initialization. Interrupt generated by RTC (INT output):
  // "INT_SECOND" - every second,
  // "INT_MINUTE" - every minute.
  RTC_RX8025T.initTUI(INT_MINUTE);

  // "INT_ON" - turn ON interrupt generated by RTC (INT output),
  // "INT_OFF" - turn OFF interrupt.
  RTC_RX8025T.statusTUI(INT_ON);

  // Assign an interrupt handler to the RTC output,
  // an interrupt will be generated every minute to display the time
  attachInterrupt(digitalPinToInterrupt(INT_RTC), rtcInterruptISR, FALLING);

  // Read setting options from eeprom memory
  time_format = EEPROM.read(ee_time_format_address);
  symbol_type = EEPROM.read(ee_symbol_type_address);
  effect_interval = EEPROM.read(ee_effect_interval_address);

  // If the read values are incorrect, set the default values
  if(time_format != HR12 && time_format != HR24) time_format = HR12;
  if(symbol_type != NUMB && symbol_type != DICE) symbol_type = NUMB;
  if(effect_interval != E00M && effect_interval != E01M && effect_interval != E60M) effect_interval = E01M;

  ClearPressButtonFlags();
  RandomFlipDisc(0);
  RandomFlipDisc(1);

  // After startup animation, display time without effect (DisplayRawTime instead of DisplayTime).
  // This ensures currentTimeDigit[] matches the physical display state.
  // On the next RTC interrupt, DisplayTime() will compute correct current_state.
  GetRtcTime();
  DisplayRawTime();

  timeSettingsStatus = false;
  effectSettingsStatus = false;

  wdt_enable(WDTO_8S);
}

// ============================================================================
// LOOP
// ============================================================================

void loop()
{
  wdt_reset();
  WatchButtons();

  // Handle the per-minute RTC interrupt.
  // Every minute the RTC generates an interrupt that sets the interruptRtcStatus flag.
  //
  // Two visual effect modes:
  // - E01M (every minute): visual effect on every interrupt
  //   (EFFECT_MINUTE middle cycles)
  // - E60M (every hour): effect only on the full hour (minutes == 0)
  //   (EFFECT_HOUR middle cycles), otherwise DisplayRawTime.
  if(interruptRtcStatus == true) DisplayTime();
  if(timeSettingsStatus == true) TimeSettings();
  if(effectSettingsStatus == true) EffectSettings();
}

void GetRtcTime(void)
{
  RTC_RX8025T.read(tm);

  hour_time = tm.Hour;
  minute_time = tm.Minute;

  // 12-Hour conversion
  if(time_format == HR12)
  {
    if(hour_time > 12) hour_time = hour_time - 12;
    if(hour_time == 0) hour_time = 12;
  }

  // Split time into 4 individual digits
  // currentTimeDigit[3] = tens of hours  (e.g. 1 from "14:25")
  // currentTimeDigit[2] = units of hours (e.g. 4 from "14:25")
  // currentTimeDigit[1] = tens of minutes  (e.g. 2 from "14:25")
  // currentTimeDigit[0] = units of minutes (e.g. 5 from "14:25")
  currentTimeDigit[3] = hour_time / 10;
  currentTimeDigit[2] = hour_time % 10;
  currentTimeDigit[1] = minute_time / 10;
  currentTimeDigit[0] = minute_time % 10;
}

// ============================================================================
// DISPLAY RAW TIME
// ============================================================================

void DisplayRawTime(void)
{
  interruptRtcStatus = false;

  for(uint8_t sector = 1; sector <= 4; sector++)
  {
    FlipDisplay(sector, currentTimeDigit[sector - 1], symbol_type);
  }
}

// ============================================================================
// DISPLAY TIME
// ============================================================================

void DisplayTime(void)
{
  interruptRtcStatus = false;

  // Compute current display state BEFORE reading new time
  // (currentTimeDigit[] still contains the OLD time)
  ComputeDisplayState(current_state);

  GetRtcTime();

  // Compute target display state for the NEW time
  // (currentTimeDigit[] has been updated by GetRtcTime)
  ComputeDisplayState(target_state);

  if(effect_interval == E00M)
  {
    DisplayRawTime();
  }
  else if(effect_interval == E01M)
  {
    RandomTimeEffect(minute_time == 0 ? EFFECT_HOUR : EFFECT_MINUTE);
  }
  else if(effect_interval == E60M)
  {
    if(minute_time == 0) RandomTimeEffect(EFFECT_HOUR);
    else DisplayRawTime();
  }
}

// ============================================================================
// RANDOM TIME EFFECT
// ============================================================================

/*
 * ComputeDisplayState() - Computes the state of all 36 discs based on currentTimeDigit[].
 *
 * The state_array parameter is a pointer to a bool[36] array where the result is stored.
 * Called twice in DisplayTime():
 * - BEFORE GetRtcTime() with current_state[] -> OLD time state (what is currently on the display)
 * - AFTER GetRtcTime() with target_state[]    -> NEW time state (what should be displayed)
 *
 * The numbArray/diceArray tables store digit patterns 0-9 as compressed bits:
 * - numbArray[digit][0] = disc bits 0-7 (first byte)
 * - numbArray[digit][1] = disc bit 8 (second byte, only bit 0)
 *
 * Result in state_array[0..35]:
 * - indices 0-8:   section 1
 * - indices 9-17:  section 2
 * - indices 18-26: section 3
 * - indices 27-35: section 4
 */
void ComputeDisplayState(bool* state_array)
{
  for(uint8_t section = 1; section <= 4; section++)
  {
    uint8_t row_index = currentTimeDigit[section - 1];

    for(uint8_t disc_number = 0; disc_number < 9; disc_number++)
    {
      uint8_t bit_number;
      uint8_t column_index;

      if(disc_number < 8)
      {
        bit_number = disc_number;
        column_index = 0;
      }
      else
      {
        bit_number = disc_number - 8;
        column_index = 1;
      }

      bool disc_status;
      if(symbol_type == DICE) disc_status = ((pgm_read_byte(&diceArray[row_index][column_index])) >> (bit_number)) & 0b00000001;
      else                    disc_status = ((pgm_read_byte(&numbArray[row_index][column_index])) >> (bit_number)) & 0b00000001;

      uint8_t idx = disc_number + 9 * (section - 1);
      state_array[idx] = disc_status;
    }
  }
}

/*
 * RandomFlipOffCurrent() - Randomly turns OFF only the discs that are currently ON.
 *
 * First step of the visual effect: the old time "disappears" from the display.
 * Operates only on discs marked as ON in current_state[],
 * making the effect faster (fewer discs to flip)
 * and more precise — only discs forming the old time are turned off.
 *
 * Algorithm: Fisher-Yates shuffle on the subset of ON discs.
 */
void RandomFlipOffCurrent(void)
{
  // Collect disc numbers (1-36) that are currently ON
  uint8_t on_discs[36];
  uint8_t on_count = 0;

  for(uint8_t i = 0; i < 36; i++)
  {
    if(current_state[i])
    {
      on_discs[on_count] = i + 1;  // 1-36 numbering for FlipDisc()
      on_count++;
    }
  }

  // Fisher-Yates shuffle — random permutation of on_discs array
  // Guard: when on_count == 0, expression (on_count - 1) for uint8_t = 255 (underflow)
  if(on_count > 1)
  {
    for(uint8_t i = on_count - 1; i > 0; i--)
    {
      uint8_t j = random(0, i + 1);
      uint8_t temp = on_discs[i];
      on_discs[i] = on_discs[j];
      on_discs[j] = temp;
    }
  }

  // Turn OFF each disc in the shuffled order
  for(uint8_t i = 0; i < on_count; i++)
  {
    FlipDisc(on_discs[i], 0);
  }
}

/*
 * RandomFlipOnTarget() - Randomly turns ON only the discs needed to display the new time.
 *
 * Last step of the visual effect: the new time "appears" on the display.
 * Operates only on discs marked as ON in target_state[],
 * so from a blank (black) screen the new time digits randomly emerge.
 *
 * Algorithm: Fisher-Yates shuffle on the subset of target-ON discs.
 */
void RandomFlipOnTarget(void)
{
  // Collect disc numbers (1-36) that should be ON in the new time
  uint8_t on_discs[36];
  uint8_t on_count = 0;

  for(uint8_t i = 0; i < 36; i++)
  {
    if(target_state[i])
    {
      on_discs[on_count] = i + 1;
      on_count++;
    }
  }

  // Fisher-Yates shuffle
  // Guard: when on_count == 0, expression (on_count - 1) for uint8_t = 255 (underflow)
  if(on_count > 1)
  {
    for(uint8_t i = on_count - 1; i > 0; i--)
    {
      uint8_t j = random(0, i + 1);
      uint8_t temp = on_discs[i];
      on_discs[i] = on_discs[j];
      on_discs[j] = temp;
    }
  }

  // Turn ON each disc in the shuffled order
  for(uint8_t i = 0; i < on_count; i++)
  {
    FlipDisc(on_discs[i], 1);
  }
}

/*
 * RandomTimeEffect() - Main visual effect function for time changes.
 *
 * The middle_cycles parameter sets the number of middle sequence repetitions (ON -> OFF).
 *
 * Full effect sequence:
 *
 * 1. Randomly turn OFF currently-ON discs (old time disappears)
 *    -> operates ONLY on discs forming the old time
 *
 * 2. Middle sequence (repeated middle_cycles times):
 *    -> random ON all 36 discs -> random OFF all 36 discs
 *
 * 3. Randomly turn ON new time discs (new time appears)
 *    -> operates ONLY on discs forming the new time
 *
 * Example every minute (middle_cycles = 1):
 *   [12:34] -> turn off old -> ON->OFF -> turn on new -> [12:35]
 *
 * Example every hour (middle_cycles = 2):
 *   [12:59] -> turn off old -> ON->OFF -> ON->OFF -> turn on new -> [13:00]
 *
 * current_state[] and target_state[] must be computed BEFORE calling
 * this function (in DisplayTime()).
 */
void RandomTimeEffect(uint8_t middle_cycles)
{
  // Step 1: Randomly turn OFF only currently-ON discs
  // Old time disappears from the display disc by disc in random order
  RandomFlipOffCurrent();

  // Step 2: Middle sequence — full flicker (ON -> OFF) repeated N times
  for(uint8_t cycle = 0; cycle < middle_cycles; cycle++)
  {
    RandomFlipDisc(1);
    RandomFlipDisc(0);
  }

  // Step 3: Randomly turn ON only discs needed for the new time
  // New time digits emerge from a blank screen disc by disc
  RandomFlipOnTarget();
}

// ============================================================================
// EFFECT SETTINGS
// ============================================================================

void EffectSettings(void)
{
  ClearPressButtonFlags();

  // Effect interval setting has three options: 0 - off, 1 - every minute, 2 - every 60 minutes (full hour)
  // For code simplicity, increment range is 0-2; when value is 2 we display 60
  uint8_t currentValue = effect_interval;
  if(currentValue == 2) currentValue = 60;

  FlipDisplay(4, currentValue / 10, NUMB);
  FlipDisplay(3, currentValue % 10, NUMB);
  FlipDisplay(2, CAD, NUMB);
  FlipDisplay(1, CAD, NUMB);

  do
  {
    WatchButtons();

    if(shortPressButton2Status == true)
    {
      shortPressButton2Status = false;
      effect_interval++;

      if(effect_interval > 2) effect_interval = 0;

      if(effect_interval == 2) currentValue = 60;
      else currentValue = effect_interval;

      FlipDisplay(4, currentValue / 10, NUMB);
      FlipDisplay(3, currentValue % 10, NUMB);
    }

    if(longPressButton1Status == true)
    {
      longPressButton1Status = false;
      break;
    }
  } while(true);

  ClearPressButtonFlags();

  FlipDisplay(1, 1, symbol_type);
  FlipDisplay(2, 1, symbol_type);
  FlipDisplay(3, CAD, NUMB);
  FlipDisplay(4, CAD, NUMB);

  do
  {
    WatchButtons();

    if(shortPressButton2Status == true)
    {
      shortPressButton2Status = false;
      if(symbol_type == NUMB) symbol_type = DICE;
      else symbol_type = NUMB;

      FlipDisplay(1, 1, symbol_type);
      FlipDisplay(2, 1, symbol_type);
    }

    if(longPressButton1Status == true)
    {
      longPressButton1Status = false;
      break;
    }
  } while(true);

  EEPROM.update(ee_effect_interval_address, effect_interval);
  EEPROM.update(ee_symbol_type_address, symbol_type);

  ClearPressButtonFlags();
  effectSettingsStatus = false;

  RandomFlipDisc(0);

  // After blanking, display time without effect — sync currentTimeDigit with display
  GetRtcTime();
  DisplayRawTime();
  interruptRtcStatus = false;
}

// ============================================================================
// TIME SETTINGS - set 4 digits one by one (section 4 -> 3 -> 2 -> 1)
// ============================================================================

void TimeSettings(void)
{
  ClearPressButtonFlags();

  uint8_t timeSettingsLevel = 4;
  uint8_t currentValue = 0;

  // Read current time from RTC
  RTC_RX8025T.read(tm);
  hour_time = tm.Hour;
  minute_time = tm.Minute;

  // 12-Hour conversion for display
  if(time_format == HR12)
  {
    if(hour_time > 12) hour_time = hour_time - 12;
    if(hour_time == 0) hour_time = 12;
  }

  currentTimeDigit[3] = hour_time / 10;
  currentTimeDigit[2] = hour_time % 10;
  currentTimeDigit[1] = minute_time / 10;
  currentTimeDigit[0] = minute_time % 10;

  RandomFlipDisc(0);

  FlipDisplay(4, time_format / 10, NUMB);
  FlipDisplay(3, time_format % 10, NUMB);

  do
  {
    WatchButtons();

    if(shortPressButton2Status == true)
    {
      shortPressButton2Status = false;
      if(time_format == HR12) time_format = HR24;
      else time_format = HR12;

      FlipDisplay(4, time_format / 10, NUMB);
      FlipDisplay(3, time_format % 10, NUMB);
    }

    if(longPressButton1Status == true)
    {
      longPressButton1Status = false;
      break;
    }
  } while(true);

  ClearPressButtonFlags();

  RandomFlipDisc(1);
  RandomFlipDisc(0);

  currentValue = currentTimeDigit[3];
  bool updateDisplayStatus = true;

  do
  {
    WatchButtons();

    // Short press button 2: increment displayed value (0-9, wraps around)
    if(shortPressButton2Status == true)
    {
      shortPressButton2Status = false;
      currentValue++;

      if(currentValue > 9) currentValue = 0;

      if(timeSettingsLevel == 4)
      {
        if(time_format == HR12 && currentValue > 1) currentValue = 0;
        if(time_format == HR24 && currentValue > 2) currentValue = 0;
      }

      if(timeSettingsLevel == 3)
      {
        if(time_format == HR12 && currentTimeDigit[3] == 0 && currentValue == 0) currentValue = 1;
        if(time_format == HR12 && currentTimeDigit[3] == 1 && currentValue > 2) currentValue = 0;
        if(time_format == HR24 && currentTimeDigit[3] == 2 && currentValue > 3) currentValue = 0;
      }

      if(timeSettingsLevel == 2 && currentValue > 5) currentValue = 0;

      FlipDisplay(timeSettingsLevel, currentValue, NUMB);
    }

    // Long press button 1: confirm value, move to next section
    if(longPressButton1Status == true)
    {
      longPressButton1Status = false;
      timeSettingsLevel--;
      currentTimeDigit[timeSettingsLevel] = currentValue;
      if(timeSettingsLevel > 0) currentValue = currentTimeDigit[timeSettingsLevel - 1];
      else currentValue = 0;

      // Hour 00 is not allowed in HR12 mode — minimum value is 01
      if(timeSettingsLevel == 3 && time_format == HR12 && currentTimeDigit[3] == 0 && currentValue == 0) currentValue = 1;

      updateDisplayStatus = true;
    }

    if(updateDisplayStatus == true)
    {
      updateDisplayStatus = false;

      // Display current value on the active section, CAD on others
      for(uint8_t sector = 4; sector >= 1; sector--)
      {
        if(sector == timeSettingsLevel) FlipDisplay(sector, currentValue, NUMB);
        else FlipDisplay(sector, CAD, NUMB);
      }
    }
  } while(timeSettingsLevel != 0);

  hour_time = (currentTimeDigit[3] * 10) + currentTimeDigit[2];
  minute_time = (currentTimeDigit[1] * 10) + currentTimeDigit[0];

  // setTime(hh, mm, ss, day, month, year)
  // The date is skipped and the seconds are set by default to 0
  // We are only interested in hours and minutes
  setTime(hour_time, minute_time, 0, 1, 1, 1);

  // Set the RTC from the system time
  RTC_RX8025T.set(now());

  EEPROM.update(ee_time_format_address, time_format);

  timeSettingsStatus = false;
  ClearPressButtonFlags();

  RandomFlipDisc(1);
  RandomFlipDisc(0);

  // After animation (ON->OFF), display time without effect — sync currentTimeDigit with display
  GetRtcTime();
  DisplayRawTime();
  interruptRtcStatus = false;
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void WatchButtons(void)
{
  wdt_reset();
  button1.tick();
  button2.tick();

  if(longPressButton1Status == true && effectSettingsStatus == false) timeSettingsStatus = true;
  if(doubleClickButton1Status == true && timeSettingsStatus == false) effectSettingsStatus = true;
  if(longPressButton2Status == true) PowerOffClock();
}

// ============================================================================
// POWER
// ============================================================================

void PowerOffClock(void)
{
  digitalWrite(ENB_PIN, LOW);
  delay(100);
  digitalWrite(ENA_PIN, LOW);
  delay(100);
  digitalWrite(PWR_PIN, LOW);
  delay(2000);
}

// ============================================================================
// LOW-LEVEL DISC CONTROL
// ============================================================================

// Clear shift register and latch cleared data to outputs
void ClearOutputs(void)
{
  // CLR resets shift register contents
  digitalWrite(CLR_PIN, LOW);
  delayMicroseconds(10);
  digitalWrite(CLR_PIN, HIGH);

  // EN pulse latches cleared register to outputs
  digitalWrite(EN_PIN, LOW);
  delayMicroseconds(10);
  digitalWrite(EN_PIN, HIGH);

  // Send zeros through SPI and latch again
  digitalWrite(EN_PIN, LOW);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(EN_PIN, HIGH);
}

void FlipDisc(uint8_t disc_number, bool disc_status)
{
  // Range validation: discs numbered 1-36, out of range -> ignore
  if(disc_number < 1 || disc_number > 36) return;

  wdt_reset();
  disc_number = disc_number - 1;

  // Start of SPI data transfer
  digitalWrite(EN_PIN, LOW);

  for(uint8_t byte_number = 0; byte_number < 3; byte_number++)
  {
    if(disc_status == 1) SPI.transfer(pgm_read_byte(&setDisc[disc_number][byte_number]));
    else                 SPI.transfer(pgm_read_byte(&resetDisc[disc_number][byte_number]));
  }

  // End of SPI data transfer
  digitalWrite(EN_PIN, HIGH);

  delayMicroseconds(1200);
  ClearOutputs();
  delay(20);
  delay(flip_disc_delay);
}

/*----------------------------------------------------------------------------------*
 * The function allows you to control 4x3x3 display.                                *
 * The first argument is the relative number of the display in the series           *
 * of all displays. The second argument is the display section_number 1-4           *
 * Third argument data_type, you can choose how the numbers are displayed.          *
 * -> section_number 1-4                                                            *
 * -> data 0-9                                                                      *
 * -> data_type - DICE/NUMB                                                         *
 *                                                                                  *
 *           Discs                       Sections                                   *
 *    36 35 34   27 26 25              o o o  o o o                                 *
 *    33 32 31   24 23 22              o 4 o  o 3 o                                 *
 *    30 29 28   21 20 19              o o o  o o o                                 *
 *                                                                                  *
 *    18 17 16    9  8  7              o o o  o o o                                 *
 *    15 14 13    6  5  4              o 2 o  o 1 o                                 *
 *    12 11 10    3  2  1              o o o  o o o                                 *
 *----------------------------------------------------------------------------------*/
void FlipDisplay(uint8_t section_number, uint8_t new_data, uint8_t data_type)
{
  // Range validation: section 1-4, data 0-11 (0-9 + CAD + SAD)
  if(section_number < 1 || section_number > 4) return;
  if(new_data > 11) return;

  bool disc_status = 0;
  uint8_t bit_number = 0;
  uint8_t column_index = 0;
  uint8_t row_index = new_data;

  for(uint8_t disc_number = 0; disc_number < 9; disc_number++)
  {
    if(disc_number < 8)
    {
      bit_number = disc_number;
      column_index = 0;
    }
    else
    {
      bit_number = disc_number - 8;
      column_index = 1;
    }

    if(data_type == DICE) disc_status = ((pgm_read_byte(&diceArray[row_index][column_index])) >> (bit_number)) & 0b00000001;
    else                  disc_status = ((pgm_read_byte(&numbArray[row_index][column_index])) >> (bit_number)) & 0b00000001;

    // Each of the 3x3 display sections is numbered the same way,
    // for each subsequent section we need to shift the disc number by 9
    uint8_t current_disc_number = disc_number + 9 * (section_number - 1);
    FlipDisc(current_disc_number + 1, disc_status);
  }
}

void RandomFlipDisc(bool disc_status)
{
  uint8_t order[36];
  for(uint8_t i = 0; i < 36; i++) order[i] = i + 1;

  // Fisher-Yates shuffle
  for(uint8_t i = 35; i > 0; i--)
  {
    uint8_t j = random(0, i + 1);
    uint8_t temp = order[i];
    order[i] = order[j];
    order[j] = temp;
  }

  for(uint8_t i = 0; i < 36; i++)
  {
    FlipDisc(order[i], disc_status);
  }
}

// ============================================================================
// DIAGNOSTICS
// ============================================================================

void WaitForUsbPowerStable(void)
{
  unsigned long previousSampleMillis = 0;
  unsigned long previousBlinkMillis  = 0;

  unsigned long blinkInterval = 500;
  unsigned long currentMillis;

  bool ledState = false;
  bool errorActive = false;

  int consecutiveOkCount = 0;   // counts consecutive in-range readings
  int sampleCounter = 0;        // counts samples in current window (max 10)

  while(true)
  {
    wdt_reset();
    currentMillis = millis();

    // =====================================================
    // 1. Measurement every SAMPLE_DELAY_MS
    // =====================================================
    if(currentMillis - previousSampleMillis >= SAMPLE_DELAY_MS)
    {
      previousSampleMillis = currentMillis;

      int measuredVbus = analogRead(ADC_VBUS_PIN);

#if DEBUG
      mySerial.print(F("VBUS: "));
      mySerial.println(measuredVbus);
#endif

      sampleCounter++;

      // Check individual measurement
      if(measuredVbus > LOWER_THRESHOLD &&
         measuredVbus < UPPER_THRESHOLD)
      {
        consecutiveOkCount++;   // another consecutive in-range reading

        // If we reached 5 consecutive in-range readings
        if(consecutiveOkCount >= REQUIRED_OK_SAMPLE)
        {
          digitalWrite(LVFD_PIN, HIGH);
          digitalWrite(LDIAG_PIN, LOW);
          break;   // exit immediately
        }
      }
      else
      {
        consecutiveOkCount = 0;   // streak broken
      }

      // =====================================================
      // 2. If 10 samples taken without 5 consecutive OK
      // =====================================================
      if(sampleCounter >= 10)
      {
        errorActive = true;   // start blinking only now

        // Determine error type from the last measurement
        if(measuredVbus <= LOWER_THRESHOLD)
        {
          blinkInterval = 1000;   // slow blink
        }
        else if(measuredVbus >= UPPER_THRESHOLD)
        {
          blinkInterval = 250;    // fast blink
        }

        // Reset for next 10-sample window
        sampleCounter = 0;
        consecutiveOkCount = 0;
      }
    }

    // =====================================================
    // 3. Non-blocking LED blinking
    // =====================================================
    if(errorActive)
    {
      if(currentMillis - previousBlinkMillis >= blinkInterval)
      {
        previousBlinkMillis = currentMillis;
        ledState = !ledState;

        digitalWrite(LVFD_PIN, ledState);
        digitalWrite(LDIAG_PIN, ledState);
      }
    }

    // =====================================================
    // 4. Button handling runs continuously
    // =====================================================
    WatchButtons();
  }
}

// ============================================================================
// MAINTENANCE
// ============================================================================

void HardFlipDiscCleaning(void)
{
  for(uint8_t i = 0; i < 36; i++)
  {
    digitalWrite(EN_PIN, LOW);
    SPI.transfer(pgm_read_byte(&resetDisc[i][0]));
    SPI.transfer(pgm_read_byte(&resetDisc[i][1]));
    SPI.transfer(pgm_read_byte(&resetDisc[i][2]));
    digitalWrite(EN_PIN, HIGH);

    delayMicroseconds(1400);
    ClearOutputs();
    delay(100);
  }

  for(uint8_t i = 0; i < 36; i++)
  {
    digitalWrite(EN_PIN, LOW);
    SPI.transfer(pgm_read_byte(&setDisc[i][0]));
    SPI.transfer(pgm_read_byte(&setDisc[i][1]));
    SPI.transfer(pgm_read_byte(&setDisc[i][2]));
    digitalWrite(EN_PIN, HIGH);

    delayMicroseconds(1500);
    ClearOutputs();
    delay(100);
  }
}
