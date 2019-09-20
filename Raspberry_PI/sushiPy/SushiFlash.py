##
 # @Auth: Reese RUssell
 # @Desc: Sushiboard Camera Flash Application Example

#Camera Flash Example Class

from smbus import SMBus #Used To Communicate with the arduino for the car ignition coil system
import serial #RPI3 Serial Library - Must Enable /dev/ttyAMA0 for this, please disable onboard bluetooth access and move it to /dev/ttyS0
import numpy as np
import time

class SushiFlash():
    def __init__(self):
        self.ser = serial.Serial('/dev/ttyAMA0', 115200) #RasPi3 Hardware Serial
        self.i2cAddr = 0x8
        self.i2c = SMBus(1) # indicates /dev/ic2-1
        print('Now using ' + str(self.ser.name) + ' for UART with Sushiboard.')
        print('Now using address ' + str(self.i2cAddr) + ' for I2C with Arduino.')
    def fireFlash(self, triggerDuration):
        length = np.uint8(triggerDuration)
        for i in range(2):
            self.i2c.write_byte(self.i2cAddr, length) # switch it on
            time.sleep(.001)
        ser.write(b't')
        
x = SushiFlash()
x.fireFlash(20)
