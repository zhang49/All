
function readConfigFromFile(fname)
fname='Config.txt'
  if not file.exists(fname) then
    file.open(fname,'w')
    local cfg={}
    cfg.ap={}
    cfg.station={}
    cfg.cloud={}
    cfg.wifimode="station"
    cfg.startmode="local"
    cfg.ap.ssid="ESP8266_Mode"
    cfg.ap.pwd="12345678"
    cfg.station.ssid="360WiFi-1AC8AE"
    cfg.station.pwd="12345678"
    cfg.cloud.ip="192.168.20.2"
    cfg.cloud.port="3380"
    local wbuf=tableToString(cfg)
    file.write(wbuf) 
    file.close()
    --node.restart()
    --print("create file!")
  end
  file.close()
  local ret,dd
  if file.open(fname,"r") then
    ret,dd = pcall(sjson.decode,file.read(3096))
    file.close()
  end
  if ret then return dd end
  return nil
end

function working()
    total_allocated, estimated_used = node.egc.meminfo()
    print('----------before working:'..total_allocated..'_use:'..estimated_used)
    -- Serving static files
    dofile('httpServer.lua')
    httpServer:listen(80)
    -- Get file
    httpServer:onRecv('/HelloHttp', function(req, res)
        print("use : Hello HttpServer")
        --res:sendFile('doge.jpg')
    end)
end
function test()
wifi.setmode( wifi.STATION )
wifi.sta.config({ ssid = '360WiFi-1AC8AE',pwd = '12345678' })
wifigotiptimer = tmr.create()
wifigotiptimer:register(1000, tmr.ALARM_SEMI, function()
  wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
    print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
    infro.netmask.."\n\tGateway IP: "..infro.gateway)
    local wifi_try_c=0
    working()
  end)
  wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
   wifigotiptimer:start()

  end)
  end)
wifigotiptimer:start()
  end 
local i=0
total_allocated, estimated_used = node.egc.meminfo()
print('----------before working:'..total_allocated..'_use:'..estimated_used)
    test()
