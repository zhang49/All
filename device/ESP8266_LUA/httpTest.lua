function working()
    -- Serving static files
    dofile('httpServer.lua')
    httpServer:listen(80)
    
    -- Get file
    httpServer:use('/doge', function(req, res)
        print("httpServer:use /doge")
        --res:sendFile('doge.jpg')
    end)
end

wifi.setmode( wifi.STATION )
wifi.sta.config({ ssid = '360WiFi-1AC8AE',pwd = '12345678' })
wifigotiptimer = tmr.create()
wifigotiptimer:register(1000, tmr.ALARM_SEMI, function()
  wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
    print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
    infro.netmask.."\n\tGateway IP: "..infro.gateway)
    wifi_try_c=0
    working()
  end) 
wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
    wifigotiptimer:start()
  end)
end)
wifigotiptimer:start()