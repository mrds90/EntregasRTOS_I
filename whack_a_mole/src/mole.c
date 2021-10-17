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
    TickType_t tiempo_aparicion;
    TickType_t tiempo_afuera;
    BaseType_t press_check;
    while( 1 ) {
        /* preparo el turno */
        
        tiempo_aparicion = random( WAM_MOLE_SHOW_MIN_TIME, WAM_MOLE_SHOW_MAX_TIME );
        tiempo_afuera    = random( WAM_MOLE_OUTSIDE_MIN_TIME, WAM_MOLE_OUTSIDE_MAX_TIME );
        press_check = xSemaphoreTake( this_mole->semaphore, pdMS_TO_TICKS(tiempo_aparicion));
        if (press_check == pdTRUE) {
            /*golpeo cuando no hay mole*/
            print_info.points = whackamole_points_no_mole();
            print_info.event = EVENT_FAIL;
        }
        else {
            tiempo_aparicion = tickRead(); //tiempo actual (reciclo variable)
            /* muestro el mole */
            gpioWrite(this_mole->led, ON);
            press_check = xSemaphoreTake(this_mole->semaphore, pdMS_TO_TICKS(tiempo_afuera));
            if (press_check == pdTRUE) {
                /*golpeo cuando hay mole*/
                print_info.points = whackamole_points_success(tiempo_afuera,  this_mole->last_time - tiempo_aparicion);
                print_info.event = EVENT_HIT;
            }
            else {
                /* no golpeo cuando hay mole */
                print_info.points = whackamole_points_miss();
                print_info.event = EVENT_MISS;
            }
            /* el mole, se vuelve a ocultar */
            gpioWrite( this_mole->led, OFF );
        }
    xQueueSend(this_mole->report_queue,&print_info,portMAX_DELAY);
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
