
gpio.mode(1,gpio.INT, pullup)
gpio.write(1, gpio.HIGH)
local function pin1cb(level, pulse2)
gpio.trig(1)
gpio.write(1, gpio.LOW)
tmr.delay(1000000)
gpio.write(1, gpio.HIGH)
gpio.trig(1, "down", pin1cb)
print('keypress.')
--print( level, pulse2 - pulse1 )
--pulse1 = pulse2
--trig(pin, level == gpio.HIGH  and "down" or "up")
end
gpio.trig(1, "down", pin1cb)
