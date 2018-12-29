configfilename="config.txt"
function getFileConfig()
     if file.exists(configfilename) then
        if file.open(configfilename, "r") then
            line=file.readline()
            content=""
            cfg={}
            i=0
            while( line )
            do
                k=string.sub(line, 0,string.find(line, "=")-1)
                v=string.sub(line, string.find(line, "=.*")+1)
                v=v.sub(v, 0,string.len(v)-1)
                cfg[k]=v
                line=file.readline()
            end
            file.close()
        end
        return cfg
    end
    return nil;
end


function start(target,cfg)
    if target=="config" then
        wifi.setmode( wifi.SOFTAP )
        wifi.ap.config({ ssid = "ESP8266_Config", pwd = "12345678" })
        wifi.ap.dhcp.start()
        
        startConfigServerPage()
    elseif target=="work" then
        config=readfile("config")
        wifi.setmode( wifi.STATION )
        print(cfg["ssid"].."__"..cfg["pwd"])
        wifi.sta.config({ ssid = cfg["ssid"],pwd = cfg["pwd"] })
        wifi_try_c=2
        wifigotiptimer = tmr.create()
        wifigotiptimer:register(1000, 1, function()
            wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
                print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
                infro.netmask.."\n\tGateway IP: "..infro.gateway)
                wifi_try_c=0
                --unregister wifigotiptimer
                wifigotiptimer:stop()
                startConfigServerPage()
            end)
            wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(infro)
                print("disconnect ssid:",infro.ssid)
                wifi_try_c=wifi_try_c+1
                if wifi_try_c<=15 then --open error led
                wifigotiptimer:start()
                end
            end)
       end)
       wifigotiptimer:start()
            
    end
end
        
function readfile(filepath)
    if file.exists(filepath) then
        if file.open(filepath, "r") then
            line=file.readline()
            content=""
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
function startConfigServerPage()
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
        if(path == "/") then
            buf=readfile(indexpage)
        end
        if(path == "/login") then
            buf = buf .. "<!DOCTYPE html>"
            buf = buf .. "<html><body><div style=\"width:500px;margin:0 auto\">"
            if(_GET.identity ~=nil and _GET.ssid ~= nil and _GET.pwd ~= nil) then
                buf = buf .. "<p>identity is:".. _GET.identity .."</p>"
                buf = buf .. "<p>ssid is:".. _GET.ssid .."</p>"
                buf = buf .. "<p>psw is:".. _GET.pwd .."</p>"
                buf = buf .. "</div></body></html>"
                print("id:".._GET.identity.."\r\nssid:".._GET.ssid.."\r\npsw:".._GET.pwd)
                if file.open(configfilename, "w+") then
                    file.writeline("id=".._GET.identity);
                    file.writeline("ssid=".._GET.ssid);
                    file.writeline("pwd=".._GET.pwd);
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


cfg=getFileConfig()
if cfg ~= nil then 
    start("work",cfg)
    end










