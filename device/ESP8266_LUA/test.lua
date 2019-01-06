i=0
sendbuf={}
sendover=0
tmr.create():alarm(2150, tmr.ALARM_AUTO, function()
table.insert(sendbuf,#sendbuf+1,{
filename='filename',
res='scktest'
})
end)

tmrtest = tmr.create()
tmrtest:register(1000, tmr.ALARM_SEMI, function()
if #sendbuf~=0 and sendover==1 then
    sendover=0
    tab=table.remove(sendbuf,1)
    tab.res:sendResourceFile(tab.filename)
end
tmrtest:start()
end)
tmrtest:start()