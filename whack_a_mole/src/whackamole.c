/**
BSD 3-Clause License

Copyright (c) 2021, Franco Bucafusco, Martin Menendez
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdarg.h>
#include <stdbool.h>


#include "sapi.h"


#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "../inc/keys.h"
#include "whackamole.h"
#include "random.h"

/* private macros */

#define WAM_GAMEPLAY_TIMEOUT        15000   //gameplay time
#define WAM_MOLE_SHOW_MAX_TIME      6000
#define WAM_MOLE_OUTSIDE_MAX_TIME   2000
#define WAM_MOLE_SHOW_MIN_TIME      1000
#define WAM_MOLE_OUTSIDE_MIN_TIME   500

#define MISS_SCORE					-10
#define NO_MOLE_SCORE				-20

#define MOLE_KEY(x)                 (x) + TEC1
#define MOLE_LED(x)                 (x) + LEDB

/* private types */

typedef enum {
    WAM_STATE_INIT,
    WAM_STATE_GAMEPLAY,
    WAM_STATE_END
} wam_state_t;

typedef struct {   
    wam_state_t        state;
    int16_t            points;
} wack_a_mole_t;
typedef struct {
    gpioMap_t           key;
    gpioMap_t           led;                //led asociado al mole
    xSemaphoreHandle    semaphore;          //semaforo para controlar el acceso al mole
    TickType_t          last_time;          //ultima vez que se golpeo el mole
    //otros recursos para cada mole
} mole_t;

typedef enum {
    WAM_MOLE_1,
    WAM_MOLE_2,
    WAM_MOLE_3,
    WAM_MOLE_4,

    WAM_MOLE_QTY
} mole_index_t;



/* private prototypes */
static void MOLE_ServiceLogic( void * pvParameters );
static void whackamole_service_logic( uintptr_t pvParameters );
static void WHACKAMOLE_ISRKeyPressed (t_key_isr_signal* event_data , uintptr_t context);
/* global objects  */

static mole_t mole[WAM_MOLE_QTY];
static wack_a_mole_t wam;





/**
   @brief init game

 */
void WHACKAMOLE_Init()
{
    BaseType_t res[WAM_MOLE_QTY];

    for (mole_index_t i = 0; i < WAM_MOLE_QTY; i++) {
        mole[i].key = MOLE_KEY(i);
        mole[i].led = MOLE_LED(i);
        mole[i].semaphore = xSemaphoreCreateBinary();
        configASSERT(mole[i].semaphore);
        mole[i].last_time = 0;
        res[i] = xTaskCreate(
            MOLE_ServiceLogic,
            (const char *)"WAM",
            configMINIMAL_STACK_SIZE,
            (void *) &mole[i],
            tskIDLE_PRIORITY + 1,
            NULL
        );
        configASSERT(res[i] == pdPASS);
    }

    KEYS_LoadPressHandler( WHACKAMOLE_ISRKeyPressed, (uintptr_t) &mole[0] );
    /* creacion de tareas */
    
    


    // configASSERT( ... );

    /* creacion de objetos */





    // configASSERT( ... );
}


/**
   @brief devuelve el puntaje de haber martillado al mole

   @param tiempo_afuera             tiempo q hubiera estado el mole esperando
   @param tiempo_reaccion_usuario   tiempo de reaccion del usuario en martillar
   @return uint32_t
 */
__STATIC_FORCEINLINE int32_t whackamole_points_success( TickType_t tiempo_afuera,TickType_t tiempo_reaccion_usuario )
{
    return ( WAM_MOLE_OUTSIDE_MAX_TIME*WAM_MOLE_OUTSIDE_MAX_TIME ) /( tiempo_afuera*tiempo_reaccion_usuario );
}

/**
   @brief devuelve el puntaje por haber perdido al mole

   @return uint32_t
 */
__STATIC_FORCEINLINE int32_t whackamole_points_miss()
{
    return MISS_SCORE;
}

/**
   @brief devuelve el puntaje por haber martillado cuando no habia mole

   @return uint32_t
 */
__STATIC_FORCEINLINE uint32_t whackamole_points_no_mole()
{
    return NO_MOLE_SCORE;
}

/**
   @brief servicio principal del juego

   @param pvParameters
 */
void whackamole_service_logic( uintptr_t pvParameters ) {
    
}

/**
   @brief servicio instanciado de cada mole

   @param pvParameters
 */
void MOLE_ServiceLogic( void* pvParameters ) {   
    mole_t this_mole = *(( mole_t*)pvParameters);
    TickType_t tiempo_aparicion;
    TickType_t tiempo_afuera;
    BaseType_t press_check;
    while( 1 )
    {
        /* preparo el turno */
        tiempo_aparicion = random( WAM_MOLE_SHOW_MIN_TIME, WAM_MOLE_SHOW_MAX_TIME );
        tiempo_afuera    = random( WAM_MOLE_OUTSIDE_MIN_TIME, WAM_MOLE_OUTSIDE_MAX_TIME );
        press_check = xSemaphoreTake( this_mole.semaphore, pdMS_TO_TICKS(tiempo_aparicion));
        if (press_check == pdTRUE) {
            /*golpeo cuando no hay mole*/
            wam.points += whackamole_points_no_mole();
        }
        else {
            tiempo_aparicion = tickRead(); //tiempo actual (reciclo variable)
            /* muestro el mole */
            gpioWrite(this_mole.led, ON);
            press_check = xSemaphoreTake(this_mole.semaphore, pdMS_TO_TICKS(tiempo_afuera));
            if (press_check == pdTRUE) {
                /*golpeo cuando hay mole*/
                wam.points += whackamole_points_success(tiempo_afuera,  this_mole.last_time - tiempo_aparicion);
            }
            else {
                /* no golpeo cuando hay mole */
                wam.points += whackamole_points_miss();
            }
            /* el mole, se vuelve a ocultar */
            gpioWrite( this_mole.led, OFF );
        }
    }
}

void WHACKAMOLE_ISRKeyPressed (t_key_isr_signal* event_data , uintptr_t context) {
    mole_t * moles = (mole_t*)context;
    
    switch (event_data->tecla) {
        case TEC1_INDEX:
            moles += WAM_MOLE_1;
            moles->last_time = tickRead();
            xSemaphoreGive(mole[WAM_MOLE_1].semaphore);
            break;
        case TEC2_INDEX:
            moles += WAM_MOLE_2;
            moles->last_time = tickRead();
            xSemaphoreGive(mole[WAM_MOLE_2].semaphore);
            break;
        case TEC3_INDEX:
            moles += WAM_MOLE_3;
            moles->last_time = tickRead();
            xSemaphoreGive(mole[WAM_MOLE_3].semaphore);
            break;
        case TEC4_INDEX:
            moles += WAM_MOLE_4;
            moles->last_time = tickRead();
            xSemaphoreGive(mole[WAM_MOLE_4].semaphore);
            break;
    }
    
    
}
