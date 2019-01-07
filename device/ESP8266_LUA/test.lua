--dofile('httpServer.lc')
function working()
  srv = net.createServer(net.TCP)
  --print(buf)
  srv= net.createServer(net.TCP)
  srv:listen(8000, function(conn)
  local pos=0
  fd=file.open('jquery-weui.js','r')
  conn:on('receive', function(sck, msg)
    sck:on('sent',function()
    buf=fd:seek('set',pos)
    buf=fd:read(1024)
    pos=pos+1024
    if pos>200000 then pos=0 end
    sck:send(buf)
    end)
    sck:send('1')
    end)
  end)
end




i=0
--tmr.create():alarm(1,tmr.ALARM_AUTO,function() i=i+1 end)
function ftest()
  --buf=fd:read(1024)
  
  local pos=0
  while true
  do
  fd=file.open('jquery-weui.js','r')
  fd:seek('set',pos)
    buf=fd:read(1024)
    pos=pos+1024
    --print(buf)
    if buf==nil then break end
    fd:close()
  end
  print('result:'..i)  
end


wifi.setmode( wifi.STATION )
wifi.sta.config({ ssid = '360WiFi-1AC8AE',pwd = '12345678' })
local wifi_try_c=1
wifigotiptimer = tmr.create()
wifigotiptimer:register(1000, tmr.ALARM_SEMI, function()
  wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
    print("\n\tSTATION - GOT IP: "..infro.IP)
    --"\n\tSubnet mask: "..infro.netmask.."\n\tGateway IP: "..infro.gateway)
    wifi_try_c=0
    --don't unregister wifigotiptimer
    working()
  end)
  wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
    --print("disconnect ssid:",infro.ssid)
    wifi_try_c=wifi_try_c+1
    --if wifi_try_c<=15 then --open error led
    wifigotiptimer:start()
    --end
  end)
end)
wifigotiptimer:start()