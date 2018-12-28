spi.setup(1, spi.MASTER, spi.CPOL_HIGH, spi.CPHA_HIGH, 8,8)

tmr.create():alarm(6870945,tmr.ALARM_AUTO ,function()
dd=spi.recv(1,1,0xaa)
print(dd)
end)


tmr.create():alarm(3000,tmr.ALARM_AUTO ,function()
dd=spi.send(1,0,255,255,255)
print(dd)
end)



   
