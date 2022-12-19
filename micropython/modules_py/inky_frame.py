from pimoroni import ShiftRegister
from machine import Pin
import time


SR_CLOCK = 8
SR_LATCH = 9
SR_OUT = 10

LED_A = 11
LED_B = 12
LED_C = 13
LED_D = 14
LED_E = 15


class Button:
    def __init__(self, sr, idx, led, debounce=50):
        self.sr = sr
        self.led = Pin(led, Pin.OUT)  # LEDs are just regular IOs
        self.led.off()
        self._idx = idx
        self._debounce_time = debounce
        self._changed = time.ticks_ms()
        self._last_value = None

    def read(self):
        value = self.raw()
        if value != self._last_value and time.ticks_ms() - self._changed > self._debounce_time:
            self._last_value = value
            self._changed = time.ticks_ms()
            return value
        return False

    def raw(self):
        return self.sr[self._idx] == 1

    @property
    def is_pressed(self):
        return self.raw()


sr = ShiftRegister(SR_CLOCK, SR_LATCH, SR_OUT)

button_a = Button(sr, 7, LED_A)
button_b = Button(sr, 6, LED_B)
button_c = Button(sr, 5, LED_C)
button_d = Button(sr, 4, LED_D)
button_e = Button(sr, 3, LED_E)