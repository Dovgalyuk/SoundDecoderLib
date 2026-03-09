curl 'http://192.168.4.1/api/firmware/upload' \
  -X 'POST' \
  -H 'User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36' \
  -H 'Content-Type: application/octet-stream' \
  -H 'Referer: http://192.168.4.1/' \
  --data-binary "@build/esp32sounddecoder.bin"
