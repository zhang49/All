cinfigData={}
cinfigData["station"]={}
cinfigData["wifimode"]="station"
cinfigData["station"]["ssid"]="zy_em"
cinfigData["station"]["pwd"]="12345678"

function wificonnect()
    if cinfigData["wifimode"] =="ap" then
        wifi.setmode( wifi.SOFTAP )
        wifi.ap.config({ ssid = cinfigData["ap"]["ssid"],pwd = cinfigData["ap"]["pwd"] })
        wifi.ap.dhcp.config({ start = "192.168.4.100" })
        wifi.ap.dhcp.start()
        return 1
    elseif cinfigData["wifimode"] =="station" then
        wifi.setmode( wifi.STATION )
        print(cinfigData["station"]["ssid"].."  "..cinfigData["station"]["pwd"])
        wifiConnectTimer = tmr.create()
        wifi_try_c=0
        wifiConnectTimer:register(1, tmr.ALARM_SEMI, function()
           --wifi.sta.connect()
           print("wifigotiptimer start。")
           wifi.sta.config({ ssid = cinfigData["station"]["ssid"],pwd = cinfigData["station"]["pwd"] })
        end)
        
        wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
          print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP)
          --.."\n\tSubnet mask: "..infro.netmask.."\n\tGateway IP: "..infro.gateway
          wifi_try_c=0
          --unregister wifigotiptimer
        end)
        wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
          print("disconnect ssid:",infro.ssid)
          wifi_try_c=wifi_try_c+1
        --if wifi_try_c<=15 then --open error led
        wifiConnectTimer:start()
        --end
        end)
        wifiConnectTimer:start()
    end  
end
--wifi connect timer


wificonnect()


--[[dhcp_config ={}
dhcp_config.start = "192.168.1.100"
wifi.ap.dhcp.config(dhcp_config)--]]

