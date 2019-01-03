sendDataOver=-1
function sendData(sck, data)
  while (sendDataOver==0) do 
    print("wating...")
    tmr.delay(1) end
  sendDataOver=0
  local response = {}
  local sublen=255
  local ret=""
  local i=0
  print(#data)
  while true 
  do
    --a problem in print(string.sub(data,i,j)) , get data NULL
    ret=string.sub(data,0,sublen)
    if(ret~=nil) then
      response[#response + 1]=ret
      data=string.sub(data,sublen+1,-1)
      i=i+1
    end
    if ret==nil or #ret~=sublen then 
      print("run:"..i)
      break 
    end
  end
  -- sends and removes the first element from the 'response' table
  print('#response:'..#response)
  sendDataOver=1
  local function send(localSocket)
    if response == nil then 
      sendDataOver=1
      return nil 
    end
    if #response > 0 then
      localSocket:send(table.remove(response,1))
    else
      --localSocket:close()
      response = nil
      sendDataOver=1
    end
  end
  -- triggers the send() function again once the first chunk of data was sent
  --sck:on("sent", send)
  --send(sck)
end

        
function sendResourceFile(sck, filename)
  if file.exists(filename) then
    if file.open(filename, "r") then
      local line=file.readline()
      local content=""
      while( line )
      do
        line=file.readline()
        if(line~=nil) then
          if(#content+#line>2048) then
            sendData(client,content)
            print("send")
          else
            content=content..line
          end
        else
          if(content~=nil) then sendData(client,content) end
        end
      end
      file.close()
      return content
    end
  end
end

--sendResourceFile(nil,'test.html')

--sendData(client,"")

fd = file.open("test.html", "r")
if fd then
  local rd
  rd=fd:read(15)
  while (rd)
  do
    print(rd)
    rd=fd:read(15)
  end
  fd:close()
  fd = nil
end










