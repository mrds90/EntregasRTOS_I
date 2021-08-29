
/*=============================================================================
 * Copyright (c) 2021, Marcos Dominguez <mrds0690@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: 2021/08/29
 * Version: v1.0
 *===========================================================================*/
/*==================[inlcusiones]============================================*/

// #include "../../sapi/sapi_v0.6.2/base/inc/sapi.h"            // <= sAPI header
#include "sapi.h"
#include "seos_pont.h"        // <= dispatcher and task management header
/*==================[definiciones y macros]==================================*/
#define BUFFER_SIZE 255
#define DELAY_TIME 1000


typedef enum
{
    STATE_OFF,
    STATE_ON
} led_state_t;


typedef enum {
   RX_IDLE,
   RX_START,
   RX_RECEIVE_COMPLETE,

   RX_STATE_QTY
} rx_state_t;

/*==================[definiciones de datos internos]=========================*/
static uint32_t led_task_index;
/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/
static void task_led(void *param);
static void task_msg(void *param);
/*==================[funcion principal]======================================*/


void onRx( void *noUsado )
{  static rx_state_t rx_state = RX_IDLE;
   static char buffer[BUFFER_SIZE];
   static uint8_t buffer_index = 0;
   char c = uartRxRead( UART_USB );
   switch( rx_state ) {
      case RX_IDLE:
         if( c == '>' ) {
            rx_state = RX_START;
         }
         break;
      case RX_START:
         if( c != '<' ) {
            buffer[buffer_index++] = c;
         } 
         else {
            rx_state = RX_RECEIVE_COMPLETE;
         }
         break;
      case RX_RECEIVE_COMPLETE:
         led_task_index = schedulerAddTask(task_led, // funcion de tarea a agregar
                        (void *)LED1,         // parametro de la tarea
                        0,                 // offset de ejecucion en ticks
                        DELAY_TIME      // periodicidad de ejecucion en ticks
                        );
         schedulerAddTask(task_msg, // funcion de tarea a agregar
                        0,
                        DELAY_TIME,
                        0
                        );
         rx_state = RX_IDLE;
         buffer_index = 0;
         break;
   }
}

int main(void)
{
   /* Inicializar la placa */
   boardConfig();

   /* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
   uartConfig(UART_USB, 115200);     
   // Seteo un callback al evento de recepcion y habilito su interrupcion
   uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
   // Habilito todas las interrupciones de UART_USB
   uartInterrupt(UART_USB, true);
   schedulerStart(1);
   while(TRUE) {
      schedulerDispatchTasks();
   }
   return 0;
}


static void task_led(void *param)
{   static led_state_t led_state = STATE_OFF;
    if (led_state == STATE_OFF)
    {
        /* toggle del led */
        gpioToggle((gpioMap_t)param);

        /* cambio de estado al led */
        led_state = STATE_ON;
    }
    else if (led_state == STATE_ON)
    {
        /* toggle del led */
        gpioToggle((gpioMap_t)param);

        /* cambio de estado al led */
        led_state = STATE_OFF;

        schedulerDeleteTask(led_task_index);
    }
}


static void task_msg(void *param) {
   printf("Recibido\n");
}
