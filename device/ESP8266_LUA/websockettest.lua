
function wsRecvProcess(msg)
    --msg='{"type":"SetConfig","data":{"has_lock":"1","open_stay_time":"4","lock_delay_time":"5","wifi_mode":"ap","wifi-ssid":"zy_em","wifi-pwd":"12345678","token":""}}'
    t = sjson.decode(msg)
    if t["type"]=="SetConfig" and t["data"]~=nil then
        print(t["data"]["wifi-ssid"])
        print(t["data"]["wifi-pwd"])
    else
        --print(recvData)
        --SendData to MasterDevice
    end
end

wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "zy_em", pwd = "12345678" })
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(infro)
  print("\n\tSTA - GOT IP".."\n\tStation IP: "..infro.IP.."\n\tSubnet mask: "..
  infro.netmask.."\n\tGateway IP: "..infro.gateway)
  ws_try_c = 1
  wsConnctTimer = tmr.create()
  wsConnctTimer:register(1000, 1, function()
  ws:connect('ws://192.168.20.103:3380')
    ws:on("connection", function(ws)
      print('got ws connection')
      ws_try_c=0
     --wsConnctTimer:unregister() --no unregister for reconnect
    end)
    ws:on("receive", function(_, msg, opcode)
      print('got message:', msg, opcode) -- opcode is 1 for text message, 2 for binary
      wsRecvProcess(msg)
    end)
    ws:on("close", function(_, status)
      print('connection closed', status)
      ws = nil -- required to Lua gc the websocket client
      ws_try_c=ws_try_c+1
      if ws_try_c>10 then print("try Connect Max.") end
      wsConnctTimer:start()
    end)
end)
wsConnctTimer:start()
end)

