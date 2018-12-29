wifi.setmode(wifi.STATION)
wifi.sta.config({ ssid = "zy_em", pwd = "12345678" })
gpio.mode(1, gpio.OUTPUT)
print(net.TCP)
srv = net.createServer(net.TCP)
srv:listen(80, function(conn)
  conn:on("receive", function(client, request)
    local buf = ""
    local _, _, method, path, vars = string.find(request, "([A-Z]+) (.+)?(.+) HTTP")
    if (method == nil) then
      _, _, method, path = string.find(request, "([A-Z]+) (.+) HTTP")
    end
    local _GET = {}
    if (vars ~= nil) then
      for k, v in string.gmatch(vars, "(%w+)=(%w+)&*") do
        _GET[k] = v
      end
    end
     buf = buf .. "<!DOCTYPE html><html><body><h1>Hello</h1><form src=\"/\"><p><input type=\"submit\" value=\"Save\" /></p><p>delay&nbsp:&nbsp<input name=\"delayvalue\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
     buf = buf .. "<p>delay&nbsp:&nbsp<input name=\"v2\" type=\"number\" value=\"\" />&nbspms</p><p>delay&nbsp:&nbsp<input name=\"v3\" type=\"number\" value=\"\" />&nbspms</p>"
    
    buf = buf .. "</form></body></html>"
   local _on, _off = "", ""
     client:send(buf)
  end)
  conn:on("sent", function(c) c:close() end)
end)
