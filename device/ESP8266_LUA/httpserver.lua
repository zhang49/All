
function start(target)
    if target=="config" then
        wifi.setmode(wifi.AP)
        wifi.ap.config({ ssid = "ESP8266_Config", pwd = "12345678" })
        wifi.ap.dhcp.start()
        startConfigServerPage()
    elseif target=="work" then
        config=readfile("config")
        wifi.setmode(wifi.STATION)
        wifi.ap.config({ ssid = "ESP8266_Config", pwd = "12345678" })
        wifi.ap.dhcp.start()
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

wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "GamePartment", pwd = "game1234" })
print(wifi.sta.getip())
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
        if(_GET.identity ~=nil ) then
        print(_GET.identity)
        buf = buf .. "<p>identity is:".. _GET.identity .."</p>"
        end
        if(_GET.ssid ~=nil ) then
        print(_GET.ssid)
        buf = buf .. "<p>ssid is:".. _GET.ssid .."</p>"
        end
        if(_GET.psw ~=nil ) then
        print(_GET.psw)
        buf = buf .. "<p>psw is:".. _GET.psw .."</p>"
        end
        buf = buf .. "</div></body></html>"
    end
   local _on, _off = "", ""
     client:send(buf)
  end)
  conn:on("sent", function(c) c:close() end)
end)













