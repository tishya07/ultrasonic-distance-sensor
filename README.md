# Ultrasonic Distance Measurement with Input Capture

This project demonstrates how to interface the HC-SR04 ultrasonic distance sensor with the STM32L4 Nucleo board using timer input capture. The sensor's Echo signal is measured using a timer to calculate the pulse width, which is then converted to distance in centimeters.

## Features

- Generate a 10 µs trigger pulse using PWM on TIM1 CH2 (PA9)
- Measure Echo pulse width using input capture on TIM4 CH1 (PB6)
- Handle timer overflows using a counter
- Convert pulse width to distance in centimeters
- Continuously update the distance variable in the main loop

## Hardware Requirements

- STM32L4 Nucleo Board  
- HC-SR04 Ultrasonic Distance Sensor  
- Breadboard and jumper wires  
- USB cable for power and debugging  

## Circuit Setup

| HC-SR04 Pin | Connects To | Description             |
|-------------|-------------|-------------------------|
| VCC         | 5V          | Sensor power            |
| GND         | GND         | Ground                  |
| Trig        | PA9         | TIM1 CH2 (PWM trigger)  |
| Echo        | PB6         | TIM4 CH1 (Input capture)|

## Configuration Summary

### Trigger – TIM1 CH2 on PA9

- Prescaler: 15 (16 MHz / (15+1) = 1 MHz → 1 tick = 1 µs)
- ARR: 0xFFFF
- CCR2: 10 → 10 µs pulse
- PWM Mode 1 used to create trigger pulse
- Output enabled via BDTR register

### Echo – TIM4 CH1 on PB6

- Input capture set to detect rising and falling edges
- Prescaler: 15 (1 MHz timer tick)
- Time interval calculated as:  
  `timeInterval = currentValue + overflowCount * (0xFFFF + 1) - lastValue`
- Overflow handling logic included in interrupt
- Pulse width converted to distance:  
  `distance = timeInterval / 58` (in centimeters)

## Behavior

- Trigger pulse sent from TIM1 causes HC-SR04 to emit ultrasonic burst
- Echo signal is measured via input capture on TIM4
- Time between rising and falling edge is used to compute distance
- Distance is updated in a global variable in the `while(1)` loop

## Notes

- Input capture is performed on both rising and falling edges
- Overflow count is used to maintain accurate timing across multiple timer overflows
- Distance is not printed but can be monitored in the debugger watch window
