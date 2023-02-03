# This example takes images and streams them over a TCP socket to a host
# Example receiver is here: https://github.com/MichaelBell/rp2040_ov2640/blob/main/scripts/image_read.py

from picocamera import PicoCamera
from machine import Pin
import time
import socket
import network
import rp2
from secrets import SSID, PSK

# IP address of receiver
IP_ADDRESS = "192.168.0.88"

button = Pin(7, Pin.IN, Pin.PULL_UP)

# Note that currently you can only make one of these, so you need
# to reset the Pico between runs.  Not quite sure why - I think
# it also hangs if you try to make it after initing WiFi, so maybe that's the cause.
camera = PicoCamera()
image_size = 1600 * 1200 * 2

rp2.country("GB")
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, PSK)

start = time.ticks_ms()
while time.ticks_diff(time.ticks_ms(), start) < 30000:
    if wlan.status() < 0 or wlan.status() >= 3:
        break
    time.sleep(0.5)

print("Connected to WiFi")

while True:
    camera.capture_image(0)
    print("Image captured")

    len_left = image_size
    addr = 0

    sock = socket.socket()
    sock.connect((IP_ADDRESS, 4242))
    print("Connected to sock")

    start = time.ticks_ms()
    while len_left > 0:
        data = camera.read_data(0, addr, min(1024, len_left))
        len_left -= len(data)
        addr += len(data)
        sock.write(data)
    sock.close()
    print("Image sent at {:.2f}kB/s".format(image_size / (1.024 * (time.ticks_ms() - start))))

    while button.value():
        pass
    time.sleep(0.5)
