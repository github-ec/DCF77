**DCF77 Simulator and Decoder**

A TimrOne driven DCF Simulator creates DCF77 data and transmits them on pin 9 of an Arduino UNO or NANO or the like.
Time and date can be set using the function
>simulator.setTime(23, 58, 29, 5, 26, 5);  // Fr, 27.03.24, 23:57

The pulse receiver does not use interrupts but has to be called in loop() in due time.
