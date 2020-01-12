#! /usr/bin/env python3

import msgpack
import serial

ARD_PORT = "/dev/ttyAMA0"
ARD_BAUD = 57600

data = {
    "cmd": "ranges",
    "shelf": 3,
    "leds":
    [
        { "s": 0, "e": 20, "c": { "r": 0, "g": 0, "b": 255 } },
        { "s": 20, "e": 40, "c": { "r": 0, "g": 255, "b": 0 } },
        { "s": 40, "e": 55, "c": { "r": 255, "g": 0, "b": 0 } }
    ]
}

wholeshelfonecolor = {
    "cmd":"entire",
    "color":{ "r": 255, "g": 0, "b": 0 }
}

serport = serial.Serial(ARD_PORT, ARD_BAUD)
serport.isOpen()

jstr = msgpack.dumps(data)
chksum = sum(bytearray(jstr)) % 256
jstr = jstr + bytes([chksum])
msglen = len(jstr)
jstr = bytes([msglen%256]) + jstr
print("Writing %d bytes" % (len(jstr)))
print(jstr)
serport.write(jstr)

