# Flipo-Modular-Clock-4x3x3-Flip-Disc-Display

## Specification
- the clock consists of two modules: controller module & flip-disc display module
- two buttons for setting the time
- one button for turning on the clock
- accurate real-time clock (RTC) on board RX8025T
- the RTC clock memory is backed up by a supercapacitor, so the clock does not require an additional battery in the event of a power failure or turn off
- clock dimensions: 53 x 137 x 142mm (2.1" x 5.4" x 5.6")
- power supply from USB-C PD 12V
- ISP programming connector - only for intermediate users

## Datasheet
  - [CLOCK USER MANUAL](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Manual-Modular-Flip-Disc-Clock.pdf)
  - [HOW TO READ THE DISPLAY](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Modular-Flip-Disc-Clock-Symbols.pdf)
  - [4x3x3 flip-disc display module v1 - outdated](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Modular-Flip-disc-Clock-4x3x3-Display-Module-Schematic-v1.pdf)
  - [4x3x3 flip-disc display module v2 - current version](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Modular-Flip-disc-Clock-4x3x3-Display-Module-Schematic-v2.pdf)
  - [4x3x3 flip-disc controller module v1 - outdated](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Modular-Flip-disc-Clock-4x3x3-Controller-Module-Schematic-v1.pdf)
  - [4x3x3 flip-disc controller module v5 - current version](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/raw/main/datasheet/Modular-Flip-disc-Clock-4x3x3-Controller-Module-Schematic-v5.pdf)


## Modular Flip-Disc Clock – Programming and Firmware
If you want to modify the firmware, you can program the clock via the ISP connector.

### MiniCore Configuration
Before burning the bootloader, install MiniCore using the following Boards Manager URL:
https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json

### In Arduino IDE → Tools, configure the settings as follows:
- Board: ATmega328P / ATmega328PA
- Clock: External 12 MHz
- Bootloader: Yes (UART0)
- BOD: 2.7V
- EEPROM: Retained
- Compiler LTO: Enabled
- Baud Rate: Default
- Variant: 328P / 328PA

⚠ Make sure the clock is set to External 12 MHz. ⚠

## Programming Procedure:
 1. Select the correct Programmer: AVR ISP MKII (recommended).
 2. Click Burn Bootloader to set the fuse bits and configure the microcontroller.
 3. After completion, select Upload Using Programmer to flash the firmware.

![Modular Flip-disc Clock](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/blob/main/extras/modular-flipdisc-clock-cover-github.webp)
![Modular Flip-disc Clock - Digits](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/blob/main/extras/Modular-Flip-Disc-Clock-Symbols.webp)
![Modular Flip-disc Clock - 3x3 Module](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/blob/main/extras/3x3-Flip-disc-Display-Counting-0-9.webp)
![Modular Flip-disc Clock - HH:MM](https://github.com/marcinsaj/Flipo-Modular-Clock-4x3x3-Flip-Disc-Display/blob/main/extras/HHMM-modular-flip-disc-clock.webp)
