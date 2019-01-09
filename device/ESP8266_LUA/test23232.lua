
function recvFromMasterDevice(msg)
if msg:byte(#msg-1) == 0xff and msg:byte(#msg) == 0x36 then
    msg=string.sub(msg,0,-3)
    uart1_recvbuf = uart1_recvbuf .. msg
    print('recv end :'..uart1_recvbuf)
    uart1_recvbuf = ""
  else 
    uart1_recvbuf = uart1_recvbuf .. msg
  end
end

--当接收到\r或者接收数达到最大值255时调用function
tmr.create():alarm(500,tmr.ALARM_SINGLE,function ()
uart.on("data",'6' ,function (data) recvFromMasterDevice(data) end, 0)
end)

