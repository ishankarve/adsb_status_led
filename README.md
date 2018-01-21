# adsb_status_led
A simple project in C that uses wiringPi library to blink led based on status of network connection and ADSB inputs.
To compile: 
gcc -Wall -O3 -o adsb_status_daemon adsb_status_daemon.c -lwiringPi
WiringPi library is required for compilation

