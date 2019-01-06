configfilename="Config.txt"
configData={}
uart.on("data", "\r",function(data)
  -- \n(10) ...... \r(13)
  --print("receive from uart:"..string.sub(data,0,#data-1))
  msg=string.sub(data,2,#data-1)
  print("recv data from STM32:"..msg)
  sendData(ws,msg)
end, 0)

function sendToMasterDevice(data)
  print('send data to STM32:'..data)
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

function readConfigFromFile(fname)
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
    cfg.station.ssid="Ares"
    cfg.station.pwd="460204415"
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

--ws = websocket.createClient()
ws = net.createConnection(net.TCP, 0)
function startCloudMode()
  ws_try_c = 1
  --ws:connect('ws://'..ws_address..':'..ws_port)
  wsConnctTimer = tmr.create()
  wsConnctTimer:register(3000, tmr.ALARM_SEMI, function()
    local ip=configData.cloud.ip
    local port=tonumber(configData.cloud.port)
    ws:connect(port,ip)
    ws:on("connection", function(ws)
      print('got ws connection')
      ws_try_c=0
      --wsConnctTimer:unregister() --no unregister for reconnect
    end)
    ws:on("receive", function(_, msg, opcode)
      --print('got message:', msg, opcode) -- opcode is 1 for text message, 2 for binary
      wsRecvProcess(msg)
    end)
	--close then reconnect
  end)
  wsConnctTimer:start()
end

function wsRecvProcess(msg)
  --msg='{"type":"SetConfig","data":{"has_lock":"1","open_stay_time":"4","lock_delay_time":"5","wifi_mode":"ap","wifi-ssid":"zy_em","wifi-pwd":"12345678","token":""}}'
  local ok,t = pcall(sjson.decode,msg)
  print(msg)
  if ok then
    --SendData to MasterDevice
    if t.type=="SetConfig" and t.data~=nil then
      configData.wifimode=t.data.wifi_mode
      local wifimode=t.data.wifi_mode
      if t.data.wifi_ssid ~=nil then
        --print(t.data.wifi_mode)
        --print(t.data.wifi_ssid)
        --print(t.data.wifi_pwd)
        configData[wifimode].ssid=t.data.wifi_ssid
        configData[wifimode].pwd=t.data.wifi_pwd
      end
    else
      sendToMasterDevice(msg..'\r\n')
    end
  else
    print("json encode error!")
  end
end
 
function readfile(filepath)
  if file.exists(filepath) then
    local fd = file.open(filepath, "r")
    if fd then
	  local buf,content="",""
	  while buf~=nil
	  do
	    content=content..buf
		buf=fd:read(1024)
	  end
	  fd:close()
	  fd=nil
      return content
    end
  end
  return nil; 
end

function sendData(sck, data)
  local response = {}
  local sublen=254
  local ret=""
  while true 
  do
    --a problem in print(string.sub(data,i,j)) , get data NULL
    ret=string.sub(data,0,sublen)
    if(ret~=nil) then
      response[#response + 1]=ret
      data=string.sub(data,sublen+1,-1)
    end
    if ret==nil or #ret~=sublen then break end 
  end
  print('-')
  -- sends and removes the first element from the 'response' table
  local function send(localSocket)
    if response == nil or #response==0 then 
      response = nil
    elseif #response > 0 then
      localSocket:send(table.remove(response,1))
    end
  end
  -- triggers the send() function again once the first chunk of data was sent
  sck:on("sent", send)
  send(sck)
end

function startLocalMode()
  dofile('httpServer.lua')
  httpServer:listen(80)
  httpServer:onRecv('/command', function(req, res)
  print('on command: GET len:'..#req.GET)
  print(tableToString(req.GET))
    res:send('success')
  end)
  httpServer:onRecv('/test', function(req, res)
    res:sendResourceFile('test.html')
  end)
  httpServer:onRecv('/login', function(req, res)
    local buf = buf .. "<!DOCTYPE html>"
    buf = buf .. '<html><body><div style="width:500px;margin:0 auto">'
    if(req.GET.token ~=nil and req.GET.apssid ~= nil) then
      print("post data is not")
      buf = buf .. "<p>token is:".. req.GET.token .."</p>"
      buf = buf .. "<p>ssid is:".. req.GET.apssid .."</p>"
      buf = buf .. "<p>pwd is:".. req.GET.appwd .."</p>"
     buf = buf .. "<p>close ap and start station now</p>"
      buf = buf .. "</div></body></html>"
      --print(req.GET.ap)
      configData.token=req.GET.token
      configData.wifimode="station";
      configData.station.ssid=req.GET.apssid
      configData.station.pwd=req.GET.appwd
      configData.startmode="local"
      local wbuf=tableToString(configData)
      print(wbuf)
      if file.open(configfilename, "w+") then
        file.write(wbuf) 
        file.close()
        res:send(client,buf)
        --waiting for senddata over
        tmr.create:alarm(200, tmr.ALARM_SINGLE , function()
          node.restart()
        end)
        return nil
      else
        --print("open error")
      end
    end   
    --print(readfile(configfilename))      
  end)
      --local function sw_root() print("root") end
      --local mswitch={
      --[""]=sw_index,
      --["command"]=sw_command,
      --["login"]=sw_login,
      --["root"]=sw_root
      --}
    --local sw=mswitch[temp]
    --if sw then sw()
    --else print("not find :"..temp)
    --end
end

function working(startmode)
  if startmode == nil then
    startmode=configData.startmode
  end
  if startmode=="local" then
    startLocalMode()
  elseif startmode=="cloud" then
    startCloudMode()
  end
end

function start(wifimode)
  --use uart1(TX) for send data to Master Device
  uart.setup(1, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
  --use uart0(RX) for recv data from Master Device
  uart.setup(0, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
  configData=readConfigFromFile(configfilename)
  wifimode=configData.wifimode
  --wifimode="station"
  if wifimode == "ap" then
    wifi.setmode( wifi.SOFTAP )
    wifi.ap.config({ ssid = configData.ap.ssid,pwd = configData.ap.pwd })
    wifi.ap.dhcp.start()
    working()
  elseif wifimode =="station" then
    wifi.setmode( wifi.STATION )
    print(configData.station.ssid.."__"..configData.station.pwd)
    wifi.sta.config({ ssid = configData.station.ssid,pwd = configData.station.pwd })
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
  elseif wifimode == 'both' then
    wifi.setmode(wifi.STATIONAP)
	working()
  end
end

start()












