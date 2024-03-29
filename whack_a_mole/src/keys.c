/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "FreeRTOS.h"
#include "task.h"

#include "sapi.h"
#include "../inc/keys.h"

/*=====[ Definitions of private data types ]===================================*/
typedef enum {
    KEY_PRESSED,
    KEY_RELEASED,

    KEY_EVENTS_QTY
} key_event_t;
/*=====[Definition macros of private constants]==============================*/
#define DEBOUNCE_TIME   40
#define DEBOUNCE_TIME_TICKS     pdMS_TO_TICKS(DEBOUNCE_TIME)



/*=====[Prototypes (declarations) of private functions]======================*/
static void KEYS_ISRConfig( void );

static void KEYS_ISRButonPressed( t_key_isr_signal* event_data );
static void KEYS_ISRButonReleased( t_key_isr_signal* event_data );

/*=====[Definitions of private global variables]=============================*/

const t_key_config  keys_config[TEC_QTY] = { [TEC1_INDEX]= {TEC1}, [TEC2_INDEX]= {TEC2}, [TEC3_INDEX]= {TEC3}, [TEC4_INDEX]= {TEC4} } ;

t_key_data keys_data[TEC_QTY];

xQueueHandle isr_queue; //almacenara el evento en una cola

USER_KEYS_EVENT_HANDLER_BUTTON_PRESSED_t user_keys_event_handler_button_pressed  = NULL;
USER_KEYS_EVENT_HANDLER_BUTTON_RELEASED_t user_keys_event_handler_button_release = NULL;
uintptr_t context[KEY_EVENTS_QTY];

/*=====[Definitions of public global variables]==============================*/

