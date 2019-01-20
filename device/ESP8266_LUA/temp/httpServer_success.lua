sendFileBuf = {}
fileSendFlag = 0
fd = nil
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
--------------------
-- Response
--------------------
Res = {
  _sck = nil,
  _mType = nil,
  _status = nil
}
function Res:new(sck)
  local o = {}
  setmetatable(o, self)--inherit self
  self.__index = self
  o._sck = sck
  return o
end

function Res:mType(mType)
  self._mType = mType
end

function Res:status(status)
  self._status = status
end
--------------------
-- Request
--------------------
--Req = {
--	_source = nil,
--	_path = nil,
--	_method = nil,
--	_GET = nil,
--}

function closeSck(sck)
  sck:close()
  sck:on('sent', function() end) -- release closures context
  sck:on('receive', function() end)
  sck = nil
end
function Res:close()
  closeSck(Res._sck)
end
function closeSck_file(sck)
  --sck琚叧闂悗濡傝嫢缁х画�彂閫佷細瀵艰嚧鎶ラ敊閲嶅惎锛屾病鏈夋帴鍙ｈ幏鍙杝ck鐘舵��
  --褰搒ck琚叧闂椂,鑾峰彇鍒扮殑port, ip涓簄il
  local _,ip = sck:getpeer()
  if ip ~= nil then
    closeSck(sck)
  end
  if fd ~= nil then fd:close() fd=nil end
end

function httpSend(sck,msg,flag)
  local res = Res:new(sck)-- new Res with sck
  res:send(msg,flag)
  collectgarbage()
end

function Res:send(body,flag)
  self._status = self._status or 200
  self._mType = self._mType or 'text/html'
  local buf = 'HTTP/1.1 ' .. self._status .. '\r\n'
    .. 'Content-Type: ' .. self._mType .. '\r\n'
    .. 'Content-Length:' .. string.len(body) .. '\r\n'
  if self._redirectUrl ~= nil then
    buf = buf .. 'Location: ' .. self._redirectUrl .. '\r\n'
  end
  buf = buf .. '\r\n' .. body
  local function doSend()
    if buf == '' then
      if flag == nil then
        local _, ip = self._sck:getpeer()
		if ip ~= nil then
		  self._sck:close()
		end
      end
    else
      self._sck:send(string.sub(buf, 1, 1460))
      buf = string.sub(buf, 1460)
    end
  end
  --sck琚叧闂悗濡傝嫢缁х画�彂閫佷細瀵艰嚧鎶ラ敊閲嶅惎锛屾病鏈夋帴鍙ｈ幏鍙杝ck鐘舵��
  --褰搒ck琚叧闂椂,鑾峰彇鍒扮殑port, ip涓簄il
  local _,ip = self._sck:getpeer()
  if ip ~= nil then
    self._sck:on('sent', doSend)
    doSend()
  end
end

function sendFile_real(sck, filename)
  local mType, status, header
  mType = guessType(filename)
  status = 200
  if file.exists(filename .. '.gz') then
    filename = filename .. '.gz'
  elseif not file.exists(filename) then
    status = 404
    filename = '404.html'
    if file.exists('404.html.gz') then filename = '404.html.gz' end
  end
  header = 'HTTP/1.1 ' .. status .. '\r\n'
  header= header .. 'Cache-Control: max-age=3592000\r\n' -- cache
  header = header .. 'Content-Type: ' .. mType .. '\r\n'
  if string.sub(filename, -3) == '.gz' then
    header = header .. 'Content-Encoding: gzip\r\n'
  end
  header = header .. '\r\n'	-------improtant
  --print('-----response header-----\r\n'..header..'\r\n---------')
  --print('* Sending ', filename)
  fd=file.open(filename, 'r')
  local function doSend()
    local buf = fd.read(1460)
    if buf == nil then
      table.remove(sendFileBuf, 1)
      closeSck_file(sck)
	  if #sendFileBuf <= 6 then sck:unhold() end
      fileSendFlag = 0 --鍏抽棴fd 鍚庡啀鍊兼爣蹇�浣�
    else
      sck:send(buf)
      fileSendFlag = 1
    end
  end
  --sck琚叧闂悗濡傝嫢缁х画�彂閫佷細瀵艰嚧鎶ラ敊閲嶅惎锛屾病鏈夋帴鍙ｈ幏鍙杝ck鐘舵��
  --褰搒ck琚叧闂椂,鑾峰彇鍒扮殑port, ip涓簄il
  local _,ip = sck:getpeer()
  if ip ~= nil then
    sck:on('sent', doSend)
    sck:send(header)
  else
    table.remove(sendFileBuf, 1)
    fileSendFlag = 0
  end
