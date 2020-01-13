from gpiozero import DigitalOutputDevice
from flask import request
from flask_restful import reqparse, abort, Api, Resource

from ArdunoIF import *

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

SHELVES  = {
    '0': { 'id': 0 , 'name': 'Left 0',      'program': 'None' },
    '1': { 'id': 1 , 'name': 'Left 1',      'program': 'None' },
    '2': { 'id': 2 , 'name': 'Left 2',      'program': 'None' },
    '3': { 'id': 3 , 'name': 'Left 3',      'program': 'None' },
    '4': { 'id': 4 , 'name': 'Center Left', 'program': 'None' },
    '5': { 'id': 5 , 'name': 'Center Right', 'program': 'None' },
    '6': { 'id': 6 , 'name': 'Right 0',     'program': 'None' },
    '7': { 'id': 7 , 'name': 'Right 1',     'program': 'None' },
    '8': { 'id': 8 , 'name': 'Right 2',     'program': 'None' },
    '9': { 'id': 9 , 'name': 'Right 3',     'program': 'None' },
}

BOOKSHELF = {
        'power':'off',
        'shelves': [ SHELVES ]
}

def abort_if_shelf_doesnt_exist(shelfId):
    if shelfId not in SHELVES:
        abort(404, message="Shelf {} doesn't exist".format(shelfId))

# Returns a single shelf
class Shelf(Resource):
    def get(self, shelfId):
        abort_if_shelf_doesnt_exist(shelfId)
        return SHELVES[shelfId];
    
class Shelves(Resource):
    def get(self):
        return SHELVES

class Bookshelf(Resource):

    def get(self):
        return BOOKSHELF

    def put(self):
        jsonobj = request.get_json(force=True)
        print(jsonobj)
        if jsonobj['power'] == 'on':
            bookshelf[0].on()
            bookshelf[1].on()
            SendJsonCommand(wholeshelfonecolor);
            BOOKSHELF['power'] = 'on'
        elif jsonobj['power'] == 'off':
            bookshelf[0].off()
            bookshelf[1].off()
            BOOKSHELF['power'] = 'off'
        else:
            return '', 422

        return BOOKSHELF
