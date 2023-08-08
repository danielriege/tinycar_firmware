# Smartcar Firmware

This repository is a esp-idf project for the esp32 based 1:87 smartcars.
To be able to configure, build and flash this firmware, you will need ESP-IDF. [Follow this instruction](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation).

## Basics

The current version of this firmware supports communication only to the smartcar. Using UDP messages, the lights and motor duty cycle can be set.

### Light Control Message
- **Byte 1**: Message type (0x01 for light control)
- **Byte 2**: Headlight mode (0 - Off, 1 - Low, 2 - High)
- **Byte 3**: Taillight mode (0 - Off, 1 - On, 2 - Brake)
- **Byte 4**: Blinker mode (0 - Off, 1 - Left, 2 - Right, 3 - Hazard)

### Motor Control Message
- **Byte 1**: Message type (0x02 for motor control)
- **Byte 2**: Motor duty cycle (0 - 100, representing duty cycle percentage)

## Configuration
Using `idf.py menuconfig` you can configure the firmware, especially the WiFi credentials, hostname and pins.
Go to `Smartcar Configuration` to make these changes. Set a unique hostname for the car, so it can be identified on the network easier.

For the Pin setup, you can choose if the lights (e.g. the two headlights) have individual GPIO Pins or if they are grouped together on one Pin (depends on Smartcar, see below for that).
You can also set a maximum duty cycle for the motor. This can be useful if you want to prevent the motor from overheating (if not done hardware wise).

## Flash
Depending on the hardware, you need to bring the ESP32 into download mode (depends on Smartcar, see below for that). Using `idf.py -p /dev/DEVICE flash` the configured firmware can be flashed. 
When this is done, you can unplug the Smartcar from your computer and turn it on. After this reboot the ESP32 should go into normal mode again (make sure IO0 is not grounded).

# Smartcars
In this section all current active Smartcars are described.
## DB Regio Bus (red bus)
This is the first prototype of a Smartcar. Therefore the features are limited and the hardware takes more space. 
### Configure and Flash
Since the LEDs are not controlled with a transistor, each LED is connected to a GPIO directly (to prevent too much current draw from IO). Because of that all the lights are configured with individual pins.
To bring this car into download mode, turn off the car (with the switch on the bottom) and connect the car with a USB-C cable to the computer (Not the USB-C connector on the back but the one inside facing to the front). Now the car can be flashed. After flashing make sure that you disconnect the cable before turning the car on.
### Charging
This car uses two 100mAh LiPo cells and a TP4056 board to charge these. This board does NOT consist of a load sharing circuit, so ALWAYS turn the car off before charging. Otherwise the TP4056 board does not recognize when the batterys are full and overcharges them, which can be dangerous with LiPos. You can see a blue light inside the car when the car is turned on.

