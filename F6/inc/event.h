/*=============================================================================
 * Copyright (c) 2021, Marcos Dominguez
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef EVENT_H_
#define EVENT_H_

#include "FreeRTOS.h"
#include "sapi.h"

/* public macros ================================================================= */


/* types ================================================================= */

typedef enum {
    EVENT_KEY_PRESSED,
    EVENT_LED_BLINK,
} event_list_t;

typedef struct {
    uint8_t type;
    uint8_t index;
    uint32_t value;
} event_t;

    
#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
