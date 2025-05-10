# KOReader Page Turner

This project aims to give a hardware page turner for those using KOReader.

It takes advantage of the KOReader HTTP inspector plugin.

![Front view](/assets/front.jpg)

# Hardware

In order to make this project readily available for everyone I used popular modules and components.

## Bill of Materials

* LILYGO® T7 S3 ESP32-S3 ([Aliexpress](https://www.aliexpress.com/item/1005004777561826.html), [LILYGO®](https://lilygo.cc/products/t7-s3?variant=42490159464629))
* Momentary Button - 19mm, High head ([Aliexpress](https://www.aliexpress.com/item/1005004407709401.html)) - IMPORTANT!!! Order the 19mm variant as the design is built around that size.
* 103450 Lipo Battery 2000mAh ([Aliexpress](https://www.aliexpress.com/item/1005008129110763.html))
* 3 x 'M3' self tapping screw 10mm ([Aliexpress](https://www.aliexpress.com/item/4000982209705.html))

Some soldering is required but nothing crazy and I will provide a detailed guide for it.

The LILYGO board has a USB Type C port which can be used to program, and charge. It also has a battery charger module and port built in. Both the USB and power switch are exposed so they can be accessed once it's fully assembled.

# Software

The software is pretty simple and a [firmware.bin](bin/firmware.bin) is provided so you don't even need Platform.io set up.

With that being said, you can change/build it yourself using Platform.io.

## Easy Firmware Install

* Download the [firmware.bin](bin/firmware.bin) file
* Go to [ESPHome Web Tool](https://web.esphome.io)
* Click on `Connect`
* Pick the USB Serial port
* Click on Install
* Select the file you just downloaded
* Click on Install

## Configure

* After installing the firmware, the device will create an access point called `KoreaderPageTurnerAP`. Connect to it on your phone.
* A captive portal will pop up where you can enter your home WiFi credentials and your KOReader HTTP inspector details.
* After saving, the device will restart and connect to your WiFi.

You can later access the settings page by looking up the IP of the clicker on your router's settings page and open it in a browser.

# 3D Case

Print it at home with any 3D printer. It consist of 3 parts:
* Top (holds the button and the microctonroller)
* Bottom (holds the battery)
* Switch (extends the onboard switch so you can switch the device off when not used)

You can find the STEP files under the `case` folder and the slice file on the [MakerWorld project](https://makerworld.com/en/models/1400135-koreader-page-turner-button#profileId-1451844)

# Assemble

The button should go in first. You have to turn the button itself as the nut cannot move.

I used hot glue to fix the battery and the microcontroller in place. They in their respective cavities.

Solder `GPIO4` to one side of the switch and `GND` to the other. 
If you have a battery like I did, you may need to switch the JST connector to the smaller one that came with the LILYGO microcontroller. It's just a matter of removing the shield tape, desolder, solder the new one, put back the tape.

Place the switch extender in between the top and bottom half (it goes in one correct way).

Screw in the 3 self tapping screws.

![Inside view](/assets/inside.jpg)
![Top view](/assets/top.jpg)

# Usage

This is gonna be a quick one. :)

| Action | Function |
| ------ | -------- |
| Single Click | Next Page |
| Long Click | Previous Page |
| 10 second click | Reset preferences |

When you reset the device, the access point will be opened again.