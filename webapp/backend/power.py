from gpiozero import DigitalOutputDevice

class ShelfHalf:
    def __init__(self, pin):
        self.pin = DigitalOutputDevice(pin)
        
    def on(self):
        self.pin.on()
    
    def off(self):
        self.pin.off()
        
left = ShelfHalf(17)
right = ShelfHalf(27)
