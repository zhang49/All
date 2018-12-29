configfilename="Config.txt"
cinfigData={}

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
function readConfigFromFile()
    if not file.exists("Config.txt") or file.open("Config.txt") and file.read() ==nil then
        file.open("Config.txt","w+")
        local table={}
        table["ap"]={}
        table["station"]={}
        table["wifimode"]="station"
        table["startmode"]="config"
        table["ap"]["ssid"]="ESP8266_Mode"
        table["ap"]["pwd"]="12345678"
        table["station"]["ssid"]="zy_em"
        table["station"]["pwd"]="12345678"
        local wbuf=tableToString(table)
        file.write(wbuf) 
        file.close()
        print("create file!")
    end
        file.close()
        file.open("Config.txt","r")
        content=file.read()
        cinfigData = sjson.decode(content)    
        file.close()   
end

function working(tar)
    if tar ~= nil then print("has parameter") end
    if cinfigData["startmode"]=="config" then
        print("working config")
        startConfigPage()
    elseif cinfigData["startmode"]=="normal" then
        print("working normal")
        startNormalPage()
    end
end

function start(wifimode)
    if workMode ~= nil then print("has parameter") end
    
    if true then
        wifi.setmode( wifi.SOFTAP )
        
        wifi.ap.config({ ssid = cinfigData["ap"]["ssid"],pwd = cinfigData["ap"]["pwd"] })
        wifi.ap.dhcp.start()
     
        working()
    elseif cinfigData["wifimode"] =="station" then
        wifi.setmode( wifi.STATION )
        print(cinfigData["station"]["ssid"].."__"..cinfigData["station"]["pwd"])
     
        wifi.sta.config({ ssid = cinfigData["station"]["ssid"],pwd = cinfigData["station"]["pwd"] })

        local wifi_try_c=2
        wifigotiptimer = tmr.create()
        wifigotiptimer:register(1000, 1, function()
            wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
                print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
                infro.netmask.."\n\tGateway IP: "..infro.gateway)
                wifi_try_c=0
                --don't unregister wifigotiptimer
                working()
            end)
            wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
                print("disconnect ssid:",infro.ssid)
                wifi_try_c=wifi_try_c+1
                --if wifi_try_c<=15 then --open error led
                wifigotiptimer:start()
                --end
            end)
       end)
       wifigotiptimer:start()
            
    end
end
        
function readfile(filepath)
    if file.exists(filepath) then
        if file.open(filepath, "r") then
            local line=file.readline()
            local content=""
            while( line ) 
            do
                content=content .. line
                line=file.readline()
            end
            file.close()
            return content
        end
    end
   return nil; 
end

function startConfigPage()
    srv = net.createServer(net.TCP)
    srv:listen(80, function(conn)
      conn:on("receive", function(client, request)
        local indexpage="index.html"
        local buf = ""
        local retcontent=""
        local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
        print(request)
        if (method == nil) then
          _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
        end
        local _GET = {}
        if (vars ~= nil) then
          for k, v in string.gmatch(vars, "(%w+)=(%w+)&*") do
            _GET[k] = v
          end
        end
        if(path == '/') then
            buf=readfile(indexpage)
        end
        
        if(path == '/favicon.ico') then
            buf=readfile('favicon.ico')
        end
        
        if(path == '/login') then
            buf = buf .. "<!DOCTYPE html>"
            buf = buf .. "<html><body><div style=\"width:500px;margin:0 auto\">"
            if(_GET.token ~=nil and _GET.ap ~= nil) then
                buf = buf .. "<p>identity is:".. _GET.token .."</p>"
                buf = buf .. "<p>ssid is:".. _GET.ap .."</p>"
                buf = buf .. "</div></body></html>"
                print(_GET.ap)
                --configData["ap"]["ssid"]=_GET.ap[1]
                --configData["ap"]["pwd"]=_GET.ap["pwd"]
                print("ap.ssid:"..configData["ap"]["ssid"])
                --print("ap.pwd:"..configData["ap"]["pwd"])
                --configData["station"]["ssid"]=_GET.station["ssid"]
                --configData["station"]["pwd"]=_GET.station["pwd"]
                if file.open(configfilename, "w+") then
                    --local wbuf=tableToString(table)
                   --file.write(wbuf) 
                    file.close()
                else
                    print("open error")
                end
            end   
            print(readfile(configfilename))        
        end
         client:send(buf)
      end)
    conn:on("sent", function(c) c:close() end)
    end)
end
function startNormalPage()

end
readConfigFromFile()
start()











