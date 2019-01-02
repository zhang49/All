configfilename="Config.txt"
configData={}
ws_address="192.168.20.2"
ws_port='3380'

uart.on("data", "\r",
function(data)
    -- \n(10) ...... \r(13)
    --print("receive from uart:"..string.sub(data,0,#data-1))
    msg=string.sub(data,2,#data-1)
    sendData()
end, 0)

function sendToMasterDevice(data)
    uart.write(1,data)
end

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
    if not file.exists(configfilename) or file.open(configfilename) and file.read() ==nil then
        file.open(configfilename,"w+")
        local table={}
        table["ap"]={}
        table["station"]={}
        table["wifimode"]="ap"
        table["startmode"]="config"
        table["ap"]["ssid"]="ESP8266_Mode"
        table["ap"]["pwd"]="12345678"
        table["station"]["ssid"]="zy_em"
        table["station"]["pwd"]="12345678"
        local wbuf=tableToString(table)
        file.write(wbuf) 
        file.close()
        node.restart()
        --print("create file!")
    end
        file.close()
        file.open(configfilename,"r")
        content=file.read()
        configData = sjson.decode(content)
        print(configData["station"]["ssid"])    
        file.close()   
end

function startNormalPage()
  ws_try_c = 1
  wsConnctTimer = tmr.create()
  wsConnctTimer:register(3000, 1, function()
  ws = websocket.createClient()
  ws:connect('ws://'..ws_address..':'..ws_port)
    ws:on("connection", function(ws)
      print('got ws connection')
      ws_try_c=0
     --wsConnctTimer:unregister() --no unregister for reconnect
    end)
    ws:on("receive", function(_, msg, opcode)
      --print('got message:', msg, opcode) -- opcode is 1 for text message, 2 for binary
      wsRecvProcess(msg)
      
    end)
    ws:on("close", function(_, status)
      print('connection closed', status)
      ws = nil -- required to Lua gc the websocket client
      ws_try_c=ws_try_c+1
      if ws_try_c>10 then print("try Connect Max.")
      else wsConnctTimer:start()
      end
    end)
end)
wsConnctTimer:start()
end

function wsRecvProcess(msg)
    --msg='{"type":"SetConfig","data":{"has_lock":"1","open_stay_time":"4","lock_delay_time":"5","wifi_mode":"ap","wifi-ssid":"zy_em","wifi-pwd":"12345678","token":""}}'
    ok, json=pcall(sjson.encode,msg)
    if ok then
        --SendData to MasterDevice
        print(msg..'\r\n')
        if t["type"]=="SetConfig" and t["data"]~=nil then
            configData["wifimode"]=t["data"]["wifi_mode"]
            local wifimode=t["data"]["wifi_mode"]
            if t["data"]["wifi-ssid"] ~=nil then
              --print(t["data"]["wifi_mode"])
              --print(t["data"]["wifi-ssid"])
              --print(t["data"]["wifi-pwd"])
              configData[wifimode]["ssid"]=t["data"]["wifi-ssid"]
              configData[wifimode]["pwd"]=t["data"]["pwd"]
            end
        else
            print("can't encode by Message:"msg)
        end
    else
        print("json encode error!")
    end
end


function working(startmode)
    if workmode == nil then
        startmode=configData["startmode"]
    end
    if startmode=="config" then
        --print("working config")
        startConfigPage()
    elseif startmode=="normal" then
        --print("working normal")
        startNormalPage()
    end
end

function start(wifimode)
    --use uart1(TX) for send data to Master Device
    uart.setup(1, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
    --use uart0(RX) for recv data from Master Device
    uart.setup(0, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
    readConfigFromFile()
    wifimode=configData["wifimode"]

    if wifimode == "ap" then
        wifi.setmode( wifi.SOFTAP )
        wifi.ap.config({ ssid = configData["ap"]["ssid"],pwd = configData["ap"]["pwd"] })
        wifi.ap.dhcp.start()
        working()
    elseif wifimode =="station" then
        wifi.setmode( wifi.STATION )
        print(configData["station"]["ssid"].."__"..configData["station"]["pwd"])
        wifi.sta.config({ ssid = configData["station"]["ssid"],pwd = configData["station"]["pwd"] })
        local wifi_try_c=1
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
                --print("disconnect ssid:",infro.ssid)
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


--send Data to Server,has test in websocket
function sendData(sck, data)
  local response = {}
    local sublen=255
    while true 
    do
        --a problem in print(string.sub(data,i,j)) , get data NULL
        ret=string.sub(data,0,sublen)
        response[#response + 1]=ret
        if #ret ~= sublen then break end
        data=string.sub(data,sublen+1,-1)
    end
  -- sends and removes the first element from the 'response' table
  local function send(localSocket)
    if #response > 0 then
      localSocket:send(table.remove(response, 1))
    else
      --localSocket:close()
      response = nil
    end
  end
  -- triggers the send() function again once the first chunk of data was sent
  sck:on("sent", send)
  send(sck)
end

function startConfigPage()
    srv = net.createServer(net.TCP)
    srv:listen(80, function(conn)
      conn:on("receive", function(client, request)
        local indexpage="index.html"
        local buf = ""
        local retcontent=""
        local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
        --print(request)
        if (method == nil) then
          _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
        end
        print("path:"..path)
        local _GET = {}
        if (vars ~= nil) then
            print("vars:"..vars)
            local index=1,rear,k,v
            while true
            do
              index=string.find (vars, "=")
              if index==nil then break;
              else 
                k=string.sub(vars,0,index-1)
                rear=string.find (vars, "&", index)
                if rear==nil then rear=#vars+1 end
                v=string.sub(vars,index+1,rear-1)
                vars=string.sub(vars,rear+1)
                _GET[k]=v
                print("key="..k.." , value="..v)
              end
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
            if(_GET.token ~=nil and _GET.apssid ~= nil) then
            print("is not nil2")
                buf = buf .. "<p>token is:".. _GET.token .."</p>"
                buf = buf .. "<p>ssid is:".. _GET.apssid .."</p>"
                buf = buf .. "<p>pwd is:".. _GET.appwd .."</p>"
                buf = buf .. "</div></body></html>"
                --print(_GET.ap)
                configData["token"]=_GET.token
                configData["wifimode"]="station";
                configData["station"]["ssid"]=_GET.apssid
                configData["station"]["pwd"]=_GET.appwd
                configData["startmode"]="normal"
                local wbuf=tableToString(configData)
                print(wbuf)
                if file.open(configfilename, "w+") then
                print("is not nil3")
                file.write(wbuf) 
                file.close()
                node.restart()
                return nil;
                else
                 --print("open error")
                end
                
            end   
            --print(readfile(configfilename))        
        end
         sendData(client,buf)
      end)
    --conn:on("sent", function(c) c:close() end)
    end)
end




start()











