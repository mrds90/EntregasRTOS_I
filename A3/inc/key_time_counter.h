/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/08/01
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __KEY_TIME_COUNTER_H__
#define __KEY_TIME_COUNTER_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "sapi.h"


/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif


/*=====[Public MACRO and Constants]==========================================*/
#define DEBOUNCE_TIME       40

/*=====[Prototypes (declarations) of public functions]=======================*/
/**
 * @brief Start the key time counter
 * 
 */
void FSMKeyTimeCounterInit(void);
/**
 * @brief Update the key time counter
 * 
 */
tick_t FSMKeyTimeCounterUpdate(void *param);
/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __KEY_TIME_COUNTER_H__ */
