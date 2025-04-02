# Final project proposal

- [X] I have reviewed the project guidelines.
- [X] I will be working alone on this project.
- [X] No significant portion of this project will be (or has been) used in other course work.

## Embedded System Description

My embedded system will take the input of a potentiometer to set the position of a servo motor. The system will also feature a saftey switch and enable/disable leds.

## Hardware Setup

The selected servo motor will be a [brushed dc type, with quadurture encoder](https://www.amazon.com/dp/B0DB28N8FJ/ref=sspa_dk_detail_0?pd_rd_i=B0DB28N8FJ&pd_rd_w=S4IQc&content-id=amzn1.sym.8c2f9165-8e93-42a1-8313-73d3809141a2&pf_rd_p=8c2f9165-8e93-42a1-8313-73d3809141a2&pf_rd_r=R2YHJAFX2T8W96AYETSX&pd_rd_wg=ZQ8JV&pd_rd_r=8e7f9e70-8a5c-4c9e-b885-8f2813bc07f5&s=hi&sp_csd=d2lkZ2V0TmFtZT1zcF9kZXRhaWw&th=1). 

## Software overview

Slave will take an integer for position over I2C and move to that exact position using feedback from the encoder and a control loop.

Master will use the ADC to read a value from the potentiometer and convert that into a position to send to the motor. Will also have saftey switch to lock the system and outputs configured as status leds.

## Testing Procedure

First, ensure turning the potentiometer while system is in "safe mode" does not move motor. Then switch system into "active mode" and ensure one rotation of the potentiometer correlates to exactly one rotation of the motor. 


## Prescaler

Desired Prescaler level: 

- [ ] 100%
- [X] 95% 
- [ ] 90% 
- [ ] 85% 
- [ ] 80% 
- [ ] 75% 

### Prescalar requirements 

**Outline how you meet the requirements for your desired prescalar level**

**The inputs to the system will be:**
1.  Quadurature Encoder
2.  Potentiometer
3.  Switch

**The outputs of the system will be:**
1.   Half bridge motor driver

**The project objective is**

Position controlled servo motor

**The new hardware or software modules are:**
1. TB67H420FTG Half Bridge Motor Driver
2. N20 200rpm Encoder Motor
3. Hall Effect Quaderature Encoder


The Master will be responsible for:

Reading the potentiometer and sending position commands over I2C. Also will host the status leds and safety switch.

The Slave(s) will be responsible for:

Quaderature encoder reading, motor driver output, and control loop.


### Argument for Desired Prescaler

Lots of new hardware and I already know getting the encoder working correctly is going to be a major pain. Also will have to do PID tuning most likely.
