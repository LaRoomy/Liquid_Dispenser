# Hardware Overview

This section provides a detailed look at the hardware setup for the Liquid Dispenser project. The hardware is built using STM32F446 microcontrollers and various connected devices, assembled on breadboards for evaluation and development purposes.

## NFC Part Application Hardware

This image shows the hardware setup for the NFC part of the application. It includes the STM32F446 microcontroller connected to the Nucleo NFC07A1 expansion board. 
The NFC module enables the smartphone to communicate with the system for user input.

![nfc part application](nfc_part_and_app.JPEG)

## Display Part Application Hardware

This image displays the hardware for the touchscreen interface. The STM32F446 is connected to an ILI9341 display, which provides a user-friendly interface for confirming or canceling operations.

![diplay part application](display_part.JPEG)

## Flow Control Application Hardware

This setup demonstrates the flow control hardware. The STM32F446 microcontroller is connected to the flow control dispenser device via an optocoupler, ensuring isolation between the logic circuit and the 12V-powered dispenser device. This separation is crucial to avoid mixing ground signals during development and testing.

![flow control part application](flowcontrol_part.JPEG)

## Complete Hardware Assembly

This image shows the entire hardware setup assembled on breadboards. All parts are connected through the CAN bus.

![complete project hardware setup](all_parts.JPEG)

## Flow Control Dispenser Device

Here is the flow control dispenser device, which pumps the selected amount of liquid. This device is powered by a separate 12V source and interfaces with the STM32F446 via the optocoupler circuit.

![flow control dispenser device](flowcontrol_device.JPEG)

## Project Demonstration Video

Watch the project in action as it dispenses water:

[![Liquid dispenser portfolio project demonstration video](http://img.youtube.com/vi/yfymt_yXDyY/0.jpg)](http://www.youtube.com/watch?v=YOUTUBE_yfymt_yXDyY "Liquid Dispenser Portfolio Project Demonstration")
