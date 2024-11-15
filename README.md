# Liquid Dispenser Portfolio Project
![project image banner](https://github.com/LaRoomy/Liquid_Dispenser/blob/main/Resources/pipes_banner_img.png)

## Overview
The Liquid Dispenser project is a comprehensive demonstration of embedded systems design and development, showcasing my expertise across a range of technical areas. This project was created as a portfolio piece to highlight my embedded development skills for potential employers. It integrates hardware control, real-time communication, and user interface components in a dynamic, real-world application.
Table of Contents

- [Features](#features)
- [Hardware Components](#hardware-components)
- [Software Architecture](#software-architecture)
- [Skills and Technologies](#skills-and-technologies)
- [Testing and Validation](#testing-and-validation)
- [Getting Started](#getting-started)
- [Architecture Resources](#architecture-resources)
- [Additional Info](#additional-info)
- [License](#license)

TODO: image of the parts and the schematic of the modules and so on.. ?

TODO: enable links to important entries and words

TODO: external tools used (e.g. GoogleTest) and links to install instructions and the tools itself

## Features

+ Interactive user interface with NFC-enabled smartphone input and touchscreen confirmation
+ Precision-controlled liquid dispensing using real-time sensor feedback
+ Efficient communication across multiple microcontrollers using CAN bus
+ Structured, modular code design with hardware-independent unit tests for robust validation
+ Developed to demonstrate expertise in hardware-software integration, real-time systems, and modern embedded software practices

## Hardware Components

![hardware-schematic](https://github.com/LaRoomy/Liquid_Dispenser/blob/main/Resources/hardware_schematic.png)

 - STM322F446 Evaluation Board
 - ILI93?? Display-Touch combined board
 - NFC07A1 ?? Nucleo Expansion board using the STDVKxx ...
 - Pump, electric vent, pressure sensor, flow rate sensor ...

## Software Architecture

The project is divided into three main applications, each running on a separate STM32F446 Evaluation Board:

1. NFC Reader Application: Handles user input from the NFC device and initiates the dispensing process.
2. Display Application: Manages the user interface, allowing the user to confirm, cancel or stop dispensing and shows notifications.
3. Liquid Control Application: Controls the pump and sensors to measure and dispense the correct amount of liquid.

The project employs a layered software architecture, including:

- Hardware Drivers: Custom, low-level drivers in C for direct hardware control
- Hardware Abstraction Layer (HAL): Connector classes in C++ for simplified access to hardware features
- Business Logic: High-level application logic in C++ that coordinates each stage of the dispensing process

## Skills and Technologies

This project demonstrates proficiency with a variety of embedded systems technologies and programming techniques:

- Programming Languages: C for hardware drivers, C++ for HAL and application layers
- Object-Oriented Programming (OOP): Applied to ensure modular, reusable code
- Design Patterns: Dependency injection and other patterns are used to create flexible and testable code
- UML (Unified Modeling Language): Used to plan and document the project architecture and workflow
- Real-Time Operating System (RTOS): Provides responsive and efficient task management
- Communication Protocols: CAN Bus for inter-board communication and I2C, UART, and SPI for interfacing with sensors and peripherals
- Near Field Communication (NFC): Integrated for secure and convenient user input
- Unit testing of embedded code

## Testing and Validation

- Hardware Driver Tests: Separate test projects validate the functionality of each hardware driver.
- HAL Tests: Test projects ensure reliable hardware integration via the Hardware Abstraction Layer.
- Unit Tests: Hardware-independent unit tests cover business logic, providing robust, environment-agnostic validation.

## Getting Started
### Prerequisites

- Hardware Requirements: STM32F446 Evaluation Boards, NFC module, touchscreen display, pump, pressure and flow sensors.
- Development Tools: STM32CubeMX, Keil MDK, or a compatible IDE for STM32 development.

### Installation

Clone the repository:

```shell

git clone https://github.com/LaRoomy/Liquid_Dispenser.git

```

Open each application in your IDE, configure board connections, and build.

### Usage

- Select an output amount using a smartphone.
- Scan the smartphone on the NFC reader.
- Confirm or cancel the output on the touchscreen.
- If confirmed, the system dispenses the selected amount of liquid.

## Architecture Resources

This project was planned using the Unified Modeling Language (UML) to outline the structure, communication flows, and key components of the system. See the Architecture Resources for diagrams and additional documentation.

## Additional Info
> [!NOTE]
> All code is developed and written by Hans Philipp Zimmermann using the reference documents of the respective devices and peripherals, except the driver for the ILI9341 display. This files are adapted and rewritten, to conform with the object based c programming, using the [stm32-ili9341](https://github.com/afiskon/stm32-ili9341/tree/master) library.

## License

This project is for demonstration purposes only and is licensed under the MIT License.