/*=====[prototype of private functions]=================================*/
void KEYS_Task( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/

/**
   @brief Obtiene el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
   @return TickType_t
 */
TickType_t KEYS_DiffValue( uint32_t index )
{
    TickType_t tiempo;

    taskENTER_CRITICAL();
    tiempo = keys_data[index].time_diff;
    taskEXIT_CRITICAL();

    return tiempo;
}

/**
   @brief Borra el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
 */
void KEYS_DiffClear( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff = 0;
    taskEXIT_CRITICAL();
}


/**
   @brief Inicializa el driver

 */
void KEYS_Init( void ) {
    BaseType_t res;

    isr_queue = xQueueCreate( 10 , sizeof( t_key_isr_signal ) ) ;

    configASSERT( isr_queue != NULL );

    for( uint32_t i = 0 ; i < TEC_QTY ; i++ )
    {
        keys_data[i].state         = BUTTON_UP;  // Set initial state
        keys_data[i].time_down      = KEYS_INVALID_TIME;
        keys_data[i].time_up        = KEYS_INVALID_TIME;
        keys_data[i].time_diff      = KEYS_INVALID_TIME;

        //  configASSERT( keys_data[i].pressed_signal != NULL );
    }

    // Crear tareas en freeRTOS
    res = xTaskCreate (
              KEYS_Task,					// Funcion de la tarea a ejecutar
              ( const char * )"KEYS_Task",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
              0,							// Parametros de tarea
              tskIDLE_PRIORITY+3,			// Prioridad de la tarea
              0							// Puntero a la tarea creada en el sistema
          );


    KEYS_ISRConfig();


    // Gestión de errores
    configASSERT( res == pdPASS );
}

void KEYS_LoadPressHandler( USER_KEYS_EVENT_HANDLER_BUTTON_PRESSED_t press_handler, uintptr_t this_context ) {
    user_keys_event_handler_button_pressed = press_handler;
    context[KEY_PRESSED] = this_context;
}

void KEYS_LoadReleaseHandler( USER_KEYS_EVENT_HANDLER_BUTTON_RELEASED_t release_handler, uintptr_t this_context ) {
    user_keys_event_handler_button_release = release_handler;
    context[KEY_RELEASED] = this_context;
}

/**
   @brief Keys SM for isr operation

   @param index
 */
void KEYS_ISRUpdate( t_key_isr_signal* event_data )
{
    uint32_t index = event_data->tecla;

    switch( keys_data[index].state )
    {
        case STATE_BUTTON_UP:

            if( event_data->event_type == TEC_FALL )
            {
                vTaskDelay( DEBOUNCE_TIME_TICKS );

                if( !gpioRead( keys_config[index].tecla ) )
                {
                    keys_data[index].state = STATE_BUTTON_DOWN;

                    /* ACCION DEL EVENTO !*/
                    KEYS_ISRButonPressed( event_data );
                }
            }
            break;


        case STATE_BUTTON_DOWN:

            if( event_data->event_type == TEC_RISE )
            {
                vTaskDelay( DEBOUNCE_TIME_TICKS );

                if(  gpioRead( keys_config[index].tecla ) )
                {
                    keys_data[index].state = STATE_BUTTON_UP;

                    /* ACCION DEL EVENTO !*/
                    KEYS_ISRButonReleased( event_data );
                }
            }

            break;
    }
}



/*=====[Implementations of private functions]================================*/



/**
   @brief   Manejador del evento de tecla pulsada

   @param event_data
 */
static void KEYS_ISRButonPressed( t_key_isr_signal* event_data ) {
    uint32_t index = event_data->tecla;

    //internal event
    taskENTER_CRITICAL();
    keys_data[index].time_down = event_data->event_time;
    taskEXIT_CRITICAL();

    //user event
    if( user_keys_event_handler_button_pressed != NULL ) {
        user_keys_event_handler_button_pressed( event_data, context[KEY_PRESSED] );
    }
}

/**
   @brief   Manejador del evento de tecla liberada

   @param event_data
 */
static void KEYS_ISRButonReleased( t_key_isr_signal* event_data )
{
    uint32_t index = event_data->tecla;

    //internal event
    taskENTER_CRITICAL();
    keys_data[index].time_up    = event_data->event_time;
    keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
    taskEXIT_CRITICAL();

    if (user_keys_event_handler_button_release != NULL) {
        user_keys_event_handler_button_release( event_data, context[KEY_RELEASED] );
    }
}

/*=====[Implementations of private functions]=================================*/

/**
   @brief Implementacion de tarea que le da servicio al driver

   @param taskParmPtr
 */
void KEYS_Task( void* taskParmPtr )
{
    t_key_isr_signal event_data;

    while( 1 )
    {
        xQueueReceive( isr_queue, &event_data, portMAX_DELAY );
        KEYS_ISRUpdate( &event_data );
    }
}


/**
   @brief   Inicializa las interrupciones asociadas al driver keys.c
			Se realiza sobre las cuatro teclas de la EDUCIAA
 */
void KEYS_ISRConfig( void ) {
    //Inicializamos las interrupciones (LPCopen)
    Chip_PININT_Init( LPC_GPIO_PIN_INT );

    //Inicializamos de cada evento de interrupcion (LPCopen)

    /* Machete:
    GLOBAL! extern pinInitGpioLpc4337_t gpioPinsInit[];
    Chip_SCU_GPIOIntPinSel( j,  gpioPinsInit[i].gpio.port, gpioPinsInit[i].gpio.pin );   // TECi
    Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj (canal j -> hanlder GPIOj)       //Borra el pending de la IRQ
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( j ) );                      // INTj //Selecciona activo por flanco
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( j ) );                        // INTj //Selecciona activo por flanco descendente
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( j ) );                       // INTj //Selecciona activo por flanco ascendente
    */


    // TEC1 FALL +  TEC1 RISE
    Chip_SCU_GPIOIntPinSel( 0, 0, 4 ); 	//(Canal 0 a 7, Puerto GPIO, Pin GPIO)
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura el canal para que se active por flanco
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH0 ); //Se configura para que el flanco sea el de bajada
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH0 ); //En este caso el flanco es de subida

    // TEC2 FALL + TEC2 RISE
    Chip_SCU_GPIOIntPinSel( 1, 0, 8 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH1 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH1 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH1 );


    // TEC3 FALL + TEC3 RISE
    Chip_SCU_GPIOIntPinSel( 2, 0, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH2 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH2 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH2 );

    // TEC4 FALL + TEC4 RISE
    Chip_SCU_GPIOIntPinSel( 3, 1, 9 );
    Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH3 );
    Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH3 );
    Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH3 );

    //Una vez que se han configurado los eventos para cada canal de interrupcion
    //Se activan las interrupciones para que comiencen a llamar al handler
    NVIC_SetPriority( PIN_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_SetPriority( PIN_INT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT1_IRQn );
    NVIC_SetPriority( PIN_INT2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_SetPriority( PIN_INT3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
}



/**
   @brief handler de evento de tecla pulsada

   @param index
 */
static void KEYS_ISRFall( uint32_t index, t_key_isr_signal*evnt ) {
    evnt->tecla         = index;
    evnt->event_time    = xTaskGetTickCountFromISR();
    evnt->event_type    = TEC_FALL;
}

/**
   @brief handler de evento de tecla liberada

   @param index
 */
static void KEYS_ISRRise( uint32_t index, t_key_isr_signal*evnt ) {
    evnt->tecla         = index;
    evnt->event_time    = xTaskGetTickCountFromISR();
    evnt->event_type    = TEC_RISE;
}

/**
   @brief Hanlder de ISR usado para detectar flanco ascendente y descendente de la tecla TEC1_INDEX
 */
void GPIO0_IRQHandler( void ) {  //asociado a tec1

    t_key_isr_signal event_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH0 ) {
        //Verificamos que la interrupción es la esperada
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 ); //Borramos el flag de interrupción

        KEYS_ISRFall( TEC1_INDEX, &event_data );
        xQueueSendFromISR(  isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH0 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH0 );
        KEYS_ISRRise( TEC1_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
   @brief Hanlder de ISR usado para detectar flanco ascendente y descendente de la tecla TEC2_INDEX
 */
void GPIO1_IRQHandler( void ) {  //asociado a tec1

    t_key_isr_signal event_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH1 )
    {
        //Verificamos que la interrupción es la esperada
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH1 ); //Borramos el flag de interrupción

        KEYS_ISRFall( TEC2_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH1 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH1 );
        KEYS_ISRRise( TEC2_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
   @brief Hanlder de ISR usado para detectar flanco ascendente y descendente de la tecla TEC3_INDEX
 */
void GPIO2_IRQHandler( void ) {
    t_key_isr_signal event_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH2 )
    {
        //Verificamos que la interrupción es la esperada
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH2 ); //Borramos el flag de interrupción

        KEYS_ISRFall( TEC3_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH2 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH2 );
        KEYS_ISRRise( TEC3_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
   @brief Hanlder de ISR usado para detectar flanco ascendente y descendente de la tecla TEC4_INDEX
 */
void GPIO3_IRQHandler( void ) {
    t_key_isr_signal event_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

    if ( Chip_PININT_GetFallStates( LPC_GPIO_PIN_INT ) & PININTCH3 )
    {
        //Verificamos que la interrupción es la esperada
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH3 ); //Borramos el flag de interrupción

        KEYS_ISRFall( TEC4_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    if ( Chip_PININT_GetRiseStates( LPC_GPIO_PIN_INT ) & PININTCH3 )
    {
        Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH3 );
        KEYS_ISRRise( TEC4_INDEX, &event_data );
        xQueueSendFromISR( isr_queue, &event_data, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

