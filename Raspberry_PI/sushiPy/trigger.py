##
 # @auth: Reese Russell
 # @desc: Simple Python Serial GPIO tigger for testing Sushiboard
 # @Note: sudo apt-get install python-serial
 ##

import serial #SERIAL LIBRARY
import RPi.GPIO as GPIO #Included GPIO Library
import time

triggerPin = 2

GPIO.setmode(GPIO.BCM)
GPIO.setup(triggerPin, GPIO.OUT)
GPIO.output(triggerPin, GPIO.HIGH)
time.sleep(1)
GPIO.output(triggerPin, GPIO.LOW)
time.sleep(1)

ser = serial.Serial('/dev/ttyAMA0', 115200) #RasPi3 Hardware Serial
print('Now using ' + str(ser.name) + ' for UART with Sushiboard.')
ser.write(b't')

GPIO.cleanup()
