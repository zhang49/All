local str="{\"type\":\"1\",\"data\":\"#1\"}"
configfilename="config.txt"
id="abcedfg"
ssid="zy_em"
psw="123456"

function getConfig()
     if file.exists(configfilename) then
        if file.open(configfilename, "r") then
            line=file.readline()
            content=""
            cfg={}
            i=0
            while( line )
            do
                k=string.sub(line, 0,string.find(line, "=")-1)
                v=string.sub(line, string.find(line, "=.*")+1)
                v=v.sub(v, 0,string.len(v)-1)
                print(v)
                cfg[k]=v
                line=file.readline()
            end
            file.close()
        end
        return cfg
    end
    return nil;
end

if file.open(configfilename, "w+") then
    file.writeline("id="..id);
    file.writeline("ssid="..ssid);
    file.writeline("pwd="..psw);
    file.close()
    cfg=getConfig()
    print(cfg['ssid'])
else
    print("open error")
end