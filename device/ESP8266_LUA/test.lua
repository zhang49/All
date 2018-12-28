local ws = websocket.createClient()

recvtimer = tmr.create()
recvtimer:register(50, tmr.ALARM_SINGLE, function() 
  print("hey there") 
end)
--if not mytimer:start() then print("uh oh") end

wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "zy_em", pwd = "12345678" })
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
  print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
  infro.netmask.."\n\tGateway IP: "..infro.gateway)
  --ws:connect('wss://192.168.3.108:3380')
  hasConnected = 0
  TcpConnect = nil
  tmr.alarm(1, 1000, 1, function()
    Client = net.createConnection(net.TCP, 0) 
    Client:on("receive", function(Client, data) 
      print(data);
      --encoude data
      --send data to usart for stm32
      
      --uart.write(0,data)
    end)
    Client:on("connection", function(sck, c) 
      hasConnected = 1
      TcpConnect = Client
      print("Connect OK")
      tmr.stop(1)
    end)
    Client:on("disconnection", function(sck, c) 
      hasConnected = 0
      TcpConnect = nil
      print("Connect Error")
      tmr.start(1)
    end)
    Client:connect(8888,"192.168.42.236")
  end)
  if(hasConnected==0) then
    print("Connect Error Over...")
  end
end)

ws:on("connection", function(ws)
  print('got ws connection')
end)
ws:on("receive", function(_, msg, opcode)
  print('got message:', msg, opcode) -- opcode is 1 for text message, 2 for binary
end)
ws:on("close", function(_, status)
  print('connection closed', status)
  ws = nil -- required to Lua gc the websocket client
end)
