/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/


/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "sapi.h"
#include "keys.h"
#include "event.h"


/*=====[Definition & macros of public constants]==============================*/
#define RATE                    1000
#define LED_RATE_TICKS          pdMS_TO_TICKS(RATE)
#define EVENT_QUEUE_SIZE          4
/*=====[Definitions of extern global functions]==============================*/

// Prototipo de funcion de la tarea
void task_led( void* taskParmPtr );
void task_print( void* taskParmPtr );
void keys_service_task( void* taskParmPtr );

/*=====[Definitions of public global variables]==============================*/
QueueHandle_t cola_1;
QueueHandle_t cola_2;
/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
    BaseType_t res;

    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();  // Inicializar y configurar la plataforma

    printf( "Ejercicio F6\n" );

    // Crear tareas en freeRTOS
    res = xTaskCreate (
            task_led,					// Funcion de la tarea a ejecutar
            ( const char * )"task_led",	// Nombre de la tarea como String amigable para el usuario
            configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
            0,							// Parametros de tarea
            tskIDLE_PRIORITY+1,			// Prioridad de la tarea
            0							// Puntero a la tarea creada en el sistema
        );
    
    configASSERT( res == pdPASS );
    
    res = xTaskCreate (
            task_print,					// Funcion de la tarea a ejecutar
            ( const char * )"task_print",	// Nombre de la tarea como String amigable para el usuario
            configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
            0,							// Parametros de tarea
            tskIDLE_PRIORITY+1,			// Prioridad de la tarea
            0							// Puntero a la tarea creada en el sistema
        );

    configASSERT( res == pdPASS );

    cola_1 = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(event_t));
    configASSERT( cola_1 != NULL );
    cola_2 = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(event_t));
    configASSERT( cola_2 != NULL );


    /* inicializo driver de teclas */
    keys_init();

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    /* realizar un assert con "false" es equivalente al while(1) */
    configASSERT( 0 );
    return 0;
}

void task_led( void* taskParmPtr ) {   
    event_t evento;
    while( 1 )
    {
        BaseType_t led_selector = xQueueReceive(cola_1, &evento, LED_RATE_TICKS);
        if(led_selector == pdTRUE && evento.type == EVENT_KEY_PRESSED) {
            gpioToggle( LED1 );
            vTaskDelay(evento.value);
            gpioToggle( LED1 );
            evento.index = LED1;
        }
        else {
            gpioToggle( LED2 );
            vTaskDelay(LED_RATE_TICKS);
            gpioToggle( LED2 );
            evento.index = LED2;
        }
        evento.type = EVENT_LED_BLINK;
        evento.value = 0;
        xQueueSend(cola_2, &evento, 0);
        
    }
}

void task_print( void* taskParmPtr ) {
    event_t evento;
    char str[40];
    while( 1 ) {
        xQueueReceive(cola_2, &evento, portMAX_DELAY);
        if (evento.type == EVENT_LED_BLINK) {
            sprintf( str, "Se encendio el led: %i.\r\n", (evento.index - LED1 + 1) );
            uartWriteString(UART_USB, str);
        }
        else if (evento.type == EVENT_KEY_PRESSED) {
            sprintf( str,"Se presiono la tecla %d, durante %d ms.\r\n", (evento.index - TEC1 + 1), evento.value);
            uartWriteString(UART_USB, str);
        }
    }
}


/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}