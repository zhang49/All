wifigotiptimer = tmr.create()
wifi_try_c=0
 wifigotiptimer:register(1000, tmr.ALARM_AUTO, function()
print("test")
 end)
 wifigotiptimer:start()
