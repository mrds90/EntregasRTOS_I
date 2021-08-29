/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/08/01
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "sapi.h"


/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif


/*=====[Public MACRO and Constants]==========================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/
/**
 * @brief Start the button state machine
 * 
 */
void FSMButtonInit(gpioMap_t key_index);

/**
 * @brief Check if the button is released
 * 
 * @param tecla 
 */
bool_t CheckRaiseState(gpioMap_t key_index);
/**
 * @brief Check if the button is pressed
 * 
 * @param tecla 
 */
bool_t CheckFallState(gpioMap_t key_index);
/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __BUTTON_H__ */
