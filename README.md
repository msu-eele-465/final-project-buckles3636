# EELE 465 Final Project
## Embedded System Description

My embedded system will take the an input in the form of degrees over i2C (Most significant Byte, Least significant byte)

## Hardware Setup

The selected servo motor will be a [brushed dc type, with quadurture encoder](https://www.amazon.com/dp/B0DB28N8FJ/ref=sspa_dk_detail_0?pd_rd_i=B0DB28N8FJ&pd_rd_w=S4IQc&content-id=amzn1.sym.8c2f9165-8e93-42a1-8313-73d3809141a2&pf_rd_p=8c2f9165-8e93-42a1-8313-73d3809141a2&pf_rd_r=R2YHJAFX2T8W96AYETSX&pd_rd_wg=ZQ8JV&pd_rd_r=8e7f9e70-8a5c-4c9e-b885-8f2813bc07f5&s=hi&sp_csd=d2lkZ2V0TmFtZT1zcF9kZXRhaWw&th=1). This will then be connected to a tb67h420ftg dual halfbridge motor driver. The motor driver and encoder outputs will be hooked up to the slave. An analog discovery 2 will be used as the master.

## Software overview

Slave will take an integer for position over I2C and move to that exact position using feedback from the encoder and a control loop. Slave will also print current encoder position over uart for debugging purposes.


