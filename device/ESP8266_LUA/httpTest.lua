function tableToString(root)
   local buf='{' 
   local index=0
    --if type(v) == "table" then return end
    for k,v in pairs(root) do
       if index ~= 0 then buf=buf.."," end
       index=index+1
       buf=buf..'"'..k..'":'
       if type(v) == "table" then 
           buf=buf..'{' 
           local index_t=0
            --if type(v) == "table" then return end
            for k,v in pairs(v) do
               if index_t ~= 0 then buf=buf.."," end
               index_t=index_t+1
               buf=buf..'"'..k..'":"'..v..'"' 
            end
            buf=buf..'}'
       else
       buf=buf..'"'..v..'"' 
       end
    end
    buf=buf..'}'
    return buf
end

function working()
    -- Serving static files
    dofile('httpServer.lua')
    httpServer:listen(80)
    -- Get file
    httpServer:onRecv('/HelloHttp', function(req, res)
        print("use : Hello HttpServer")
        --res:sendFile('doge.jpg')
    end)
    httpServer:onRecv('/command', function(req, res)
        print('use command: GET len:'..#req.GET)
        print(tableToString(req.GET))
        res:send('success')
        --res:sendFile('doge.jpg')
    end)
end

wifi.setmode( wifi.STATION )
wifi.sta.config({ ssid = '360WiFi',pwd = '12345678' })
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
