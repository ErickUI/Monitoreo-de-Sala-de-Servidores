@echo off
curl.exe -i -X POST "%INFLUX_URL%/api/v2/write?org=%INFLUX_ORG%&bucket=%INFLUX_BUCKET%&precision=s" -H "Authorization: Token %INFLUX_TOKEN_NODERED%" --data-raw "prueba,dispositivo=manual temperatura=25.5"
