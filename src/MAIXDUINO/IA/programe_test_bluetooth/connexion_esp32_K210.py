import time, network, utime
from Maix import GPIO,
from fpioa_manager import fm
#IO map of the PIN2


class wifi():
    # IO map for ESP32 on Maixduino
    fm.register(8,fm.fpioa.GPIOHS11)#en
    fm.register(9,fm.fpioa.GPIOHS12)#rdy
    fm.register(28,fm.fpioa.SPI1_D0, force=True)#mosi
    fm.register(26,fm.fpioa.SPI1_D1, force=True)#miso
    fm.register(27,fm.fpioa.SPI1_SCLK, force=True)#sclk
    fm.register(25,fm.fpioa.GPIOHS10)#cs
    nic = network.ESP32_SPI(cs=fm.fpioa.GPIOHS10, rst=fm.fpioa.GPIOHS11, rdy=fm.fpioa.GPIOHS12, spi=1)
    
print("ESP32_SPI firmware version:", wifi.nic.version())
