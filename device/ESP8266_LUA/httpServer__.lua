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


function Res:send(body)
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
		if buf== nil or buf == '' then 
			self:close()
		else
			self._sck:send(string.sub(buf, 1, 1460))
			buf = string.sub(buf, 1460)
		end
	end
	self._sck:on('sent', doSend)
	doSend()
end

function Res:sendResourceFile(filename)
	self._mType=guessType(filename)
	if file.exists(filename .. '.gz') then
		print('--'..filename..' has gz ....')
		filename = filename .. '.gz'
	elseif not file.exists(filename) then
		self:status(404)
		if filename == '404.html' then
			self:send(404)
		else
			self:sendResourceFile('404.html')
		end
		return
	end
	self._status = self._status or 200 
    local header = 'HTTP/1.1 ' .. self._status .. '\r\n'
    header= header .. 'Cache-Control: max-age=3592000\r\n' -- cache
    header = header .. 'Content-Type: ' .. self._mType .. '\r\n'
    if string.sub(filename, -3) == '.gz' then
        header = header .. 'Content-Encoding: gzip\r\n'
    end
	header = header .. '\r\n'	-------improtant
    --print('-----response header-----\r\n'..header..'\r\n---------')
    --print('* Sending ', filename)
    local pos = 0
	local readlen = 0	
    local function doSend()
		file.open(filename, 'r')
		if file.seek('set', pos) == nil then
			readlen=pos-300+readlen
			self:close()
			--print(filename..'Finished. total len:'..readlen)
		else
			buf = file.read(300)
			self._sck:send(buf)
			pos = pos + 300
			readlen=#buf
		end
		file.close()
    end
    self._sck:on('sent', doSend)
    self._sck:send(header)
end

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

function staticFile(req, res)
	local filename = ''
	--print('staticFile get path:'..req.path)
	if req.path == '/' then
		filename = 'index.html'
		res:sendResourceFile(filename)
	else
		filename=string.sub(req.path,#req.path-string.find(string.reverse(req.path),"/")+2,#req.path)
		if string.find(filename,'[\.]') then
			print("request resourse file:"..filename)
			res:sendResourceFile(filename)
		end
		--filename = string.gsub(string.sub(req.path, 2), '/', '_')
	end
	--print("staticFile over")
	return true
end

--------------------
-- HttpServer
--------------------
httpServer = {
	_srv = nil,
	_mids = {{
		url = '.*',
		cb = staticFile
	}
	}
}

function httpServer:onRecv(_url, _cb)
	table.insert(self._mids, #self._mids+1, {
		url = _url,
		cb = _cb
	})
end

function Res:close()
	self._sck:on('sent', function() end) -- release closures context
	self._sck:on('receive', function() end)
	self._sck:close()
	self._sck = nil
end

function httpServer:close()
	self._srv:close()
	self._srv = nil
end


function httpServer:listen(port)
	self._srv = net.createServer(net.TCP)
	self._srv:listen(port, function(conn)
		local buffer = {}
		buffer.ip=nil
		conn:on('receive', function(sck, msg)	
		--local ip = sck:getpeer()
		if( buffer.ip == nil )then 
			buffer.ip = msg 
		else
			buffer.ip = buffer.ip .. msg 
			--print("Merge buffer .")
		end

		
		local i=string.find(buffer.ip,'\r\n\r\n')
		if i then
			if string.find(string.sub(buffer.ip,1,6),'POST /')==1 then
				local _,_,len=string.find(buffer.ip,'Content[\-]Length: ([0-9]+)')
				--print(len)
				if len~=nil then
					local length=tonumber(len)
					i=i+4
					if #buffer.ip>=length+i-1 then
						reqData=string.sub(buffer.ip,0,length+i-1)
						--print("reqData is:"..reqData)
						buffer.ip=string.sub(buffer.ip,length+i,-1)
					else return nil
					end
				end
			else
				--print('is GET------------------')
				_,_,reqData,nextstr=string.find(buffer.ip,'(.*)\r\n\r\n(.*)')
				buffer.ip=nextstr
				--print("reqData is:"..reqData)
			end
		else
			return nil
		end
		
--[[		local _,_,reqData,nextstr=string.find(buffer.ip,'(.-\r\n\r\n.*)(GET /.*)')
		if nextstr == nil then
			_,_,reqData,nextstr=string.find(buffer.ip,'(.-\r\n\r\n.*)(POST /.*)')
		end
		if nextstr == nil then
			_,_,reqData,nextstr=string.find(buffer.ip,'(.-\r\n\r\n.*)')
			nextstr = ""
		end
		if reqData == nil then 
			return nil
		end
		if string.find(string.sub(buffer.ip,1,6),'POST /')==1 then
			--POST method must has data
			if string.find(buffer.ip,'\r\n\r\n([^%s]+)') == nil then	
				return nil
			end
		end
		if nextstr~=nil then
			buffer.ip=nextstr
			--print('catch next header:'..buffer.ip)
		end
--]]
		
		local req = { source = reqData, path = nil, method = nil, GET = {},ip = sck:getpeer() }
		if not parseRequestHeader(req,nil) then
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



























