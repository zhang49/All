local str="{\"type\":\"1\",\"data\":\"#1\"}"
configfilename="Config.txt"
id="abcedfg"
ssid="Ares"
psw="460204415"
cinfigData={}
--print(t["value"])

function tableToString(root)
   local buf='{' 
   index=0
    --if type(v) == "table" then return end
    for k,v in pairs(root) do
       if index ~= 0 then buf=buf.."," end
       index=index+1
       buf=buf..'"'..k..'":'
       if type(v) == "table" then 
           buf=buf..'{' 
           index_t=0
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

function readConfigFromFile()
    if not file.exists("Config.txt") or file.open("Config.txt") and file.read() ==nil then
        file.open("Config.txt","w+")
        local table={}
        table["ap"]={}
        table["station"]={}
        table["tonken"]="tonken"
        table["wifimode"]="station"
        table["startmode"]="config"
        table["ap"]["ssid"]="ESP8266_Mode"
        table["ap"]["pwd"]="12345678"
        table["station"]["ssid"]="Ares"
        table["station"]["pwd"]="460204415"
        local wbuf=tableToString(table)
        file.write(wbuf) 
        file.close()
        print("create file!")
    end
        file.close()
        file.open("Config.txt","r")
        content=file.read()
        cinfigData = sjson.decode(content)    
        file.close()   
end
readConfigFromFile()
print(cinfigData["station"]["ssid"])










