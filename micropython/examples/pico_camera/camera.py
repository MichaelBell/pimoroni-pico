# This example takes images and streams them over a TCP socket to a host
# Example receiver is here: https://github.com/MichaelBell/rp2040_ov2640/blob/main/scripts/image_read.py

import picocamera
from machine import Pin
from sd import mount_sd
import time
import socket
import network
import rp2
from secrets import SSID, PSK

# IP address of receiver
IP_ADDRESS = "192.168.1.248"

button = Pin(7, Pin.IN, Pin.PULL_UP)

# Mount the SD card
mount_sd()

print("SD mounted")

# Create the camera before connecting to WiFi because the camera takes a couple of seconds to
# adjust after first initialization.
xfer_buffer = bytearray(4096)
picocamera.init(xfer_buffer)
image_size = picocamera.get_image_len()
picocamera.set_image_mode(picocamera.MODE_YUYV)

print("Camera created")

# Connect to WiFi
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

# Ensure the camera has had time to wake up
time_to_connect = time.ticks_diff(time.ticks_ms(), start)
if time_to_connect < 5000:
    time.sleep(5.0 - time_to_connect / 1000)

while True:
    # Capture an image
    picocamera.capture_image()
    print("Image captured")

    # Read image out to TCP socket
    len_left = image_size
    addr = 0

    sock = socket.socket()
    sock.connect((IP_ADDRESS, 4242))
    print("Connected to sock")

    start = time.ticks_ms()
    while len_left > 0:
        data = picocamera.read_data(addr, min(len(xfer_buffer), len_left))
        len_left -= len(data)
        addr += len(data)
        sock.write(data)
    sock.close()
    print("Image sent at {:.2f}kB/s".format(image_size / (1.024 * (time.ticks_ms() - start))))

    # Save image to SD card
    len_left = image_size
    addr = 0

    with open("/sd/image.raw", "wb") as image_file:
        start = time.ticks_ms()
        while len_left > 0:
            data = picocamera.read_data(addr, min(len(xfer_buffer), len_left))
            len_left -= len(data)
            addr += len(data)
            image_file.write(data)
        print("Image saved at {:.2f}kB/s".format(image_size / (1.024 * (time.ticks_ms() - start))))

    sock = socket.socket()
    sock.connect((IP_ADDRESS, 4242))
    print("Connected to sock")

    # Read image back from SD card to TCP socket
    with open("/sd/image.raw", "rb") as image_file:
        start = time.ticks_ms()
        mv = memoryview(xfer_buffer)
        while True:
            bytes_read = image_file.readinto(mv)
            if bytes_read > 0:
                sock.write(mv[0:bytes_read])
            else:
                break
        sock.close()
        print("Image read back at {:.2f}kB/s".format(image_size / (1.024 * (time.ticks_ms() - start))))

    # Wait for button press before taking next image
    while button.value():
        pass

    # Wait for a further half second before taking image
    # to reduce camera shake
    time.sleep(0.5)
