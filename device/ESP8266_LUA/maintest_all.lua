CONFIGFILENAME='Config.txt'
configData = {}
sendFileBuf = {}
saveSckBuf = {}
fileSendOverFlag = 0
fd = nil
function recvFromMasterDevice(msg)
  -- \n(10) ...... \r(13)
  --print("receive from uart:"..string.sub(data,0,#data-1))
  msg=string.sub(msg,2,#msg-1)
  print("recv data from STM32:"..msg)
  if configData.startmode == 'local' then
  elseif configData.startmode == 'cloud' then
  end
  sck=table.remove(saveSckBuf,1)
  if sck ~=nil then
    httpSend(sck,msg)
  else 
    --send to websocket    httpSend(ws,msg)???
  end
end
uart.on("data", "\r",function (data) recvFromMasterDevice(data) end, 0)
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
      for m,n in pairs(v) do
        if index_t ~= 0 then buf=buf.."," end
        index_t=1
        buf=buf..'"'..m..'":"'..n..'"' 
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
  end
  file.close()
  local ret,dd
  if file.open(fname,"r") then
    ret,dd = pcall(sjson.decode,file.read(2048))
    file.close()
  end
  if ret then return dd end
  return nil
end
--ws = websocket.createClient()
ws = net.createConnection(net.TCP, 0)
function handleRecv(sck,root)
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
		  configData.wifimode = 'ap'
		  configData.startmode = 'local'
		elseif root.data.wifi_mode == 0 then
		  configData.wifimode = 'station'
		  configData.startmode = 'cloud'
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
	  httpSend(sck, buf)
	elseif root.type=="GetWiFiConfig" then
	  local root = {}
	  local error_code = 0
	  local buf = ""
	  root.type = 'Reply_GetWiFiConfig'
	  root.data = {}
	  root.data.work_mode = (configData.wifimode == 'station' and 0 or 1)
	  root.data.wifi_ap_ssid = configData.ap.ssid
	  root.data.wifi_ap_pwd = configData.ap.pwd
	  root.data.wifi_station_ssid = configData.station.ssid
	  root.data.wifi_station_pwd = configData.station.pwd
	  buf=tableToString(root) 
	  buf = buf .. error_code ..'","error_str":"","data":""}'
	  httpSend(sck, buf)
	else
	  --print('this msg will send to Master:'..tableToString(root))
	  table.insert(saveSckBuf, #saveSckBuf+1, sck)
	  --print('#saveSckBuf:'..#saveSckBuf)
	  sendToMasterDevice(tableToString(root)..'\r\n')
	end
	return true
end
function wsRecvProcess(msg)
  --msg='{"type":"SetConfig","data":{"has_lock":"1","open_stay_time":"4","lock_delay_time":"5","wifi_mode":"ap","wifi-ssid":"zy_em","wifi-pwd":"12345678","token":""}}'
  local ok,root = pcall(sjson.decode,msg)
  if not ok then print("json encode error!") return nil end
  handleRecv(root)
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
function guessType(filename)
  local types = {
    ['.css'] = 'text/css', 
    ['.js'] = 'application/javascript', 
    ['.html'] = 'text/html',
    ['.png'] = 'image/png',
    ['.jpg'] = 'image/jpeg',
    ['.ico'] = 'image/jpeg'
  }
  for ext, type in pairs(types) do
    if string.sub(filename, -string.len(ext)) == ext
      or string.sub(filename, -string.len(ext .. '.gz')) == ext .. '.gz' then
      return type
    end
  end
  return 'text/plain'
end
-----------
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
  -- sends and removes the first element from the 'response' table
  local function send(localSocket)
  if response == nil or #response==0 then response = nil
  elseif #response > 0 then localSocket:send(table.remove(response,1)) end
  end
  -- triggers the send() function again once the first chunk of data was sent
  sck:on("sent", send)
  send(sck)
end
function closeHttpeSck(sck)
  sck:on('sent', function() end) -- release
  sck:on('receive', function() end)
  sck:close()
  sck=nil
  if fd ~= nil then fd:close() fd=nil end
end
function send(sck, data)
  local function doSend()
    if data == nil or data == '' then
      if sck ~= nil then closeHttpeSck(sck)
	  else
	    sck:close()
        sck:on('sent', function() end)
		sck = nil
	  end
    else
      sck:send(string.sub(data, 1, 512))
      data = string.sub(data, 512)
    end
  end
  sck:on('sent', doSend)
  doSend()
end
------------------closesck after sendover
function httpSend(sck,body)
  local status = 200
  local mType = mType or 'text/html'
  local buffer = 'HTTP/1.1 ' .. status .. '\r\n'
    .. 'Content-Type: ' .. mType .. '\r\n'
    .. 'Content-Length:' .. string.len(body) .. '\r\n'
  if redirectUrl ~= nil then buffer = buffer .. 'Location: ' .. redirectUrl .. '\r\n' end
  buffer = buffer .. '\r\n' .. body
  send(sck, buffer)
end
function sendResourceFile(sck,filename)
  local mType = guessType(filename)
  local status = 200
  if file.exists(filename .. '.gz') then filename = filename .. '.gz'
  elseif not file.exists(filename) then
    status=404
    if filename == '404.html' then sck:send(404) else sendResourceFile(sck,'404.html') end
    return nil
  end
  local header = 'HTTP/1.1 ' .. status .. '\r\n'
  header= header .. 'Cache-Control: public\r\n' -- cache
  header= header .. 'Cache-Control: max-age=3592000\r\n' -- cache
  header = header .. 'Content-Type: ' .. mType .. '\r\n'
  if string.sub(filename, -3) == '.gz' then header = header .. 'Content-Encoding: gzip\r\n' end
  header = header .. '\r\n'	-------improtant
  print(header)
  fd=file.open(filename, 'r')
  local function doSend()
        buf = fd.read(1460)
        if buf==nil then
		  table.remove(sendFileBuf, 1)
		  closeHttpeSck(sck)
          fileSendOverFlag = 0
        else
          sck:send(buf)
		  fileSendOverFlag = 1
        end
  end
  sck:on('sent', doSend)
  sck:send(header)
end
--[[
fileSendOverFlag set 1 when has send data, and fileSendOverFlag set 0 when send over
if fileSendOverFlag equal 200, it means that never send nothing in 2 seconds 
--]]
tmr.create():alarm(10,tmr.ALARM_AUTO,function()
  if fileSendOverFlag ~=0 then fileSendOverFlag=fileSendOverFlag+1 end
  if fileSendOverFlag == 0 and #sendFileBuf > 0 then
    fileSendOverFlag=1
    local tb=sendFileBuf[1]
    sendResourceFile(tb.s ,tb.f)
  elseif fileSendOverFlag>200 and #sendFileBuf > 0 then
    local tb = table.remove(sendFileBuf, 1)
    closeHttpeSck(tb.s)
	fileSendOverFlag=0
  end
end)
function parseRequestHeader(req)
  local _, _, method, path, vars = string.find(req.source, "([A-Z]+) (.+)?(.+) HTTP")
  _, _, method, path = string.find(req.source, "([A-Z]+) (.+) HTTP")
  if (method == 'POST') then	--POST Data is behind the tail
    _,_,vars=string.find(req.source,'\r\n\r\n(.*)')
  end
  req.GET=nil
  if ( vars ~=nil ) then
    req.GET={}
    vars=string.gsub(vars,'%%5B','[')
    vars=string.gsub(vars,'%%5D','')
    for k, v in string.gmatch(vars, "([^=]*)=([^&]*)&*") do
      if (string.find(k, '[\[]')) then 
        for f,b in string.gmatch(k,'(.-)[\[](.*)') do
          if req.GET[f]==nil then req.GET[f]={} end
          req.GET[f][b]=v
        end
      else
        req.GET[k]=v
      end
    end
  end
  req.method=method
  req.path=path
  return true
end
function startCloudMode()
  ws_try_c = 1
  --ws:connect('ws://'..ws_address..':'..ws_port)
  wsConnctTimer = tmr.create()
  wsConnctTimer:register(3000, tmr.ALARM_SEMI, function()
    local ip=configData.cloud.ip
    local port=tonumber(configData.cloud.port)
    ws:connect(port,ip)
    ws:on("connection", function(ws)
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
function startLocalMode()
  srv= net.createServer(net.TCP)
  srv:listen(80, function(conn)
  local buffer = {}
  conn:on('disconnection',function(sck)
    for i=#sendFileBuf,1,-1 do if sendFileBuf[i].s == sck then table.remove(sendFileBuf,i) break end
	end
  end)
  conn:on('receive', function(sck, msg)
  --print('recv Data---------------')
    local ip = sck:getpeer()
    if buffer.ip == nil then
      buffer.ip = msg
    else
      buffer.ip = buffer.ip .. msg
    end
	--print('--[['..buffer.ip..']]--')
	local i, reqData, nextstr
    i=string.find(buffer.ip,'\r\n\r\n')
    if i then
      if string.find(string.sub(buffer.ip, 1, 6), 'POST /') == 1 then
        local length, len
		_, _, len = string.find(buffer.ip, 'Content[\-]Length: ([0-9]+)')
        if len ~= nil then
          length = tonumber(len)
          i=i+4
          if #buffer.ip >= length+i-1 then
            reqData = string.sub(buffer.ip, 0, length+i-1)
            buffer.ip = string.sub(buffer.ip, length+i, -1)
          else collectgarbage() return nil end
		end
      else 
        _, _, reqData, nextstr = string.find(buffer.ip, '(.*)\r\n\r\n(.*)')
        buffer.ip=nextstr
      end
    else collectgarbage() return nil end
    ip=nil
    local req = { source = reqData, path = nil, method = nil, GET = {},ip = sck:getpeer() }
    if not parseRequestHeader(req) then collectgarbage() return nil end
    collectgarbage()
    print('recv path',req.path)
    local fname=string.sub(req.path,#req.path-string.find(string.reverse(req.path),"/")+2,#req.path)
    if not string.find(fname,'[\.]') then
      fname=nil
    end
    if fname then
      --sendResourceFile(sck,fname)
	  --sendFile by tmr
	   print('before insert #sendFileBuf = '..#sendFileBuf)
      table.insert(sendFileBuf, #sendFileBuf+1, { s = sck, f = fname})
    elseif req.path=='/' then
      --sendResourceFile(sck,'index.html')
    elseif req.path=='/command' then
      --print('recv command :'..tableToString(req.GET))
	  handleRecv(sck,req.GET)
	  --httpSend(sck,'{"test" : ""}')
    elseif req.path=='/login' then
     
    end
    collectgarbage() end) end) end

function working(startmode)
if startmode == nil then startmode=configData.startmode end
if startmode=="local" then startLocalMode() elseif startmode=="cloud" then startCloudMode() end
end
function start(wifimode)
  --use uart1(TX) for send data to Master Device
  uart.setup(1, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
  --use uart0(RX) for recv data from Master Device
  uart.setup(0, 115200, 8, uart.PARITY_NONE, uart.STOPBITS_1,0)
  configData=readConfigFromFile(CONFIGFILENAME)
  wifimode=configData.wifimode
  --wifimode="station"
  if wifimode == "ap" then
    wifi.setmode( wifi.SOFTAP )
    wifi.ap.config({ ssid = configData.ap.ssid,pwd = configData.ap.pwd })
    wifi.ap.dhcp.start()
    working()
  elseif wifimode == "station" then
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












