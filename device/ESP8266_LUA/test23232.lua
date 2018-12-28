request='GET /login?3232=3232&23%5B323%5D=332&232%5B323%5D=323  HTTP/1.1'
local _,_,method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")

if (method == nil) then
  _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
end

print("method is : "..method)
print("path is : "..path)

vars=string.gsub(vars,'%%5B','[')
vars=string.gsub(vars,'%%5D',']')
vars=string.gsub(vars,'&',' ')

print("var is : "..vars)
local _GET = {}
if (vars ~= nil) then
  for k, v in string.gmatch(vars, "(%[0-9]+)=(%[0-9]+)/r") do
    print(k.."_"..v)
    --_GET[k] = v
  end
end
        