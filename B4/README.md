# B4

## Descripción

Partiendo del ejercicio B.3 implemente un sistemade 4 tareas:
Tarea A - Prioridad IDLE + 4 - LED asociado LEDB
Tarea B - Prioridad IDLE + 2 - LED asociado LED1
Tarea C - Prioridad IDLE + 2 - LED asociado LED2
Tarea D - Prioridad IDLE + 1 - LED asociado LED3
Valide que la secuencia es la siguiente.

![alt text](https://github.com/mrds90/EntregasRTOS_I/blob/main/B4/Doc/GraficoTemporal.jpeg?raw=true)


Las tareas B y C DEBEN tener un código fuente equivalente(salvando la parte en donde se accede al LED).
Ahora, configure en freertosconfig.h:
```
#defineconfigUSE_TIME_SLICING     0
```
¿Qué sucedió?

Proponga una manera de contrarrestar el efecto sin tocar la configuración mencionada (no utilice la Suspend/Resume para solucionarlo)
### Respuesta

El comportamiento no se valida. Aunque en primera medida parece que si, el tiempo que estan encedidos los LEDs 1 y 2 es el doble (1seg). Esto es lógico porque el delay blockeante (500 ms) de las dos tareas suceden en simultaneao.

Cuando se realiza el cambio del `#defineconfigUSE_TIME_SLICING  0` la secuencia de ejecución es

![alt text](https://github.com/mrds90/EntregasRTOS_I/blob/main/B4/Doc/ComportamientoSinSlicing.png?raw=true)

para resolverlo sin utilizar Suspend/Resume se llama de forma forzada al scheduler con `taskYIELD()`. El comportamiento finalmente es igual al que tenía el sistema con:

```
#defineconfigUSE_TIME_SLICING     1
```
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
