configfilename="Config.txt"
cinfigData={}
function tableToString(root)
   local buf='{' 
   index=0
    --if type(v) == "table" then return end
    for k,v in pairs(root) do
       if index ~= 0 then buf=buf.."," end
       local index=index+1
       buf=buf..'"'..k..'":'
       if type(v) == "table" then 
           buf=buf..'{' 
           local index_t=0
            --if type(v) == "table" then return end
            for k,v in pairs(v) do
               if index_t ~= 0 then buf=buf.."," end
               index_t=index_t+1
               buf=buf..'"'..k..'":"'..v..'"' 
            end
            buf=buf..'}'
       else
       buf=buf..'"'..v..'"' 
       end
    end
    buf=buf..'}'
    return buf
end
function sendData(client,content)

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

function startConfigPage()
    srv = net.createServer(net.TCP)
    srv:listen(80, function(conn)
      conn:on("receive", function(client, request)
        local indexpage="index.html"
        local buf = ""
        local retcontent=""
        local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
        print(request)
        if (method == nil) then
          _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
        end
        local _GET = {}
        
        if(path == '/') then
            buf=readfile(indexpage)
        end
        
        if(path == '/favicon.ico') then
            buf=readfile('favicon.ico')
        end
        
        if(path == '/login') then
        
        end
      end)
    conn:on("sent", function(c) c:close() end)
    end)
end

--startConfigPage()
request="/a/login.jpg"
local indexpage="index.html"
local buf = ""
local retcontent=""
local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
if (method == nil) then
  _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
end
path='/jquery-3.2.1.min.js'
local _GET = {}
  local temp=string.sub(path,#path-string.find(string.reverse(path),"/")+2,#path)
  local i=string.find(temp,'[\.]')
  if i~=nil then
    print("request resourse file:"..temp)
    --local type=string.sub(temp,i+1,#temp)
    --local name=string.sub(temp,1,string.find(temp,'[\.]')-1)
    content=sendResourceFile(client,temp)
    --print(content)
  else  
    print("not request resource data.")
    local function sw_command() print("command") end
    local function sw_index() print("index") end
    local function sw_login() print("login") end
    local function sw_root() print("login") end
    local mswitch={
      [""]=sw_index,
      ["command"]=sw_command,
      ["login"]=sw_login,
      ["root"]=sw_root
    }
    local sw=mswitch[temp]
    if sw then sw()
    else print("not find :"..temp)
    end
  end











