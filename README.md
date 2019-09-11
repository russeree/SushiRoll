# SushiRoll
 SushiRoll - A Parallel IGBT driver STM32F03 - DMA - UART

 SushiRoll is a firmware for Sushiboard. Currently this firmware has some functionality but is still incomplete. The SushiRoll firmware is engineered in such a way that it is DMA and interrupt driven to 'guarantee' timing. The reason timing is important is because low impedance loads driven by parallel IGBTs will lead to failure if the parts ‘short circuit’ duration is violated.

![SushiBoard V0.1](/Assets/SushiBoard.jpg)
