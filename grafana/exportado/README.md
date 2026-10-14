# Exportación directa de Grafana

`dashboard_exportado_grafana_v2.json` es el dashboard tal como lo exportó Grafana 13.2.3, en el formato nuevo de esa versión (`dashboard.grafana.app/v2`).

| Archivo | Uso |
|---|---|
| `../dashboard_sala_servidores.json` | Versión portable para importar: pide elegir la fuente de datos al importar |
| `dashboard_exportado_grafana_v2.json` | Copia de referencia; referencia la fuente de datos de la instalación original |

Ambos tienen los mismos 20 paneles, las mismas consultas y los mismos umbrales. La exportación directa incluye metadatos internos de Grafana (identificadores y fechas) y apunta a la fuente de datos de la instalación original, por lo que en otra instalación habría que volver a asociarla.

Para instalar el sistema se importa el archivo de la carpeta superior.
