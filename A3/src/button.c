/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/08/01
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "button.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definition of private data type]==============================*/
typedef enum {
    UP,
    FALLING,
    DOWN,
    RISING,

    DEBOUNCE_QTY,
} debounce_state_t;
    
typedef struct {
   gpioMap_t tecla;
   debounce_state_t state;
   tick_t down_time;
   bool_t flag_rise;
   bool_t flag_fall;
} dbn_t; 	

/*=====[Definitions of private global variables]=============================*/
static dbn_t edu_ciaa_keys[TEC4-TEC1+1];

/*=====[Delaration of private functions]==================================*/
/**
 * @brief Update the state of the button
 * 
 * @param key_index 
 */
static void FSMButtonUpdate(gpioMap_t key_index);
/**
 * @brief Up State function of the button
 * 
 * @param dataTecla 
 */
static void UpState(dbn_t *dataTecla);
/**
 * @brief Falling State function of the button
 * 
 * @param dataTecla 
 */
static void FallingState(dbn_t *dataTecla);
/**
 * @brief Down State function of the button
 * 
 * @param dataTecla 
 */
static void DownState(dbn_t *dataTecla);
/**
 * @brief Rising State function of the button
 * 
 * @param dataTecla 
 */
static void RisingState(dbn_t *dataTecla);
/**
 * @brief Release event function of the button
 * 
 * @param dataTecla 
 */
static void buttonReleased(dbn_t *tecla);
/**
 * @brief Press event function of the button
 * 
 * @param tecla 
 */
static void buttonPressed(dbn_t *tecla);

/*=====[Implematation of public functions]==================================*/
void FSMButtonInit(gpioMap_t key_index) {
   uint8_t array_index = key_index - TEC1;
   edu_ciaa_keys[array_index].tecla = key_index;
   edu_ciaa_keys[array_index].state = UP;
   edu_ciaa_keys[array_index].flag_rise = FALSE;
   edu_ciaa_keys[array_index].flag_fall = FALSE;
}

bool_t CheckRaiseState(gpioMap_t key_index) { 
   uint8_t array_index = key_index - TEC1;
   FSMButtonUpdate(key_index);
   bool_t flag = edu_ciaa_keys[array_index].flag_rise;   
   edu_ciaa_keys[array_index].flag_rise = 0;
   return flag;
}

bool_t CheckFallState(gpioMap_t key_index) { 
   uint8_t array_index = key_index - TEC1;
   FSMButtonUpdate(key_index);
   bool_t flag = edu_ciaa_keys[array_index].flag_fall;
   edu_ciaa_keys[array_index].flag_fall = 0;
   return flag;
}
/*=====[Implematation of private functions]==================================*/

static void FSMButtonUpdate(gpioMap_t key_index) {
   uint8_t array_index = key_index - TEC1;
   edu_ciaa_keys[array_index].tecla = key_index;
   void (*DebounceFunc[DEBOUNCE_QTY])(dbn_t *dataTecla) = {UpState, FallingState ,DownState, RisingState};
   DebounceFunc[edu_ciaa_keys[array_index].state](&edu_ciaa_keys[array_index]);
}

static void UpState(dbn_t *dataTecla) {
   if (!gpioRead(dataTecla->tecla)) {
            dataTecla->state = FALLING;
   }
}

static void FallingState(dbn_t *dataTecla) {
   if (!gpioRead(dataTecla->tecla)) {
      dataTecla->state = DOWN;
      buttonPressed(dataTecla);
   }
   else {
      dataTecla->state = UP;
   }
}

static void DownState(dbn_t *dataTecla) {
   if (gpioRead(dataTecla->tecla)) {
      dataTecla->state = RISING;
   }
}

static void RisingState(dbn_t *dataTecla) {
   if (gpioRead(dataTecla->tecla)) {
      dataTecla->state = UP;
      buttonReleased(dataTecla);
   }
   else {
      dataTecla->state = DOWN;
   }
}

static inline void buttonReleased(dbn_t *tecla) {
   tecla->flag_rise = 0xFF;
}

static inline void buttonPressed(dbn_t *tecla) {
   tecla->flag_fall = 0xFF;
}
