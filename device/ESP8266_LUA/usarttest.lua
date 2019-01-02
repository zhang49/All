usartData=nil
--usart callback function
uart.on("data", "\r",
  function(data)
    -- \n(10) ...... \r(13)
    --print("receive from uart:"..string.sub(data,0,#data-1))
    msg=string.sub(data,2,#data-1)
    if msg == "command"  then print("is true") 
    else print("is false")
    end
end, 0)

