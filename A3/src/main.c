/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * 					   Marcos Dominguez <mrds0690@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2021/08/29
 * Version: v1.0
 *===========================================================================*/

/*==================[inlcusiones]============================================*/

//#include "cooperative_os_scheduler_03_seos_Pont2014.h"   // <= own header (optional)
#include "sapi.h"             // <= sAPI header
#include "seos_pont.h"        // <= dispatcher and task management header
#include "key_time_counter.h" // <= key time counter header
/*==================[definiciones y macros]==================================*/
typedef enum
{
    STATE_OFF,
    STATE_ON
} led_state_t;

/*==================[definiciones de datos internos]=========================*/
static uint32_t led_task_index[LED3 - LEDB + 1]; // array of task indexes
/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/


/*==================[declaraciones de funciones externas]====================*/
void keys_service_task(void *param); // key task function

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
    // ---------- CONFIGURACIONES ------------------------------
    // Inicializar y configurar la plataforma
    boardConfig();

    /* inicializo el planificador */
    schedulerInit();

    /* inicializo el modulo de tecla */
    FSMKeyTimeCounterInit();

    /* agregamos al planificador una tarea que "consulte" el estado de las teclas,
       de manera periodica */
    gpioMap_t i = 0;
    for (i = TEC1; i <= TEC4; i++)
    {
        schedulerAddTask(keys_service_task, // funcion de tarea a agregar
                         (void *)i,         // parametro de la tarea
                         0,                 // offset de ejecucion en ticks
                         DEBOUNCE_TIME      // periodicidad de ejecucion en ticks
        );
    }
    // FUNCION que inicializa la interrupcion que ejecuta el planificador de
    // tareas con tick cada 1ms.
    schedulerStart(1);

    // ---------- REPETIR POR SIEMPRE --------------------------
    while (TRUE)
    {
        // Se despachan (ejecutan) las tareas marcadas para su ejecucion.
        // Luego se pone el sistema en bajo consumo hasta que ocurra la
        // proxima interrupcion, en este caso la de Tick.
        // Al ocurrir la interrupcion de Tick se ejecutara el planificador
        // que revisa cuales son las tareas a marcar para su ejecucion.
        schedulerDispatchTasks();
    }

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return 0;
}

/*==================[definiciones de funciones internas]=====================*/

void task_led(void *param)
{
    static uint32_t led_state[LED3 - LEDB + 1] = {STATE_OFF}; // array of led states

    if (led_state[(gpioMap_t)param - LEDB] == STATE_OFF)
    {
        /* toggle del led */
        gpioToggle((gpioMap_t)param);

        /* cambio de estado al led */
        led_state[(gpioMap_t)param - LEDB] = STATE_ON;
    }
    else if (led_state[(gpioMap_t)param - LEDB] == STATE_ON)
    {
        /* toggle del led */
        gpioToggle((gpioMap_t)param);

        /* cambio de estado al led */
        led_state[(gpioMap_t)param - LEDB] = STATE_OFF;

        schedulerDeleteTask(led_task_index[(gpioMap_t)param - LEDB]);
    }
}

/**
   @brief Funcion que se ejecuta cada DEBOUNCE_TIME ticks.

   @param param
 */
void keys_service_task(void *param)
{
    tick_t time = FSMKeyTimeCounterUpdate(param);

    if (time)
    {
        /* planifico que la tarea de LED se ejecute en 0 ticks */
        led_task_index[(gpioMap_t)param - TEC1] = schedulerAddTask(task_led,                                   // funcion de tarea a agregar
                                                                   (void *)((gpioMap_t)param + (LEDB - TEC1)), // LEDB - TEC1 es el offset que hay entre las teclas y los leds. De esta forma convertimos una tecla a su led correspondiente.
                                                                   0,                                          // offset -> 0 = "ejecutate inmediatamente"
                                                                   time                                        // periodicidad de ejecucion en ticks
        );
    }
}
/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
