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

Res = {
	_sck = nil,
	_mType = nil,
	_status = nil,
	_path=nil
}
--Req = {
--	_source = nil,
--	_path = nil,
--	_method = nil,
--	_GET = nil,
--}

function Res:new(sck)
	local o = {}
	setmetatable(o, self)--inherit self(HttpServer)
    self.__index = self
    o._sck = sck
    return o
end

function Res:type(type)
	self._type = type
end

function Res:status(status)
	self._status = status
end

function Res:sendResourceFile(filename)
	if file.exists(filename .. '.gz') then
		filename = filename .. '.gz'
	elseif not file.exists(filename) then
		self:status(404)
		if filename == '404.html' then
			self:send(404)
		else
			self:sendFile('404.html')
		end
		return
	end
	self._status = self._status or 200 
	self._mType=guessType(filename)
    local header = 'HTTP/1.1 ' .. self._status .. '\r\n'
    header= header .. 'Cache-Control: max-age=3592000\r\n'
    header = header .. 'Content-Type: ' .. self._mType .. '\r\n'
    if string.sub(filename, -3) == '.gz' then
        header = header .. 'Content-Encoding: gzip\r\n'
    end
	header = header .. '\r\n'-------improtant
    print('-----response header-----\r\n'..header..'\r\n---------')
    print('* Sending ', filename)
    local pos = 0
	local readlen = 0
    local function doSend()
        file.open(filename, 'r')
        if file.seek('set', pos) == nil then
            self:close()
			readlen=pos-1450+readlen
            print(filename..'Finished. total len:'..readlen)
        else
            local buf = file.read(1460)
            pos = pos + 1460
			readlen=#buf
            self._sck:send(buf)
        end
        file.close()
    end
    self._sck:on('sent', doSend)
    self._sck:send(header)
end

function parseRequestHeader(req,res)
	local _, _, method, path, vars = string.find(req.source, "([A-Z]+) (.+)?(.+) HTTP")
    
    print('-----request source-----\r\n'..req.source..'\r\n---------')
	if (method == nil) then
		_, _, method, path = string.find(req.source, "([A-Z]+) (.+) HTTP")
	end
	if (method == 'POST') then
          local i=string.find(req.source,'\r\n\r\n')
          if i~=nil then
            vars=string.sub(req.source,i+4,#req.source)
          else
            print('find double \r\n is nil')
          end
    end
	req.GET=nil
	if ( vars ~=nil ) then
    print('-----request vars-----\r\n'..vars..'\r\n---------')
	  local index=1,rear,k,v
	  while true
	  do
		index=string.find (vars, "=")
		if index==nil then break;
		else
		  k=string.sub(vars,0,index-1)
		  rear=string.find (vars, "&", index)
		  if rear==nil then rear=#vars+1 end
		  v=string.sub(vars,index+1,rear-1)
		  vars=string.sub(vars,rear+1)
		  req.GET[k]=v
		  print('"'..k..'"'..' : "'..v..'"')
		end
	  end
	end
	req.method=method
	req.path=path
    print('-----request path-----\r\n'..req.path..'\r\n---------')
	
	return true
end

function staticFile(req, res)
	local filename = ''
	print('staticFile get path:'..req.path)
	if req.path == '/' then
		filename = 'index.html'
		res:sendResourceFile(filename)
		return true
	else
		filename=string.sub(req.path,#req.path-string.find(string.reverse(req.path),"/")+2,#req.path)
		if string.find(filename,'[\.]') then
			print("request resourse file:"..filename)
			res:sendResourceFile(filename)
			return true
		end
		--filename = string.gsub(string.sub(req.path, 2), '/', '_')
	end
	print("staticFile over")
end

--------------------
-- HttpServer
--------------------
httpServer = {
	_srv = nil,
	_mids = {{
		url = '.*',
		cb = parseRequestHeader
	},{
		url = '.*',
		cb = staticFile
	}
	}
}

function httpServer:use(_url, _cb)
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
		conn:on('receive', function(sck, msg)	
		
		local ip = sck:getpeer()
			if( buffer.ip == nil )then 
				buffer.ip = msg 
			else
				buffer.ip = buffer.ip .. msg 
			end
			local req = { source = msg, path = '', method='', GET={},ip = sck:getpeer() }
			parseRequestHeader(req,nil)
			if( req.method == 'post' or req.method == 'POST' ) then
				if( req.GET == nil )then return nil	end
			end
			
			local res = Res:new(sck)-- new Res with sck
				for i = 3, #self._mids do
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











