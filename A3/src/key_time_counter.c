/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/08/29
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "key_time_counter.h"
#include "button.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definition of private data type]==============================*/
typedef enum {
    COUNTER_IDLE,
    COUNTER_RUNNING,
    
    COUNTER_STATE_QTY,
} counter_states_t;
    
typedef struct {
   counter_states_t state;
   uint32_t init_time;
} time_counter_t;

/*=====[Definitions of private global variables]=============================*/
static time_counter_t time_counter[TEC4-TEC1+1];
/*=====[Delaration of private functions]==================================*/

/*=====[Implematation of public functions]==================================*/
void FSMKeyTimeCounterInit(void) {
   for(int i=0; i<TEC4-TEC1+1; i++) {
      time_counter[i].state = COUNTER_IDLE;
      FSMButtonInit(i+TEC1);
   }
}
tick_t FSMKeyTimeCounterUpdate(void *param) {
   gpioMap_t key = (gpioMap_t) param;
   tick_t time = 0;
   switch (time_counter[key-TEC1].state) {
      case COUNTER_IDLE:
         if(CheckFallState(key)) {
            time_counter[key-TEC1].state = COUNTER_RUNNING;
            time_counter[key-TEC1].init_time = tickRead();
         }
         break;
      case COUNTER_RUNNING:
         if(CheckRaiseState(key)) {
            time_counter[key-TEC1].state = COUNTER_IDLE;
            time = tickRead() - time_counter[key-TEC1].init_time;
         }
         break;
      default:
         break;
   }
   return time;
}
/*=====[Implematation of private functions]==================================*/
