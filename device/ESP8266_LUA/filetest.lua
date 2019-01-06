
function readfile(filepath)
    if file.exists(filepath) then
        fd=file.open(filepath,'r')
        local resData={}
        i=0
        local buf
        if fd then
            buf=fd:read(1460)
            while buf do
            i=i+1
            print(i)
            ok=pcall(table.insert,resData,#resData+1,buf)
            if ok then print('ok')
            else 
            
            return
            print('error')
            end 
            buf=fd:read(1460)
            end
        end
        print(#resData)
    end
end
pos=0
buf=""
readlen=0
filename='jquery-weui.js'
i=0

buf=""
print('strat')
while buf~=nil
do
file.open(filename, 'r')
if file.seek('set', pos) == nil then
    buf=nil
    readlen=pos-1460+readlen
    print(filename..'Finished. total len:'..readlen)
    print('use i:'..i)
else
    buf = file.read(1460)
    pos = pos + 1460
    readlen=#buf
end
file.close()
end




--readfile('jquery-weui.js')




