end
--淇濆瓨socket, filename鍒皊endFileBuf涓紝瀹氭椂鍣ㄦ鏌ュ彂閫�
function Res:sendFile(filename)
  local port,ip=self._sck:getpeer()
  local _,_,mark=string.find(string.reverse(ip),'(%d+)')
  mark = mark .. port
  table.insert(sendFileBuf, #sendFileBuf+1, { m = mark, s = self._sck, f = filename})
end
--姣�10ms 妫�鏌ワ紝鍙戦�乻endFileBuf涓殑filname锛岃秴鏃�2s鍏抽棴socket
--sendFileTmr = tmr.create()
tmr.create():alarm(10,tmr.ALARM_AUTO,function()
  if fileSendFlag ~=0 then fileSendFlag=fileSendFlag+1 end
  if fileSendFlag == 0 and #sendFileBuf > 0 then
    fileSendFlag=1
    local tb=sendFileBuf[1]
    sendFile_real(tb.s ,tb.f)
  elseif fileSendFlag>200 then
    if #sendFileBuf > 0 then
      local tb = table.remove(sendFileBuf, 1)
      closeSck_file(tb.s)
    end
    fileSendFlag=0
  end
end)

function parseRequestHeader(req,res)
  local _, _, method, path, vars = string.find(req.source, "([A-Z]+) (.+)?(.+) HTTP")    
  --print('-----request source-----\r\n'..req.source..'\r\n---------')
  if (method == nil) then	--doesn't has vars
    _, _, method, path = string.find(req.source, "([A-Z]+) (.+) HTTP")
  end
  if (method == 'POST') then	--POST Data is behind the tail
    _,_,vars=string.find(req.source,'\r\n\r\n(.*)')
    --print('vars is:'..vars)
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

function isRequestFile(req, res)
  local filename
  filename=string.sub(req.path,#req.path-string.find(string.reverse(req.path),"/")+2,#req.path)
  if string.find(filename,'[\.]') then
    print("request resourse file:"..filename)
    res:sendFile(filename)
  end
  --filename = string.gsub(string.sub(req.path, 2), '/', '_')
  return true
end

--------------------
-- HttpServer
--------------------
httpServer = {
  _srv = nil,
  _mids = {{
    url = '.*',
    cb = isRequestFile
  }
  }
}

function httpServer:onRecv(_url, _cb)
  table.insert(self._mids, #self._mids+1, {
    url = _url,
    cb = _cb
  })
end

function httpServer:close()
  self._srv:close()
  self._srv = nil
end

function httpServer:listen(port)
  self._srv = net.createServer(net.TCP)
  self._srv:listen(port, function(conn)
    local buffer = {}
    conn:on('disconnection', function(sck)
      --local port,ip=sck:getpeer()
      --local _,_,mark=string.find(string.reverse(ip),'(%d+)')
      --mark = mark .. port
      sck:on('sent', function() end) -- release closures context
      sck:on('receive', function() end)
      sck = nil
      --don't do this  sck:close
    end)
    conn:on('receive', function(sck, msg)
	  if #sendFileBuf >= 6 then sck:hold() end
      local port,ip=sck:getpeer()
      local _,_,cid=string.find(string.reverse(ip),'(%d+)')
      cid = cid .. port
      if( buffer.cid == nil )then 
        buffer.cid = msg 
      else
        buffer.cid = buffer.cid .. msg 
        --print("Merge buffer .")
      end
      local i, reqData, nextstr, len
      i=string.find(buffer.cid,'\r\n\r\n')
      if i then
        if string.find(string.sub(buffer.cid,1,6),'POST /')==1 then
          _,_,len=string.find(buffer.cid,'Content[\-]Length: ([0-9]+)')
          --print(len)
          if len~=nil then
            local length=tonumber(len)
            i=i+4
            if #buffer.cid>=length+i-1 then
              reqData=string.sub(buffer.cid,0,length+i-1)
              --print("reqData is:"..reqData)
              buffer.cid=string.sub(buffer.cid,length+i,-1)
              collectgarbage()
            else return nil
            end
          end
        else
          --print('is GET------------------')
          _,_,reqData,nextstr=string.find(buffer.cid,'(.*)\r\n\r\n(.*)')
          buffer.cid=nextstr
          --print("reqData is:"..reqData)
        end
      else
        collectgarbage()
        return nil
      end	
      --[[		local _,_,reqData,nextstr=string.find(buffer.cid,'(.-\r\n\r\n.*)(GET /.*)')
      if nextstr == nil then
        _,_,reqData,nextstr=string.find(buffer.cid,'(.-\r\n\r\n.*)(POST /.*)')
      end
      if nextstr == nil then
        _,_,reqData,nextstr=string.find(buffer.cid,'(.-\r\n\r\n.*)')
        nextstr = ""
      end
      if reqData == nil then 
        return nil
      end
      if string.find(string.sub(buffer.cid,1,6),'POST /')==1 then
        --POST method must has data
        if string.find(buffer.cid,'\r\n\r\n([^%s]+)') == nil then	
          return nil
        end
      end
      if nextstr~=nil then
        buffer.cid=nextstr
        --print('catch next header:'..buffer.cid)
      end
      --]]
      local req = { source = reqData, path = nil, method = nil, GET = {},ip = sck:getpeer() }
      if not parseRequestHeader(req,nil) then
        collectgarbage()
        return nil
      end
      local res = Res:new(sck)-- new Res with sck
      for i = 1, #self._mids do
        --print("mids url:"..self._mids[i].url)
        if string.find(req.path, '^' .. self._mids[i].url .. '$') then
          if not self._mids[i].cb(req, res) then
            break
          end
        end
      end
      collectgarbage()
    end)
  end)
end
