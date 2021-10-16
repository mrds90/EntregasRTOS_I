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

#include "keys.h"
#include "whackamole.h"
#include "random.h"

/* private macros */

#define WAM_GAMEPLAY_TIMEOUT        20000   //gameplay time
#define WAM_MOLE_SHOW_MAX_TIME      6000
#define WAM_MOLE_OUTSIDE_MAX_TIME   2000
#define WAM_MOLE_SHOW_MIN_TIME      1000
#define WAM_MOLE_OUTSIDE_MIN_TIME   500

#define MISS_SCORE									-10
#define NO_MOLE_SCORE								-20


/* private types */



typedef struct
{
    gpioMap_t           led;                //led asociado al mole
    //otros recursos para cada mole
} mole_t;





/* private prototypes */
void mole_service_logic( void* pvParameters );
void whackamole_service_logic( void* pvParameters );

/* global objects  */







/**
   @brief init game

 */
void whackamole_init()
{
    BaseType_t res;

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
int32_t whackamole_points_success( TickType_t tiempo_afuera,TickType_t tiempo_reaccion_usuario )
{
    return ( WAM_MOLE_OUTSIDE_MAX_TIME*WAM_MOLE_OUTSIDE_MAX_TIME ) /( tiempo_afuera*tiempo_reaccion_usuario );
}

/**
   @brief devuelve el puntaje por haber perdido al mole

   @return uint32_t
 */
int32_t whackamole_points_miss()
{
    return MISS_SCORE;
}

/**
   @brief devuelve el puntaje por haber martillado cuando no habia mole

   @return uint32_t
 */
uint32_t whackamole_points_no_mole()
{
    return NO_MOLE_SCORE;
}

/**
   @brief servicio principal del juego

   @param pvParameters
 */
void whackamole_service_logic( void* pvParameters )
{
    BaseType_t res;
    bool game_alive;

    // wam_event_t evnt;

    int32_t puntos;
    uint32_t hits;
    uint32_t miss;


    TickType_t init_time;

    while( 1 )
    {
        /* inicio de juego ??? */
        puntos=0;

        /* randomizo (se usa el tick count del sistema)*/
        random_seed_freertos();

        while( true == game_alive )
        {

        }
    }
}

/**
   @brief servicio instanciado de cada mole

   @param pvParameters
 */
void mole_service_logic( void* pvParameters )
{
    mole_t* mole = ( mole_t* ) pvParameters;
    TickType_t tiempo_aparicion;
    TickType_t tiempo_afuera;

    while( 1 )
    {
        /* preparo el turno */
        tiempo_aparicion = random( WAM_MOLE_SHOW_MIN_TIME, WAM_MOLE_SHOW_MAX_TIME );
        tiempo_afuera    = random( WAM_MOLE_OUTSIDE_MIN_TIME, WAM_MOLE_OUTSIDE_MAX_TIME );


        /* el mole, sale del agujero */
        gpioWrite( mole->led, ON );




        /* el mole, se esconde en agujero */
        gpioWrite( mole->led, OFF );
    }
}

