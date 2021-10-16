/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef KEYS_H_
#define KEYS_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1


typedef enum {
    TEC1_INDEX,
    TEC2_INDEX,
    TEC3_INDEX,
    TEC4_INDEX,

    TEC_QTY
} teclas_index_t;


/* types ================================================================= */
typedef enum {
    TEC_FALL,
    TEC_RISE,
} tecla_event_t;

typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;

typedef struct
{
    gpioMap_t tecla;			//config
} t_key_config;

typedef struct
{
    keys_ButtonState_t state;   //variables
    TickType_t time_down;		//timestamp of the last High to Low transition of the key
    TickType_t time_up;		    //timestamp of the last Low to High transition of the key
    TickType_t time_diff;	    //variables


    // SemaphoreHandle_t pressed_signal;

} t_key_data;

typedef struct
{
    gpioMap_t     tecla;
    TickType_t	  event_time;
    tecla_event_t event_type;
} t_key_isr_signal;

typedef void (*USER_KEYS_EVENT_HANDLER_BUTTON_PRESSED_t)(t_key_isr_signal* event_data , uintptr_t context);
typedef void (*USER_KEYS_EVENT_HANDLER_BUTTON_RELEASED_t)(t_key_isr_signal* event_data , uintptr_t context);

/* methods ================================================================= */
void KEYS_Init(void);
void KEYS_LoadPressHandler( USER_KEYS_EVENT_HANDLER_BUTTON_PRESSED_t press_handler, uintptr_t this_context );
void KEYS_LoadReleaseHandler( USER_KEYS_EVENT_HANDLER_BUTTON_RELEASED_t release_handler, uintptr_t this_context );
TickType_t keys_get_diff( uint32_t index );
void keys_clear_diff();


#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
