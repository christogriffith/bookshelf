from gpiozero import DigitalOutputDevice
from flask import request
from flask_restful import Resource

class ShelfHalf:
    def __init__(self, halfid, pin):
        self.halfid = halfid
        self.pin = DigitalOutputDevice(pin)
        self.pinnum = pin
        self.off()
        self.state = False
        
    def on(self):
        print("Setting pin %d to on" % (self.pinnum))
        self.pin.on()
        self.state = True
    
    def off(self):
        print("Setting pin %d to off" % (self.pinnum))
        self.pin.off()
        self.state = False

    def setstate(self, state):
        if state is 1:
            self.on()
        else:
            self.off()

bookshelf = [
    ShelfHalf(0, 27),
    ShelfHalf(1, 17)
]

class Bookshelf(Resource):

    def get(self, halfid=None):
        return { 's': [{'id':0, 'on': bookshelf[0].state}, {'id':1, 'on': bookshelf[1].state}]}

    def put(self, halfid):
        d = int(request.form['on'])
        if d > 1:
            return None, 404
        bookshelf[halfid].setstate(d)
        return {'id':halfid, 'on': bookshelf[halfid].state}, 200
