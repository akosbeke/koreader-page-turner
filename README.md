# KOReader Page Turner

This project aims to give a hardware page turner for those using KOReader.

It takes advantage of the KOReader HTTP inspector plugin.

# Hardware

In order to make this project readily available for everyone I used popular modules and components.

## Bill of Materials

* LILYGO® T7 S3 ESP32-S3 ([Aliexpress](https://www.aliexpress.com/item/1005004777561826.html), [LILYGO®](https://lilygo.cc/products/t7-s3?variant=42490159464629))
* Momentary Button - 19mm ([Aliexpress](https://www.aliexpress.com/item/1005004407709401.html)) - IMPORTANT!!! Order the 19mm variant as the design is built around that size.
* Lipo Battery 3.7mAh (dimensions later)

Some of these components don't have a link yet because the 3D model is a WIP.

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

Work in progress, but eventually it should be a quick print (either at home or your local 3D printing shop) and the components should just click to place.

# Usage

This is gonna be a quick one. :)

| Action | Function |
| ------ | -------- |
| Single Click | Next Page |
| Long Click | Previous Page |
| 10 second click | Reset preferences |

When you reset the device, the access point will be opened again.