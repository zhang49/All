
function readfile(filepath)
    if file.exists(filepath) then
        if file.open(filepath, "r") then
            line=file.readline()
            content=""
            while( line ) 
            do
                print(line)
                content=content .. line
                line=file.readline()
            end
            file.close()
            return content
        end
    end
   return nil; 
end

wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "GamePartment", pwd = "game1234" })
print(wifi.sta.getip())

htmlname="index.html"
content=readfile(htmlname)
print(content)
print(string.format("0x%x",file.fscfg()))





















