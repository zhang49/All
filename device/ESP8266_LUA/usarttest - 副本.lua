usartData=nil
--usart callback function
uart.on("data", "\r",
  function(data)
    --print("receive from uart:", data)
    usartData=data;
    if data=="quit" then
      uart.on("data") -- unregister callback function
    end
end, 0)
--check usart data
tmr.create():alarm(10,tmr.ALARM_AUTO ,function()
if(usartData ~= nil) then
print(usartData)
usartData=nil
end
end)
