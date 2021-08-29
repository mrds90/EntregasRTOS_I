# A3

Empleando dos tareas, medir el tiempo de pulsación de 4 botones utilizando un algoritmo anti-rebote.Luego destellar el led correspondiente durante el tiempo medido.
Ayuda: Se puede consultar el contador de ticks para obtener el tiempo del sistema (enticks) al inicio y al fin del mismo. En este caso hay que prever que esta variable puede desbordar.

## Descripción

La aplicación enciente los LED1, LED2, LED3 y LED4 el mismo tiempo que se presionó la tecla correspondiente (TEC1, TEC2, TEC3 y TEC4) justo al soltarla.

## Dependences
### CIAA Software
El software de la CIAA es necesario para compilar el programa
El software de la CIAA puede descargarse e instalarse siguiento el siguiente tutorial: [CIAA SOFTWARE TUTORIAL](https://github.com/epernia/software/)
### Firmware v3
El [firmware_v3](https://github.com/epernia/firmware_v3/) es el firmware que el proyecto CIAA desarrollo para utilizar el Hardware.
De este firmware es utilizada la sapi y seos_pont.

Para mas información sobre como usar el firmware_v3 puede leer [English Readme](https://github.com/epernia/firmware_v3/blob/master/documentation/firmware/readme/readme-en.md) o [Readme en Español](https://github.com/epernia/firmware_v3/blob/master/documentation/firmware/readme/readme-es.md)

## Arquitectura

main es la capa de mayor nivel de abtracción y posee dos capas mas compuesta por los archivos

* key_time_counter
* button


### Main
El la capa de la aplicación. Esta utiliza los servicios de la sapi, seos_pont y de key_time_counter para administrar las tareas en los tiempos adecuados.

### Key_time_counter

Esta capa cuenta el tiempo desde que baja tecla consultada hasta que sube la misma.

### Button
Es una Maquina de estados finitos para verificar el estado del boton consultado (con algoritmo de antirebote). 
Cada tecla puede estar en alguno de los siguientes estados:
* UP
* FALLING
* DOWN
* RISING

En el estado FALLING y RISING la MEF controla por el efecto de rebote. La maquina genera un evento si se confirma la transición de UP a DOWN o viceversa.

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
