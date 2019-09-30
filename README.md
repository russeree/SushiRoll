# SUSHIBOARD FIRMWARE - PARALLEL IGBT DRIVER
Based on the STM32F03 MCU this firmware source code is well documented and should be able to help users who are looking to use more advanced features of the STM32F0 platform. This project incorporates FLASH Writing - DMA UART - DMA Timers - Interrupts, EXTI, and much more. 

## INSTRUCTIONS 
Open the project file using the STM32 Studio (AC6). This build works on Windows and Mac versions. Nothing special is required. Hardware required to flash this frimware is the STM32F031F6P7TR MCU. These can be obtained on Mouser.com or Digikey.com for about 2 USD, The variant used for this project has 4KB SRAM; There are 6KB SRAM versions in the same package. 

## Why the name Sushiboard?
Sushibaord started about as an idea for a IGBT controlled xenon Flash. Where the goal is to quench the flash almost instantaneously after triggering on the order of one microsecond. As well as creating a method to PWM a xenon flash as to allow a more uniform flash intensity when using fast shutter speeds with a focal plane shutter. *ex. 1/2000, 1/4000, 1/8000 second shutter speeds* The reason for a parallel IGBT driver being needed for this application is the incredibly intense surge current through a flashtube. Current values as large as 700 amps have been measured across a 400 joule flash lamp. Overall the need was a high power low side switch.

## Why build Sushiroll?
Sushiroll is the name given to the firmware package used with the onboard STM32F0 MCU. Sushiroll uses the STM32 HAL driver set but avoids the used of CUBEMX for anything other than the initial boot code generation. The need for a true frimware arose with the failure of about 4 different IGBT and MOSFET ICs. This is because at peak voltage the IGBT can not conduct in a short circuit. This is discovered when combing through the datasheets of IXYS IGBTs. The IGBT datasheet even when it says there the capability to handle a large current for 1ms, there is also listed a second parameter. This parameter is the SHORT CIRCUIT time; usually on the order of just a few uS, not even close to the mS time. This parameter is critical to not be ignored, when an IGBT fails the mode of failure is to become short circuit. This means that with loads such as motors that could propel people would be latched up and consuming as much power as the source can deliver. This will occur until either the IGBT or load fails. Suhiboards firmware implements timers and protection methods to enable safer operation of these IGBTs under near short loads. 

![SushiBoard V0.1](/Assets/SushiBoard.jpeg)
