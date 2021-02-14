#! /usr/bin/env python3

import serial
import time

ARD_PORT = "/dev/ttyAMA0"
ARD_BAUD = 57600

serport = serial.Serial(ARD_PORT, ARD_BAUD, timeout=2)
serport.isOpen()

jstr = [ 0x05, 0x69, 0x00, 0x09,
         0xFF, 0x00, 0x00,
         0x00, 0xFF, 0x00,
         0x00, 0x00, 0xFF ]

msglen = len(jstr)
resp = ""
while True:
    print("Writing %d bytes" % (len(jstr)))
    print(jstr)
    serport.write(jstr)
    time.sleep(0.200) # 200 ms
    temp = jstr[4]
    jstr[4] = jstr[5]
    jstr[5] = jstr[6]
    jstr[6] = temp
    
    temp = jstr[7]
    jstr[7] = jstr[8]
    jstr[8] = jstr[9]
    jstr[9] = temp
    
    temp = jstr[10]
    jstr[10] = jstr[11];
    jstr[11] = jstr[12];
    jstr[12] = temp;
        

print("Leaving")
