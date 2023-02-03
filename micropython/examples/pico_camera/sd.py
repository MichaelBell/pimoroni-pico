# This requires micropython-lib/drivers/storage/sdcard to either be
# baked into the firmware or copied alongside this file.
import machine
import sdcard
import uos


def mount_sd(baudrate=20000000):
    # Assign chip select (CS) pin (and start it high)
    cs = machine.Pin(26, machine.Pin.OUT)

    # Intialize SPI peripheral
    spi = machine.SPI(0,
                      baudrate=100000,           # SD Card will increase this once initialized
                      polarity=0,
                      phase=0,
                      bits=8,
                      firstbit=machine.SPI.MSB,
                      sck=machine.Pin(18),
                      mosi=machine.Pin(19),
                      miso=machine.Pin(20))

    # Initialize SD card
    sd = sdcard.SDCard(spi, cs, baudrate)

    # Mount filesystem
    vfs = uos.VfsFat(sd)
    uos.mount(vfs, "/sd")
