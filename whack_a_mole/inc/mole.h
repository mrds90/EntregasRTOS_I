/*=============================================================================
 * Author: Marcos Dominguez <mrds0690@gmail.com>
 * Date: 2021/10/17
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/
#ifndef __MOLE_H__
#define __MOLE_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

typedef struct {
    gpioMap_t           key;
    gpioMap_t           led;                //led asociado al mole
    SemaphoreHandle_t    semaphore;          //semaforo para controlar el acceso al mole
    QueueHandle_t        report_queue;       //Cola por donde reporta lo sucedido
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

/*=====[Prototypes (declarations) of public functions]=======================*/
void MOLE_ServiceLogic( void* pvParameters );
void MOLE_Downs(mole_t* moles , uint8_t n_moles);
/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __I2C_CUSTOM_H__ */
