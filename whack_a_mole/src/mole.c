/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/10/17
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdarg.h>
#include <stdbool.h>


#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "random.h"
#include "../inc/whackamole.h"
#include "../inc/mole.h"
/*=====[Definition macros of private constants]==============================*/
#define WAM_MOLE_SHOW_MAX_TIME      6000
#define WAM_MOLE_OUTSIDE_MAX_TIME   2000
#define WAM_MOLE_SHOW_MIN_TIME      1000
#define WAM_MOLE_OUTSIDE_MIN_TIME   500

#define MISS_SCORE					-10
#define NO_MOLE_SCORE				-20
/*=====[Definitions of private functions]====================================*/

/**
   @brief devuelve el puntaje de haber martillado al mole

   @param tiempo_afuera             tiempo q hubiera estado el mole esperando
   @param tiempo_reaccion_usuario   tiempo de reaccion del usuario en martillar
   @return uint32_t
 */
__STATIC_FORCEINLINE int32_t whackamole_points_success( TickType_t tiempo_afuera,TickType_t tiempo_reaccion_usuario );

/**
   @brief devuelve el puntaje por haber perdido al mole

   @return uint32_t
 */
__STATIC_FORCEINLINE int32_t whackamole_points_miss(void);
/**
   @brief devuelve el puntaje por haber martillado cuando no habia mole

   @return uint32_t
 */
__STATIC_FORCEINLINE uint32_t whackamole_points_no_mole(void);

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Implementation of public functions]==================================*/
/**
   @brief servicio instanciado de cada mole

   @param pvParameters
 */
void MOLE_ServiceLogic( void* pvParameters ) {   
    print_info_t print_info = {0,0};
    mole_t *this_mole = (( mole_t*)pvParameters);
    TickType_t tiempo_aparicion = 0;
    TickType_t tiempo_afuera;
    TickType_t tiempo_auxiliar;
    BaseType_t press_check;
    while( 1 ) {
        /* preparo el turno */
        if (tiempo_aparicion == 0) {
            tiempo_aparicion = random( WAM_MOLE_SHOW_MIN_TIME, WAM_MOLE_SHOW_MAX_TIME ); // tiempo de aparicion aleatorio
        }
        tiempo_auxiliar = tickRead(); // tiempo actual
        press_check = xSemaphoreTake( this_mole->semaphore, pdMS_TO_TICKS(tiempo_aparicion)); // veo si el usuario apreta el boton sin mole activo (el handler de presionado de la tecla da este semaforo)
        if (press_check == pdTRUE) { // si el usuario apreto antes de que aparezca el mole
            /*golpeo cuando no hay mole*/
            tiempo_aparicion -= tickRead() - tiempo_auxiliar; // Estrategia para conservar el tiempo de aparicion constante
            print_info.points = whackamole_points_no_mole();
            print_info.event = EVENT_FAIL;
        }
        else {
            tiempo_aparicion = 0; // reseteo el tiempo de aparicion
            tiempo_afuera    = random( WAM_MOLE_OUTSIDE_MIN_TIME, WAM_MOLE_OUTSIDE_MAX_TIME ); // tiempo de afuera aleatorio
            tiempo_auxiliar = tickRead(); //tiempo actual 
            /* muestro el mole */
            gpioWrite(this_mole->led, ON); // enciendo el led del mole
            press_check = xSemaphoreTake(this_mole->semaphore, pdMS_TO_TICKS(tiempo_afuera)); // espero a que el usuario apriete (el handler de presionado de la tecla da este semaforo)
            if (press_check == pdTRUE) { // si el usuario acerto al mole
                /*golpeo cuando hay mole*/
                print_info.points = whackamole_points_success(tiempo_afuera,  this_mole->last_time - tiempo_auxiliar); // calculo los puntos
                print_info.event = EVENT_HIT; // marco el evento
            }
            else {
                /* no golpeo cuando hay mole */
                print_info.points = whackamole_points_miss(); // calculo los puntos
                print_info.event = EVENT_MISS; // marco el evento
            }
            /* el mole, se vuelve a ocultar */
            gpioWrite( this_mole->led, OFF ); // apago el led del mole
        }
    xQueueSend(this_mole->report_queue,&print_info,portMAX_DELAY); // envio el reporte al report_queue que lo procesa la tarea principal en el estado WAM_STATE_GAMEPLAY
    }
}

void MOLE_Downs(mole_t* moles , uint8_t n_moles) {
    for(int i = 0; i < n_moles; i++) {
        gpioWrite(moles[i].led, OFF);
    }
}
/*=====[Implementations of private functions]================================*/

__STATIC_FORCEINLINE int32_t whackamole_points_success( TickType_t tiempo_afuera,TickType_t tiempo_reaccion_usuario ) {
    return ( WAM_MOLE_OUTSIDE_MAX_TIME*WAM_MOLE_OUTSIDE_MAX_TIME ) /( tiempo_afuera*tiempo_reaccion_usuario );
}

__STATIC_FORCEINLINE int32_t whackamole_points_miss(void) {
    return MISS_SCORE;
}

__STATIC_FORCEINLINE uint32_t whackamole_points_no_mole(void) {
    return NO_MOLE_SCORE;
}
/*=====[Implementations of interrupt functions]==============================*/
