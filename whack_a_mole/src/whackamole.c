/**
BSD 3-Clause License

Copyright (c) 2021, Franco Bucafusco, Martin Menendez
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdarg.h>
#include <stdbool.h>


#include "sapi.h"


#include "FreeRTOS.h"
#include "../inc/FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "../inc/keys.h"
#include "../inc/mole.h"
#include "../inc/whackamole.h"


/* private macros */

#define WAM_GAMEPLAY_TIMEOUT        2000//15000   //gameplay time

#define QUEUE_SIZE                  3

#define MOLE_KEY(x)                 (x) + TEC1
#define MOLE_LED(x)                 (x) + LEDB

/* private types */



/* private prototypes */
void WHACKAMOLE_ServiceLogic( void * pvParameters );
void WHACKAMOLE_TimeOutControl(void* taskParmPtr);
void WHACKAMOLE_ServicePrint( void* taskParmPtr );
void WHACKAMOLE_ISRKeyPressed (t_key_isr_signal* event_data , uintptr_t context);

 

/**
   @brief init game

 */
void WHACKAMOLE_Init() {
    
    BaseType_t main_logic = xTaskCreate(
            WHACKAMOLE_ServiceLogic,
            (const char *)"WAM",
            configMINIMAL_STACK_SIZE * 2,
            NULL,
            tskIDLE_PRIORITY + 1,
            NULL
    );
    configASSERT(main_logic == pdPASS);

}
/**
   @brief servicio principal del juego

   @param pvParameters
 */
void WHACKAMOLE_ServiceLogic( void * pvParameters ) {
    static wack_a_mole_t wam;
    static mole_t mole[WAM_MOLE_QTY];
    static TaskHandle_t task_mole[WAM_MOLE_QTY];
    static TaskHandle_t task_time_out = NULL;
    print_info_t print_info;
    BaseType_t task_return;
    
    wam.event_queue = xQueueCreate(QUEUE_SIZE,sizeof(print_info_t));
    configASSERT(wam.event_queue != NULL);
    
    wam.print_queue = xQueueCreate(QUEUE_SIZE, sizeof(print_info_t));
    configASSERT( wam.print_queue != NULL );
    
    task_return = xTaskCreate(
            WHACKAMOLE_ServicePrint,
            (const char *)"Print",
            configMINIMAL_STACK_SIZE * 3,
            (void*) wam.print_queue,
            tskIDLE_PRIORITY + 1,
            NULL
    );
    configASSERT(task_return == pdPASS);
    
    while (1) {
        switch (wam.state) {
        case WAM_STATE_INIT:
            
            KEYS_LoadPressHandler( WHACKAMOLE_ISRKeyPressed, (uintptr_t) &mole[0] );
            task_return = xTaskCreate(
                WHACKAMOLE_TimeOutControl,
                (const char *)"TimeOut",
                configMINIMAL_STACK_SIZE,
                (void *) &wam,
                tskIDLE_PRIORITY + 1,
                &task_time_out
            );
            configASSERT(task_return == pdPASS);

            for (mole_index_t i = 0; i < WAM_MOLE_QTY; i++) {
                mole[i].key = MOLE_KEY(i);
                mole[i].led = MOLE_LED(i);
                
                mole[i].semaphore = xSemaphoreCreateBinary();
                configASSERT(mole[i].semaphore != NULL);
                
                mole[i].report_queue = wam.event_queue; //Observar que aca se igualan estas colas. Para lograr encapsular y evitar la sentencia "extern".
                mole[i].last_time = 0;
            
                task_return = xTaskCreate(
                    MOLE_ServiceLogic,
                    (const char *)"MOLE",
                    configMINIMAL_STACK_SIZE ,
                    (void *) &mole[i],
                    tskIDLE_PRIORITY + 1,
                    &task_mole[i]
                );
                configASSERT(task_return == pdPASS);
            }
            wam.queue_time_out = xQueueCreate(QUEUE_SIZE, sizeof(print_info_t));
            configASSERT(wam.queue_time_out != NULL);
            wam.state = WAM_STATE_GAMEPLAY;
            break;

        case WAM_STATE_GAMEPLAY:
            xQueueReceive(wam.event_queue, &print_info ,portMAX_DELAY);
            wam.points += print_info.points;
            print_info.points = wam.points;
            if (print_info.event != EVENT_MISS) {
                xQueueSend(wam.queue_time_out, &print_info, portMAX_DELAY);
            }
            xQueueSend(wam.print_queue, &print_info, portMAX_DELAY);
            break;
        case WAM_STATE_END:
            vQueueDelete(wam.queue_time_out);
            vTaskDelete(task_time_out);
            for (mole_index_t i = 0; i < WAM_MOLE_QTY; i++) {
                vTaskDelete(task_mole[i]);
                vSemaphoreDelete(mole[i].semaphore);
            }
            wam.state = WAM_STATE_INIT;
            break;
        default:
            break;
        }
    }
}

void WHACKAMOLE_TimeOutControl(void* taskParmPtr) {
    BaseType_t time_out_check;
    print_info_t print_info;
    wack_a_mole_t* wam = (wack_a_mole_t*) taskParmPtr;
    
    while (1) {
        time_out_check = xQueueReceive(wam->queue_time_out, &print_info ,WAM_GAMEPLAY_TIMEOUT);
        if(time_out_check != pdTRUE) {
            print_info.event = EVENT_GAME_OVER;
            print_info.points = wam->points;
            wam->state = WAM_STATE_END;
            xQueueReset(wam->print_queue);
            xQueueSend(wam->print_queue, &print_info, portMAX_DELAY);
        }
    }
}

void WHACKAMOLE_ServicePrint( void* taskParmPtr ) {
    QueueHandle_t print_queue = (QueueHandle_t) taskParmPtr;
    print_info_t game;
    char str[100];
    while( 1 ) {
        xQueueReceive(print_queue, &game, portMAX_DELAY);
        switch (game.event) {
            case EVENT_WAKE_UP:
                sprintf( str, "Presione cualquier tecla por un rato r\n");
                break;
            case EVENT_GAME_START:
                sprintf( str, "A GOLPEAR! r\n" );
                break;
            case EVENT_HIT:
                sprintf( str, "HIT! - Score: %i.\r\n", game.points);
                break;
            case EVENT_MISS:
                sprintf( str, "MISS! - Score: %i.\r\n", game.points);
                break;
            case EVENT_FAIL:
                sprintf( str, "NOTHING THERE! - Score: %i.\r\n", game.points);
                break;
            case EVENT_GAME_OVER:
                sprintf( str, "JUEGO TERMINADO: - Score: %i.\r\n", game.points);
                
                break;
            }

        uartWriteString(UART_USB, str);
    }
}


void WHACKAMOLE_ISRKeyPressed (t_key_isr_signal* event_data , uintptr_t context) {
    mole_t * moles = (mole_t*)context;
    
    switch (event_data->tecla) {
        case TEC1_INDEX:
            moles += WAM_MOLE_1;
            break;
        case TEC2_INDEX:
            moles += WAM_MOLE_2;
            break;
        case TEC3_INDEX:
            moles += WAM_MOLE_3;
            break;
        case TEC4_INDEX:
            moles += WAM_MOLE_4;
            break;
    }
    taskENTER_CRITICAL();
    moles->last_time = tickRead();
    taskEXIT_CRITICAL();
    xSemaphoreGive(moles->semaphore);
}

