import requests
import re
import time
import hashlib
respose=requests.get('http://www.xiaohuar.com/v/')
print(respose.status_code)
print(respose.content)
print(respose.text)
urls=re.findall(r'class="items".*?href="(.*?)"',respose.text,re.S)
url=urls[5]
result=requests.get(url)
mp4_url=re.findall(r'id="media".*?src="(.*?)"',result.text,re.S)[0]

video=requests.get(mp4_url)

with open('D:\\a.mp4','wb') as f:
    f.write(video.content)