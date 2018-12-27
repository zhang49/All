spi.transaction(0, 8, "222", 8, "22", 8, 8, 8)
dat={}
dat = spi.get_miso(0, 0xb0000,500,255)
print(dat)