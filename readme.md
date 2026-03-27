**DCF77 Simulator and Decoder**

A TimrOne driven DCF Simulator creates DCF77 data and transmits them on pin 9 of an Arduino UNO or NANO or the like.
Time and date can be set using the function
>simulator.setTime(23, 58, 29, 5, 26, 5);  // Fr, 27.03.24, 23:57

The pulse receiver does not use interrupts but has to be called in loop() in due time.

For testing just connect pin 2 (Receiver) and pin 9 (Simulator):

<img width="357" height="282" alt="image" src="https://github.com/user-attachments/assets/fb2b2c1a-0e36-49ec-8a68-35921b1b728d" />
