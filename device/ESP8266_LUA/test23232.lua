request='GET /login?3232=3232&23%5B323%5D=332&232%5B323%5D=323  HTTP/1.1'
local _,_,method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
if (method == nil) then
  _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
end
print("method is : "..method)
print("path is : "..path)
vars=string.gsub(vars,'%%5B','[')
vars=string.gsub(vars,'%%5D',']')
vars='token=hgddgh&apssid=360WiFi-1AC8AE&appwd=12345678'
print("var is : "..vars)
index=1
while true
do
  index=string.find (vars, "=")
  if index==nil then break;
  else 
    k=string.sub(vars,0,index-1)
    rear=string.find (vars, "&", index)
    if rear==nil then rear=#vars end
    v=string.sub(vars,index+1,rear-1)
    vars=string.sub(vars,rear+1)
    print("key="..k.." , value="..v)
  end
end
msg='{"type":"request","data":{"per":"cat","number":"1024"}}'
local ok,t = pcall(sjson.decode,msg)
if ok then
print(msg..'\r\n')
else print("error")
uart.write(1,"232")
end

    



