configfilename="config.txt"
if file.open(configfilename, "w+") then
    file.writeline("id=".."token");
    file.writeline("ssid=".."Ares");
    file.writeline("pwd=".."460204415");
    file.writeline("startmode=".."work");
    file.close()
else
    print("open error")
end