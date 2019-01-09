configfilename = "Config.txt"
configData = {}
saveHttpSckBuf = {}
recvFMD_count = 0
uart1_recvbuf = ""
function sendToMasterDevice(data)
  print('send data to STM32:'..data)
  uart.write(1,data)
end

function recvFromMasterDevice(msg)
  --stm32发送过来的消息末尾带 \r\n
  if msg == '\n' then return nil end
  if msg:byte(#msg-1) == 0xff and msg:byte(#msg) == 65 then
    msg=string.sub(msg,0,-3)
	uart1_recvbuf = uart1_recvbuf .. msg
    print("recv single data over:"..uart1_recvbuf:byte(#uart1_recvbuf).."\r\nmsg:"..uart1_recvbuf)
	if configData.startmode == 'local' then
      sck = table.remove(saveHttpSckBuf,1)
      if sck ~=nil then
	    httpSend(sck, uart1_recvbuf)
	  end
	  uart1_recvbuf = ""
    elseif configData.startmode == 'cloud' then
	
    end
  else 
    uart1_recvbuf = uart1_recvbuf .. msg
  end
end

--当接收到\n或者接收数达到最大值255时调用function
uart.on("data", "A", function (data) recvFromMasterDevice(data) end, 0)

function tableToString(root)
  local buf = '{' 
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
  local total_allocated, estimated_used = node.egc.meminfo()
  print('----------before decode json:'..total_allocated..'_use:'..estimated_used)
  if file.open(fname,"r") then
    ret,dd = pcall(sjson.decode,file.read(3096))
    file.close()
  end
  
  local endtotal_allocated, estimated_used = node.egc.meminfo()
  print('----------after decode json:'..total_allocated..'_use:'..estimated_used)
  if ret then return dd end
  return nil
end

function handleRecv(res,root)
    --SendData to MasterDevice
    if root.type=="SetWiFiConfig" and root.data~=nil then
	  local buf = '{"type":"Reply_SetWiFiConfig","error_code":"'
	  local error_code = 0
      if root.data.wifi_ap_ssid == nil or root.data.wifi_ap_pwd == nil or root.data.wifi_station_ssid == nil
	    or root.data.wifi_station_pwd == nil or root.data.wifi_mode == nil then
		error_code = 1
	  else
	    configData.ap.ssid = root.data.wifi_ap_ssid
        configData.ap.pwd = root.data.wifi_ap_pwd
        configData.station.ssid = root.data.wifi_station_ssid
        configData.station.pwd = root.data.wifi_station_pwd
	    if root.data.wifi_mode == 1 then
		  configData.startmode = 'local'
		  configData.wifimode = 'ap'
		elseif root.data.wifi_mode == 0 then
		  configData.startmode = 'cloud'
		  configData.wifimode = 'station'
		end
		if false and file.open(CONFIGFILENAME, "w+") then
		  --file.write(wbuf)
		  file.close()
		  --wheather here send response to net?
		  tmr.create:alarm(200, tmr.ALARM_SINGLE , function()
            node.restart()
		  end)
	    else
		  error_code = 1
          --print("open error")
        end
	  end
	  buf = buf .. error_code ..'","error_str":"","data":""}'
	  res:send(buf)
	elseif root.type=="GetWiFiConfig" then
	  local root = {}
	  local error_code = 0
	  local buf = ""
	  root.type = 'Reply_GetWiFiConfig'
	  root.error_str = ""
	  root.error_code = error_code
	  root.data = {}
	  root.data.work_mode = (configData.startmode == 'cloud' and 0 or 1)
	  root.data.wifi_ap_ssid = configData.ap.ssid
	  root.data.wifi_ap_pwd = configData.ap.pwd
	  root.data.wifi_station_ssid = configData.station.ssid
	  root.data.wifi_station_pwd = configData.station.pwd
	  buf=tableToString(root) 
	  res:send(buf)
	else
	  --print('this msg will send to Master:'..tableToString(root))
	  --table.insert(saveHttpSckBuf, #saveHttpSckBuf+1, sck)
	  --print('#saveHttpSckBuf:'..#saveHttpSckBuf)
	  table.insert(saveHttpSckBuf, #saveHttpSckBuf+1, res._sck)
	  sendToMasterDevice(tableToString(root)..'\r\n')
	end
	return true
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
  dofile('httpServer.lc')
  httpServer:listen(80)
  httpServer:onRecv('/', function(req, res)
    res:sendFile('index.html')
  end)
  httpServer:onRecv('/command', function(req, res)
    handleRecv(res,req.GET)
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
tmr.create():alarm(10,tmr.ALARM_AUTO,function()

end)
function working(startmode)
  local total_allocated, estimated_used = node.egc.meminfo()
  print('----------after working:'..total_allocated..'_use:'..estimated_used)
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










