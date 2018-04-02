import RPi.GPIO as GPIO
from time import sleep
from sys import exit

import os,sys

import threading

GPIO.setmode(GPIO.BCM)

print "KEYPAD Ready!"

PAD = [ # Here are sound file names.
["clap-808","clap-analog","crash-acoustic","crash-tape"],
["hihat-808","hihat-acoustic01","hihat-dist01","hihat-dist02"],
["kick-808","kick-big","kick-cultivator","openhat-analog"],
["perc-hollow","perc-laser","ride-acoustic01","ride-acoustic02"]
]

ROW=[5,6,13,19]
COL=[12,16,20,21]

for j in range(4):
  GPIO.setup(COL[j], GPIO.OUT) #COL pins are OUTPUTS
  GPIO.output(COL[j], 1) #OUTPUTS are HIGH

for i in range(4):
  GPIO.setup(ROW[i], GPIO.IN, pull_up_down = GPIO.PUD_UP) #애매하게 눌렸으면 눌렸다고 침

def drum():
  try:
    while(True):
      for j in range(4):
        GPIO.output(COL[j],0) #LOW
        for i in range(4):
          if GPIO.input(ROW[i])==0: #button pressed
            print (PAD[i][j]) #print PAD value

            # play sound by using omxplayer
            # volume is -3000, sound files are in Music.
            os.system('omxplayer --vol -3000 Music/'+str(PAD[i][j])+'.wav')
            while(GPIO.input(ROW[i])==0): #If button pressed multiple times print only one time.
              pass
        GPIO.output(COL[j],1) #HIGH

  except KeyboardInterrupt:
    GPIO.cleanup()


def drum2():
  try:
    while(True):
      for j in range(4):
        GPIO.output(COL[j],0) #LOW
        for i in range(4):
          if GPIO.input(ROW[i])==0: #button pressed
            print (PAD[i][j]) #print PAD value
            os.system('omxplayer --vol -3000 Music/'+str(PAD[i][j])+'.wav')
            while(GPIO.input(ROW[i])==0): #If button pressed multiple times print only one time.
              pass
        GPIO.output(COL[j],1) #HIGH

  except KeyboardInterrupt:
    GPIO.cleanup()


def drum3():
  try:
    while (True):
      for j in range(4):
        GPIO.output(COL[j], 0)
        for i in range(4):
          if GPIO.input(ROW[i]) == 0:
            print (PAD[i][j])
            os.system('omxplayer --vol -1500 --display 10 drum/' + str(PAD[i][j]) + '.wav')
            while (GPIO.input(ROW[i]) == 0):
              pass
        GPIO.output(COL[j], 1)

  except KeyboardInterrupt:
    GPIO.cleanup()


def drum4():
  try:
    while (True):
      for j in range(4):
        GPIO.output(COL[j], 0)
        for i in range(4):
          if GPIO.input(ROW[i]) == 0:
            print (PAD[i][j])
            os.system('omxplayer --vol -1500 --display 10 drum/' + str(PAD[i][j]) + '.wav')
            while (GPIO.input(ROW[i]) == 0):
              pass
        GPIO.output(COL[j], 1)

  except KeyboardInterrupt:
    GPIO.cleanup()


def drum5():
  try:
    while (True):
      for j in range(4):
        GPIO.output(COL[j], 0)
        for i in range(4):
          if GPIO.input(ROW[i]) == 0:
            print (PAD[i][j])
            os.system('omxplayer --vol -1500 --display 10 drum/' + str(PAD[i][j]) + '.wav')
            while (GPIO.input(ROW[i]) == 0):
              pass
        GPIO.output(COL[j], 1)

  except KeyboardInterrupt:
    GPIO.cleanup()


def drum6():
  try:
    while (True):
      for j in range(4):
        GPIO.output(COL[j], 0)
        for i in range(4):
          if GPIO.input(ROW[i]) == 0:
            print (PAD[i][j])
            os.system('omxplayer --vol -1500 --display 10 drum/' + str(PAD[i][j]) + '.wav')
            while (GPIO.input(ROW[i]) == 0):
              pass
        GPIO.output(COL[j], 1)

  except KeyboardInterrupt:
    GPIO.cleanup()


thread1 = threading.Thread(target=drum)
thread1.start()

thread2 = threading.Thread(target=drum2)
thread2.start()

thread3 = threading.Thread(target=drum3)
thread3.start()

thread4 = threading.Thread(target=drum4)
thread4.start()

thread5 = threading.Thread(target=drum5)
thread5.start()

thread6 = threading.Thread(target=drum6)
thread6.start()