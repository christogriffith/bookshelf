#! /usr/bin/env python3

import msgpack
import serial
import time

ARD_PORT = "/dev/ttyAMA0"
ARD_BAUD = 57600

data = {
    "cmd": "ranges",
    "shelf": 1,
    "leds":
    [
        { "s": 0, "e": 20, "c": { "r": 0, "g": 0, "b": 255 } },
        { "s": 20, "e": 40, "c": { "r": 0, "g": 255, "b": 0 } },
        { "s": 40, "e": 55, "c": { "r": 255, "g": 0, "b": 0 } }
    ]
}

wholeshelfonecolor = {
    "cmd":"entire",
    "c":{ "r": 127, "g": 127, "b": 127 }
}

serport = serial.Serial(ARD_PORT, ARD_BAUD, timeout=2)
serport.isOpen()

jstr = msgpack.dumps(wholeshelfonecolor)
chksum = sum(bytearray(jstr)) % 256
jstr = jstr + bytes([chksum])
msglen = len(jstr)
resp = ""
while not resp.startswith('ACK'):
    serport.write(bytes([msglen%256]))
    time.sleep(0.25)
    print("Writing %d bytes" % (len(jstr)))
    print(jstr)
    serport.write(jstr)
    resp = serport.readline().decode()
    print("Received %s" % resp)
    time.sleep(1)

print("Leaving")
