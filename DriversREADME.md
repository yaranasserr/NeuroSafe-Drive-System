# STM32F103xx Drivers

## Overview

This branch contains the drivers and application code for controlling a wheelchair using the STM32F103C8T6 microcontroller. It includes the following components:

- **MCAL Layer**: Drivers for microcontroller peripheral control

  - `RCC` (Reset and Clock Control)
  - `GPIO` (General Purpose Input/Output)
  - `NVIC` (Nested Vectored Interrupt Controller)
  - `SCB` (System Control Block)
  - `TIM` (Timer)
  - `EXTI` (External Interrupt)
  - `AFIO` (Alternate Function I/O)
  - `STK` (SysTick Timer)
  - `CAN` (Controller Area Network)
  - `PWM` (Pulse Width Modulation)

- **HAL Layer**: High-level drivers for motor control

  - `Motor.c` and `Motor.h`: Functions to control the motion directions of the wheelchair

- **APP Layer**: Application code for controlling the wheelchair
  - `app.c` and `app.h`: High-level control functions for motor operation based on received motion commands

## Setup

1. **Clone the Repository**

   ```
   git clone <repository_url>
   cd <repository_directory>
   ```

1. **Switch to the Branch**

   ```
   git checkout stm32f103xx_Drivers
   ```

   ## Usage

### MCAL Layer

The MCAL layer includes low-level drivers for various STM32F103C8T6 peripherals. Each peripheral driver is implemented in a separate file within the `MCAL` directory.

### HAL Layer

The HAL layer provides high-level functions for motor control. Use the functions in `Motor.c` and `Motor.h` to interface with the motors.

### APP Layer

The APP layer includes `app.c` and `app.h`, which contain the application-specific logic for controlling the wheelchair. This code processes the received list of commands and controls the motors accordingly.
