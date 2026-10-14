# Monitoreo de temperatura e iluminación con ESP32, MQTT, Node-RED, InfluxDB y Grafana

Sistema de monitoreo ambiental que mide temperatura, humedad e iluminación con una placa ESP32 , transmite los datos por MQTT, los valida y clasifica en Node-RED, los almacena con marca de tiempo en InfluxDB y los presenta en un dashboard de Grafana.

## Contenido

1. [Funcionamiento](#1-funcionamiento)
2. [Contenido del repositorio](#2-contenido-del-repositorio)
3. [Requisitos](#3-requisitos)
4. [Instalación](#4-instalación)
5. [Configuración](#5-configuración)
6. [Arranque](#6-arranque)
7. [Pruebas](#7-pruebas)
8. [Fallas comunes y resolución](#8-fallas-comunes-y-resolución)
9. [Seguridad y manejo de credenciales](#9-seguridad-y-manejo-de-credenciales)
10. [Limitaciones](#10-limitaciones)

---

## 1. Funcionamiento

![Arquitectura](docs/img/arquitectura.png)

El recorrido de cada dato comprende seis etapas:

1. El **ESP32** lee temperatura, humedad y luz cada 2 segundos y arma un mensaje en formato JSON.
2. El mensaje se **publica por MQTT** en un broker público (HiveMQ), porque Wokwi se ejecuta en internet y no alcanza un servidor local.
3. Un **relay de Node-RED** copia cada mensaje al **Mosquitto local**.
4. **Node-RED** valida el dato, lo clasifica como normal, advertencia o crítico, y genera una alerta cuando el estado cambia.
5. **InfluxDB** almacena las lecturas, las alertas y el estado del monitoreo con su fecha y hora.
6. **Grafana** presenta valores actuales, tendencias, alertas e indicadores de desempeño.

| Componente | Herramienta | Versión | Puerto |
|---|---|---|---|
| Sensores y placa | DHT22, LDR y ESP32 en Wokwi | — | — |
| Broker público | HiveMQ (`broker.hivemq.com`) | — | 1883 |
| Broker local | Mosquitto | 2.1.2 | 1883 |
| Procesamiento | Node-RED sobre Node.js | 5.x y 24.14.1 | 1880 |
| Base de datos | InfluxDB | 2.9.1 | 8086 |
| Dashboard | Grafana | 13.2.3 | 3000 |


---

## 2. Contenido del repositorio

| Carpeta o archivo | Contenido |
|---|---|
| `esp32/` | Programa del ESP32, librerías y circuito de Wokwi |
| `mosquitto/local.conf` | Configuración del broker local |
| `nodered/flujo_monitoreo.json` | Flujo principal de Node-RED |
| `nodered/flujo_relay.json` | Flujo que copia los mensajes del broker público al local |
| `nodered/exportado/` | Exportación directa de Node-RED, como copia de referencia |
| `influxdb/consultas/` | Consultas Flux de verificación e indicadores |
| `influxdb/escritura_prueba.cmd` | Escritura de un dato de prueba mediante la API de InfluxDB |
| `grafana/dashboard_sala_servidores.json` | Dashboard para importar |
| `grafana/exportado/` | Exportación directa de Grafana, como copia de referencia |
| `grafana/provisioning/` | Ejemplo opcional de fuente de datos por aprovisionamiento |
| `tests/publicar_invalidos.cmd` | Publicación de tres mensajes inválidos de prueba |
| `.env.example` | Variables de ejemplo, sin valores reales |
| `docs/img/` | Diagramas utilizados en este documento |

Para la instalación del sistema se debe utilizar los archivos de las carpetas `nodered/` y `grafana/`, que corresponden a la versión limpia para desplegar. Las carpetas `exportado/` conservan mis exportaciones que genero cada programa y sirve solo como ejemplo.

---

## 3. Requisitos

| Programa | Versión | Función | Descarga |
|---|---|---|---|
| Windows | 10 u 11 de 64 bits | Sistema operativo | — |
| Mosquitto | 2.1.2 | Broker MQTT local | https://mosquitto.org/download/ |
| InfluxDB | 2.9.1 | Almacenamiento de series de tiempo | https://download.influxdata.com/influxdb/releases/influxdb2-2.9.1-windows_amd64.zip |
| Node.js | 24.14.1 (mínimo 22.9) | Entorno de ejecución de Node-RED | https://nodejs.org/ |
| Node-RED | 5.x | Procesamiento de los datos | Instalación con `npm` |
| node-red-contrib-influxdb | 0.7.0 | Conexión de Node-RED con InfluxDB | Gestor de paletas de Node-RED |
| Grafana | 13.2.3 | Dashboard | https://grafana.com/grafana/download?platform=windows |
| Git | Reciente | Clonación del repositorio | https://git-scm.com/ |

Librerías del ESP32 (`esp32/libraries.txt`)

El sistema requiere acceso a internet y los puertos locales 1883, 8086, 1880 y 3000 disponibles.

---

## 4. Instalación

### 4.1 Obtención del proyecto

```
git clone https://github.com/<usuario>/<repositorio>.git
cd <repositorio>
```

### 4.2 Mosquitto

Mosquitto se instala con el instalador de Windows. Si queda registrado como servicio, se detiene desde CMD con permisos de administrador:

```
net stop mosquitto
```

### 4.3 InfluxDB

El zip se descomprime y la carpeta se renombra:

```
mkdir C:\InfluxData
Expand-Archive "$env:USERPROFILE\Downloads\influxdb2-2.9.1-windows_amd64.zip" -DestinationPath C:\InfluxData
ren "C:\InfluxData\influxdb2-2.9.1-windows-amd64" influxdb
```

El servidor se inicia con:

```
cd C:\InfluxData\influxdb
.\influxd
```

En http://localhost:8086 se completa la configuración inicial con los siguientes datos:

| Dato | Valor |
|---|---|
| Organización | `DataCenterAndino` |
| Bucket | `sala_servidores` |

Después se crean dos tokens en Load Data, API Tokens, Generate API Token, Custom API Token. El valor completo de cada token solo se muestra en el momento de su creación.

| Nombre | Permiso sobre `sala_servidores` |
|---|---|
| `node_red_escritura` | Write |
| `grafana_lectura` | Read |

### 4.4 Node-RED

```
npm install -g --unsafe-perm node-red
node-red
```

Con Node-RED disponible en http://localhost:1880, el nodo de InfluxDB se instala desde el menú ☰, Manage palette, Install, buscando `node-red-contrib-influxdb`.

### 4.5 Grafana

El zip se descomprime en `C:\Grafana` y el servidor se inicia desde la carpeta `bin`:

```
cd C:\Grafana\grafana-13.2.3\bin
.\grafana.exe server
```

La interfaz está disponible en http://localhost:3000 con el usuario inicial `admin`; la contraseña inicial se reemplaza en el primer acceso.

### 4.6 Wokwi

1. Se crea un proyecto con una placa ESP32.
2. El programa se reemplaza por el contenido de `esp32/esp32_sala_servidores.ino`.
3. Los archivos `diagram.json` y `libraries.txt` del proyecto se reemplazan por los de la carpeta `esp32/`.
4. Al iniciar la simulación, el monitor serie muestra `conectado` y `Publicado OK`.

---

## 5. Configuración

### 5.1 Mosquitto

Contenido de `mosquitto/local.conf`:

```
listener 1883
allow_anonymous true

log_type error
log_type warning
log_type notice
log_type information
connection_messages true
```

| Parámetro | Valor | Función |
|---|---|---|
| `listener` | `1883` | Puerto de recepción de mensajes |
| `allow_anonymous` | `true` | Acepta conexiones sin usuario (entorno de pruebas) |
| `log_type` | varios | Niveles de registro en consola |
| `connection_messages` | `true` | Registro de conexiones y desconexiones |


### 5.2 Tópicos MQTT

Todos los tópicos tienen el prefijo `datacenter_andinoSAC/sala_servidores/`.

| Tópico | Publica | Contenido |
|---|---|---|
| `…/lecturas` | ESP32 | Temperatura, humedad, luz, `seq` y hora |
| `…/estado` | ESP32 y broker | `online` u `offline` (retenido) |
| `…/error` | ESP32 | Motivo del fallo del sensor |
| `…/alertas` | Node-RED | Alertas generadas |

### 5.3 Estructura de los datos

Mensaje publicado por el ESP32:

```json
{"dispositivo":"esp32-sala01","seq":120,"ts_ms":1791750000000,"temperatura":24.00,"humedad":52.0,"ldr_raw":975,"luz_porcentaje":23}
```

| Campo | Descripción |
|---|---|
| `dispositivo` | Nombre de la placa |
| `seq` | Número de secuencia; permite detectar mensajes perdidos |
| `ts_ms` | Hora de la medición en milisegundos |
| `temperatura` y `humedad` | Valores del DHT22 |
| `ldr_raw` y `luz_porcentaje` | Luz cruda (0 a 4095) y luz relativa en porcentaje |

Datos almacenados en el bucket `sala_servidores`:

| Medición | Contenido |
|---|---|
| `lecturas_sala` | Lecturas válidas con su estado |
| `alertas` | Alertas y eventos, con tipo y severidad |
| `estado_monitoreo` | Placa en línea, sin datos o desconectada |
| `calidad_datos` | Mensajes perdidos y reinicios de la placa |
| `lecturas_invalidas` | Lecturas rechazadas y su motivo |

### 5.4 Node-RED

1. Se importa `nodered/flujo_monitoreo.json` desde el menú ☰, Import.
2. Con Wokwi, se importa también `nodered/flujo_relay.json`.
3. En el nodo `InfluxDB (escritura en lote)`, la configuración `InfluxDB local v2` recibe el token `node_red_escritura`.
4. Se ejecuta Deploy. Los nodos MQTT muestran el estado `connected`.

Las alertas también se registran en `C:/proyecto_iot/logs/alertas.log`; la ruta se modifica en el nodo `alertas.log`.

Los valores configurables se encuentran en el nodo `Parametros del sistema` (pestaña Setup) y se aplican con Deploy.

| Parámetro | Valor | Función |
|---|---|---|
| `umbral_advertencia` | 28 | Temperatura desde la que hay advertencia |
| `umbral_critico` | 30 | Temperatura desde la que el estado es crítico |
| `luz_oscuro_max` | 15 | Porcentaje máximo del estado oscuro |
| `luz_tenue_max` | 50 | Porcentaje máximo del estado tenue; por encima, iluminado |
| `ventana_inicio_h` y `ventana_fin_h` | 8 y 18 | Horario de mantenimiento |
| `recordatorio_critico_s` | 60 | Segundos entre recordatorios mientras el estado sigue crítico |

Clasificación de cada lectura:

| Variable | Estado | Condición |
|---|---|---|
| Temperatura | normal | Menos de 28 °C |
| Temperatura | advertencia | De 28 °C a menos de 30 °C |
| Temperatura | crítico | 30 °C o más |
| Luz | oscuro | 15 % o menos |
| Luz | tenue | De 16 % a 50 % |
| Luz | iluminado | Más de 50 % |

Se rechazan las lecturas que no tienen formato JSON, las que contienen valores no numéricos y las que salen de rango. Las lecturas rechazadas se almacenan en `lecturas_invalidas`.

Las alertas de temperatura se generan solo cuando cambia el estado; mientras el estado sigue crítico, se repite un recordatorio cada 60 segundos. Además, un vigilante genera el evento `sin_datos` tras 10 segundos sin lecturas, y el mensaje de último deseo del ESP32 genera `dispositivo_offline` cuando la placa se desconecta.

### 5.5 Grafana

Fuente de datos (Connections, Data sources, Add data source, InfluxDB):

| Campo | Valor |
|---|---|
| Query language | Flux |
| URL | `http://127.0.0.1:8086` |
| Organization | `DataCenterAndino` |
| Token | `grafana_lectura` |
| Default Bucket | `sala_servidores` |
| Min time interval | `2s` |

Con Save & test, Grafana confirma la conexión.

El dashboard se importa desde Dashboards, New, Import, con el archivo `grafana/dashboard_sala_servidores.json` y la fuente InfluxDB como fuente de datos. Se actualiza cada 5 segundos.

### 5.6 ESP32

| Constante | Valor | Descripción |
|---|---|---|
| `DHT_PIN` | 4 | Pin del DHT22 |
| `LDR_PIN` | 34 | Pin del sensor de luz |
| `LDR_INVERTIDO` | `true` | Corrige el sensor simulado, que entrega mayor valor con menor luz |
| `MQTT_SERVER` | `broker.hivemq.com` | Broker de publicación |
| `DEVICE_ID` | `esp32-sala01` | Nombre de la placa |
| `INTERVALO_MS` | 2000 | Periodo de medición |

Conexiones del circuito:

| Componente | Pin del sensor | Pin del ESP32 |
|---|---|---|
| DHT22 | VCC | 3V3 |
| DHT22 | SDA | GPIO 4 |
| DHT22 | GND | GND |
| Resistencia de 10 kΩ | Entre SDA y 3V3 | — |
| Módulo LDR | VCC | 3V3 |
| Módulo LDR | GND | GND |
| Módulo LDR | AO | GPIO 34 |

El broker de publicación se define en el programa del ESP32 y en el nodo `Broker publico (HiveMQ)` del relay; ambos valores coinciden.

### 5.7 Variables de ejemplo

El archivo `.env.example` lista los valores que utiliza el proyecto. Una copia con el nombre `.env`, excluida del repositorio, contiene los valores reales. Los scripts `.cmd` leen las variables de la sesión de CMD:

```
set INFLUX_URL=http://127.0.0.1:8086
set INFLUX_ORG=DataCenterAndino
set INFLUX_BUCKET=sala_servidores
set INFLUX_TOKEN_NODERED=<token de escritura>
```

---

## 6. Arranque

Cada programa se ejecuta en una ventana independiente, en el siguiente orden desde la carpeta repositorio:

| Paso | Programa | Comando | Dirección |
|---|---|---|---|
| 1 | Mosquitto | `"C:\Program Files\Mosquitto\mosquitto.exe" -c mosquitto\local.conf -v`  | `127.0.0.1:1883` |
| 2 | InfluxDB | `cd C:\InfluxData\influxdb` y `.\influxd` | http://localhost:8086 |
| 3 | Node-RED | `node-red` | http://localhost:1880 |
| 4 | Grafana | `cd C:\Grafana\grafana-13.2.3\bin` y `.\grafana.exe server` | http://localhost:3000 |
| 5 | Wokwi | Inicio de la simulación con la pestaña visible | https://wokwi.com |

Verificación de los servicios activos:

```
netstat -ano | findstr "LISTENING" | findstr ":1883 :8086 :1880 :3000"
```

---

## 7. Pruebas

Ventanas de observación (CMD en `C:\Program Files\Mosquitto`):

| Ventana | Comando | Muestra |
|---|---|---|
| Publicado | `mosquitto_sub -h broker.hivemq.com -t "datacenter_andinoSAC/sala_servidores/lecturas" -v` | Mensajes publicados por el ESP32 |
| Recibido | `mosquitto_sub -h localhost -t "datacenter_andinoSAC/#" -v` | Mensajes recibidos en el Mosquitto local |

| ID | Prueba | Procedimiento | Resultado esperado |
|---|---|---|---|
| P1 | Lecturas normales | DHT22 en 24 °C durante 5 minutos | Estado normal y sin alertas |
| P2 | Advertencia | Temperatura de 28 °C y luego de 29.9 °C | Un único aviso de advertencia |
| P3 | Temperatura crítica | 31 °C durante 75 s y regreso a 24 °C | Alerta crítica, recordatorio al minuto y aviso de normalización |
| P4 | Iluminación | LDR en 1, 100, 500 y 5000 lux | Cambian el porcentaje y el estado de luz |
| P5 | Interrupción | Detención de Wokwi durante 45 s y reinicio | Eventos de sin datos, desconectado y datos recuperados |
| P6 | Lecturas inválidas | DHT22 en −40 °C y ejecución de `tests\publicar_invalidos.cmd` | Registros en `lecturas_invalidas` |
| P7 | Persistencia | Reinicio de InfluxDB | Los datos anteriores se conservan |
| P8 | Publicados y recibidos | Comparación durante 5 minutos | Los conteos de publicados, recibidos y almacenados coinciden |

Las consultas de `influxdb/consultas/` se ejecutan en InfluxDB, Data Explorer, Script Editor.

| Archivo | Muestra |
|---|---|
| `q1_ultimas_lecturas.flux` | Últimas lecturas con su hora |
| `q2_alertas.flux` | Alertas y eventos |
| `q3_estado_monitoreo.flux` y `q3_calidad_datos.flux` | Estado de conexión y mensajes perdidos |
| `q4_lecturas_invalidas.flux` | Lecturas rechazadas |
| `q5_conteo_por_seq.flux` | Cantidad de lecturas entre dos valores de `seq` (se editan los números) |
| `q6_conteo_ventana_fija.flux` | Cantidad de lecturas en una ventana de tiempo (se editan las horas en UTC) |
| `ind1_…`, `ind2_…`, `ind3_…` | Indicadores de mensajes recibidos, latencia y tiempo de detección |

En la prueba P8, los mensajes publicados se calculan como `seq` final − `seq` inicial + 1; los recibidos se cuentan con `mosquitto_sub` y los almacenados con `q5_conteo_por_seq.flux`.

Indicadores del dashboard:

| Indicador | Medición |
|---|---|
| IND-1 | Porcentaje de mensajes recibidos |
| IND-2 | Latencia aparente entre el ESP32 y Node-RED |
| IND-3 | Tiempo de detección de una temperatura crítica |


---

## 8. Fallas comunes y resolución

| Síntoma | Causa | Resolución |
|---|---|---|
| Mosquitto muestra `Starting in local only mode` | No se cargó el archivo de configuración | Inicio con `-c mosquitto\local.conf`, verificando que el archivo no tenga extensión `.txt` |
| Nodos MQTT de Node-RED en amarillo | Mosquitto local no está en ejecución | Se inicia Mosquitto |
| El ESP32 muestra `rc=3` o `rc=-4` | El broker público rechaza la conexión por saturación | Se reintenta o se cambia de broker en el programa y en el relay |
| Wokwi publica y Node-RED no recibe | Los brokers del ESP32 y del relay difieren, o el relay no está importado | Se igualan los brokers y se importa el relay |
| Error 401 al escribir en InfluxDB | Token incorrecto | Se vuelve a ingresar `node_red_escritura` |
| `unable to open boltdb: timeout` | Hay otra instancia de InfluxDB abierta | `taskkill /IM influxd.exe /F` y nuevo inicio |
| `grafana-server.exe` no existe | Grafana 13 utiliza un único ejecutable | Uso de `.\grafana.exe server` |
| Grafana muestra No data | Rango de tiempo sin datos, o token o lenguaje incorrectos | Se ajusta el rango y se verifica la fuente (Flux y token de lectura) |
| Mayor luz entrega menor porcentaje | El sensor simulado opera de forma inversa | `LDR_INVERTIDO = true` |
| El token aparece como puntos en Node-RED | Node-RED no muestra las credenciales guardadas | Se ingresa nuevamente el valor |

---

## 9. Seguridad y manejo de credenciales

- Este repositorio no contiene tokens, contraseñas ni archivos `.env`; el `.gitignore` los excluye.
- Node-RED almacena las credenciales por separado en `flows_cred.json`, archivo excluido del repositorio.
- Cada token tiene un único permiso: Node-RED escribe y Grafana solo lee.
- La configuración corresponde a un entorno de pruebas: Mosquitto acepta conexiones sin usuario y los mensajes viajan sin cifrar. Un entorno de producción requiere usuario, contraseña y cifrado (TLS).

---

## 10. Limitaciones

| Limitación | Efecto |
|---|---|
| Broker público y Mosquitto sin autenticación ni TLS | Los mensajes viajan sin cifrar |
| Publicación con QoS 0 y sesión limpia | Los mensajes perdidos durante un corte no se recuperan |
| Sin almacenamiento intermedio ante la caída de InfluxDB | Las lecturas de ese lapso no se almacenan |
| Reloj simulado de Wokwi | La latencia y el tiempo de detección incluyen el atraso del simulador |
| Vigilancia solo de temperaturas altas | No existe alerta de temperatura baja |
| Iluminación como porcentaje relativo | No corresponde a lux calibrados |
| Alertas en InfluxDB, archivo y MQTT | No hay notificaciones por correo ni mensajería |
