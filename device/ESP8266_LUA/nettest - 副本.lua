--send Data to Server
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

--Recv Data from Server
function wsRecvProcess(sck,msg)
    --msg='{"type":"SetConfig","data":{"has_lock":"1","open_stay_time":"4","lock_delay_time":"5","wifi_mode":"ap","wifi-ssid":"zy_em","wifi-pwd":"12345678","token":""}}'
    --print('recv message:', msg)
    local ok,t = pcall(sjson.decode,msg)
    if ok then
        if t["type"]=="SetConfig" and t["data"]~=nil then
            print(t["data"]["wifi-ssid"])
            print(t["data"]["wifi-pwd"])
        else
            --print(recvData)
            --SendData to MasterDevice 
        end
    else
    
            print('decode RecvData error.')
            local buf='123456789abcdefgh'
            local data=''
            local response={}
            for i=1,50,1 do
                data=data..buf
            end
            sendData(sck,msg)
        print('decode RecvData error.')--..msg..'"')
    end
end


wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "zy_em", pwd = "12345678" })
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
  print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
  infro.netmask.."\n\tGateway IP: "..infro.gateway)
  local ws_try_c = 1
  wsConnctTimer = tmr.create()
  ws = net.createConnection(net.TCP, 0)
  wsConnctTimer:register(1000, 2, function()
    ws:connect(3380,'192.168.20.103')
    ws:on("connection", function(ws)
      print('got ws connection')
      ws_try_c=0
     --wsConnctTimer:unregister() --no unregister for reconnect
    end)
    ws:on("receive", function(_, msg, opcode)
      --print('got message:', msg, opcode) -- opcode is 1 for text message, 2 for binary
      print(#msg)
      wsRecvProcess(ws,msg)
    end)
end)
wsConnctTimer:start()
end)

