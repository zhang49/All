local pos = 0
filename='jquery-weui.js'
i=0
function test1()
    while true
    do
        file.open(filename, 'r')
        if file.seek('set', pos) == nil then
            print('test1 Finished ', filename)
            file.close()
            break
        else
            local buf = file.read(1400)
            pos = pos + 1400
        end
    end
end
fd=nil
function test2()
    data=""
    pos = 0
    local i=0
    fd=file.open(filename,'r')
        function doSend()
            data=fd:read(1400)
            i=i+1
     
            if data==nil then
                   fd:close()
                   fd=nil
                return 1
            end
            print("i="..i)
        end
        while doSend()~=1 do end
        print('test2 Finished ', filename)
end

test2()







