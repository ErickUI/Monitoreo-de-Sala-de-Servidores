@echo off
set "MOSQ=C:\Program Files\Mosquitto"
set "TOPIC=datacenter_andinoSAC/sala_servidores/lecturas"
"%MOSQ%\mosquitto_pub.exe" -h localhost -t "%TOPIC%" -m "{\"dispositivo\":\"prueba_manual\",\"seq\":1,\"temperatura\":150,\"humedad\":40,\"ldr_raw\":1000,\"luz_porcentaje\":24}"
"%MOSQ%\mosquitto_pub.exe" -h localhost -t "%TOPIC%" -m "esto no es json"
"%MOSQ%\mosquitto_pub.exe" -h localhost -t "%TOPIC%" -m "{\"dispositivo\":\"prueba_manual\",\"temperatura\":25,\"humedad\":40,\"ldr_raw\":1000,\"luz_porcentaje\":24}"
