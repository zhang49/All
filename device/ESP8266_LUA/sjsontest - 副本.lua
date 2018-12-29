function pacalltest()
    ok, json=pcall(sjson.encode,'{"ds:"ds"}')
    if not ok then print("error") end
            
    if ok then
      print(json)
    else
      print("failed to encode!")
    end
end

local buf='123456789abcdefgh'
local data=''
local response={}
for i=1,50,1 do
    data=data..buf
end
print(data)
reponse = {}
local sublen=100
while true 
do
    --a problem in print(string.sub(data,i,j)) , get data NULL
    ret=string.sub(data,0,sublen)
    response[#response + 1]=ret
    if #ret ~= sublen then break end
    data=string.sub(data,sublen+1,-1)
end
print(#response)
while #response > 0 
do
    print(table.remove(response, 1))
end




