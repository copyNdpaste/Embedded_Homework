Welcomed 4x4 keypad for drum!
===================

You can enjoy drum sound by using 4x4 keypad with raspberry pi.

In source codes you have to import RPi.GPIO, os, sys, threading. Especially threading is used for simultanously playing. 4x4 keypad has 16 buttons. So PAD has 2x2 list that has sound file name. When you click the button, sounds are came out.

There are 6 drum functions. They are run by using the thread.

4x4 keypad's principle is like below

Attach matrix 8pins interface to 8 GPIO pins.
Setup column pins as outputs and set high.
Setup row pins as inputs with pull-up resistors, inputs are set high.
Set outputs low, one at a time.
When a button is pressed, input became low, indicating which button has been pressed.

Thank you.