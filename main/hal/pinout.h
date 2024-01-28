#pragma once

//#define PIN_IO_MISC1 1 USED FOR BATTERY ADC SINCE WIRING ON PCB IS WRONG
#define PIN_IO_MISC2        2
#define PIN_HEADLIGHT       42
#define PIN_TAILLIGHT       41
#define PIN_BLINKER_LEFT    40
#define PIN_BLINKER_RIGHT   39

#define PIN_MOTOR_RIN       48
#define PIN_MOTOR_FIN       47
#define PIN_BAT_ADC         21 // not used
#define PIN_BAT_ADC_EN      14

#define CAM_PIN_PWDN        -1 //power down is not used
#define CAM_PIN_RESET       -1 //software reset will be performed
#define CAM_PIN_XCLK        15
#define CAM_PIN_SIOD        4
#define CAM_PIN_SIOC        5

#define CAM_PIN_D9          16
#define CAM_PIN_D8          17
#define CAM_PIN_D7          18
#define CAM_PIN_D6          12
#define CAM_PIN_D5          10
#define CAM_PIN_D4          8
#define CAM_PIN_D3          9
#define CAM_PIN_D2          11
#define CAM_PIN_D1          -1
#define CAM_PIN_D0          -1
#define CAM_PIN_VSYNC       6
#define CAM_PIN_HREF        7
#define CAM_PIN_PCLK        13