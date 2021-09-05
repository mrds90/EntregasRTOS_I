# A4

## Descripción

Utilizando el ejemplofirmware_v3\examples\c\sapi\uart\rx_interruptimplemente un programa con una tarea:
- Configura el driver de sapi para que puedan utilizarseinterrupciones para recibir datos via UART.
- Esperara un paquete que comience con '>' y finalicecon '<'. Al recibirlo:
  - Una tarea deberá encender un led testigo (lo hace inmediatamente y por 1 segundo).
  - Pasados 1 segundo de la recepción, deberá enviarse(usando printf) el texto "Recibido".


## Dependences
### CIAA Software
El software de la CIAA es necesario para compilar el programa
El software de la CIAA puede descargarse e instalarse siguiento el siguiente tutorial: [CIAA SOFTWARE TUTORIAL](https://github.com/epernia/software/)
### Firmware v3
El [firmware_v3](https://github.com/epernia/firmware_v3/) es el firmware que el proyecto CIAA desarrollo para utilizar el Hardware.
De este firmware es utilizada la sapi y seos_pont.

Para mas información sobre como usar el firmware_v3 puede leer [English Readme](https://github.com/epernia/firmware_v3/blob/master/documentation/firmware/readme/readme-en.md) o [Readme en Español](https://github.com/epernia/firmware_v3/blob/master/documentation/firmware/readme/readme-es.md)

## Descarga
Abrir una terminal en la carpeta donde carga los proyectos dentro de firmware v3 y escribir
```
git clone https://github.com/mrds90/PmC_Actividad3.git
```
## Compilacion
### Corregir error en configuración de interrupciones
En el archivo "libs/lpc_open/lpc_startup/src/vendor_interrupt.c"

existe la condicion para el presprocesador:
```
#if __CORTEX_M == 0U
```
en la linea 6, reemplace esta sentencia por:

```
#if 0
```
para que las interrupciones por defectos queden definidas como WEAK

### Compilar
* Abrir la terminal del CIAA launcher e ir a la ruta del firmware v3.
* Seleccionar el programa con el comando
```
make select_program
```
* Elegir la carpeta del repositorio descargado (PmC_Actividad3)
* Compilar el programa el programa en la misma terminal con:
```
make all
```
##  Descargar programa en edu_cia_nxp

* En la misma terminal y con la edu_ciaa_nxp conectada escribir

```
make download
```

## Help

Puede no tener seleccionado la placa correcta. Para corregir esto escriba
```
make select_board
```
y seleccione la edu_ciaa_nxp

Siempre debe estar en la ruta raiz de la carpeta clonada "firmware_v3" con la terminal de CIAA Launcher si desea usar los comandos.

## Autor

Marcos Dominguez
[@mrds90](https://github.com/mrds90)
