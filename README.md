# STMQRCode
QR code generator for STM32F407

## Overview
**STMQRCode** is the project of QR code generator. It generates QR codes based on text stored in SD card and displays it on Nokia5110 display. Additionally it gives the opportunity to learn chinese radicals by flashcards module.

## Description
The main step is to generate QR code based on text stored in .txt file on SD card. Limitation of length input text is related to resolution of Nokia5110 display and QR code size. User can store on card many files. A file extension filter was used to read the .txt file names and store them in a bidirectional, cyclic list. This list is used to navigate through the files. User can switch files by 2 external buttons in a row of 3 buttons mounted on the breadboard (previous and next file). Using middle button (in a row of 3 buttons) user may generate QR code based on text stored in chosen .txt file. If you push additional button, mode changes into learning chinese radicals mode. Then the screen shows the chinese radical and next to him QR code. If you scan QR code by app, you will know the meaning of radical. Two external buttons in a row of 3 buttons give you opportunity to change flashcards in pseudo-random way. If you want to go back to the previous mode, push additional button.

## Demo
https://www.youtube.com/watch?v=rdWzFnNedUs

## Progress
- [x] QR code generation based on .txt files stored in SD card,
- [x] implementation of bidirectional, cyclic list used to navigate through files,
- [x] support for user interface buttons,
- [x] display file names on Nokia5110 display,
- [x] display chinese radicals with QR codes

## Tools
CooCox CoIDE, Version: 1.7.8

## How to run
To run the project you should have hardware:
* STM32f4-DISCOVERY board,
* SD Card Module and SD Card formatted to FAT32,
* Nokia 3310/5110 LCD screen,
* 4 switches.

How to use?
1. Connect STM32F4-DISCOVERY board with SD Card Module:
```
    STM32 <---> SD Card Module
    GND <---> GND
    3V <---> 3V3
    PB11 <---> CS
    PB15 <---> MOSI
    PB13 <---> SCK
    PB14 <---> MISO
    GND <---> GND
```
2. Connect 4 switches (PA5, PA7, PA8, PA9).
3. Connect Nokia 3310/5110 LCD screen:
```
    STM32 <---> SD Card Module
    GND <---> GND
    VDD <---> BL
    3V <---> VCC
    PB10 <---> CLK
    PC3 <---> DIN
    PC14 <---> DC
    PC13 <---> CE
    PC15 <---> RST
```
![alt text](https://github.com/kazimierczak-robert/STMQRCode/blob/master/schema.JPG)

4. Build this project with CooCox CoIDE and Download Code to Flash.


## How to compile
The only step is download the project and compile it with CooCox CoIDE.

## Attributions
* https://www.kismetwireless.net/code-old/svn/hardware/kisbee-02/firmware/drivers/displays/tft/bmp.c
* https://www.kismetwireless.net/code-old/svn/hardware/kisbee-02/firmware/drivers/displays/tft/bmp.h
* http://elm-chan.org/fsw/ff/00index_e.html
* https://github.com/trezor/trezor-qrenc
* https://magazeta.com/chinese-radicals

## License
MIT

## Credits
* Monika Grądzka,
* Robert Kazimierczak

Supervisor: dr inż. Krzysztof Zwierzyński
