
# blink LED connected to GPIO 18 on raspberry pi

import RPi.GPIO as GPIO
import time

LED = 12

GPIO.setmode(GPIO.BOARD)
GPIO.setup(LED, GPIO.OUT)

GPIO.output(LED, 1)
time.sleep(2)

GPIO.output(LED, 0)
time.sleep(1)

GPIO.cleanup()

