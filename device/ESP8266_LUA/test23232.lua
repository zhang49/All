function tableToString(root)
   local buf='{' 
   local index=0
    --if type(v) == "table" then return end
    for k,v in pairs(root) do
       if index ~= 0 then buf=buf.."," end
       index=index+1
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



request='GET /login?3232=3232&23%5B323%5D=332&232%5B323%5D=323  HTTP/1.1'
local _,_,method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
if (method == nil) then
  _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
end
print("method is : "..method)
print("path is : "..path)
vars='type=command&data%5Btime_tick%5D=232_s2&data%5Bfruit%5D=pear'
vars=string.gsub(vars,'%%5B','[')
vars=string.gsub(vars,'%%5D','')

print("var is : "..vars)
index=1
_GET={}
print('-----------\r\n')

for k, v in string.gmatch(vars, "([^=]*)=([^&]*)&*") do
print('key:'..k..'  value:'..v)
    if (string.find(k, '[\[]')) then 
        for f,b in string.gmatch(k,'(.-)[\[](.*)') do
        if _GET[f]==nil then _GET[f]={} end
        _GET[f][b]=v
        end
    else
        _GET[k]=v
    end
end
          
    

    ---print('front str is:'..string.sub(k,0,string.find(k, '[\[]')-1))
     --   print('back str is:'..string.sub(k,string.find(k, '[\[]')+1,-1))
    



print('-----------\r\n')
print(#_GET)
print('-----------\r\n')

print(tableToString(_GET))

print('-----------\r\n')
function sjsontest()
    msg='{"type":"request","data":{"per":"cat","number":"1024"}}'
    local ok,t = pcall(sjson.decode,msg)
    if ok then
    print(msg..'\r\n')
    else print("error")
    uart.write(1,"232")
    end
end
    



