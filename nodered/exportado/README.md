# Exportación directa de Node-RED

`flows_exportado_nodered.json` es la exportación tal como la generó Node-RED (menú Export, todos los flujos) desde el entorno donde se ejecutó el sistema. Incluye los flujos principal y del relay.

| Archivo | Uso |
|---|---|
| `../flujo_monitoreo.json` y `../flujo_relay.json` | Versión limpia, sin comentarios en el código, para desplegar |
| `flows_exportado_nodered.json` | Copia de referencia; tiene la misma lógica y propiedades que la versión limpia |

La lógica de las 10 funciones y las propiedades de los nodos son las mismas en ambas versiones. Las diferencias son solo de presentación: identificadores de nodos, comentarios en el código de algunos nodos y un nodo interno `global-config` que registra las versiones de los módulos instalados.

Para instalar el sistema se importan los archivos de la carpeta superior.
