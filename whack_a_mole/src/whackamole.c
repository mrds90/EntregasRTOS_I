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
#include "../inc/random.h"



/* private macros */

#define WAM_GAMEPLAY_TIMEOUT        15000   //gameplay time
#define WAM_GAME_END_DELAY          20000   //delay to end game
#define WAM_GAME_INIT_DELAY         500    //mole timeout
#define QUEUE_SIZE                  3

#define MOLE_KEY(x)                 (x) + TEC1
#define MOLE_LED(x)                 (x) + LEDB

/* private types */



/* private prototypes */
/**
 * @brief Rutina principal del juego. Tarea infinita con todos los estados del juego.
 * 
 * @param pvParameters 
 */
static void WHACKAMOLE_ServiceLogic( void * pvParameters );
/**
 * @brief Tarea que controla el tiempo de no presionado de las teclas durante el juego.
 * 
 * @param taskParmPtr 
 */
static void WHACKAMOLE_TimeOutControl(void* taskParmPtr);
/**
 * @brief Tarea que controla cuando finaliza el juego.
 * 
 * @param taskParmPtr 
 */
static void WHACKAMOLE_EndGame(void* taskParmPtr);
/**
 * @brief Tarea para imprimir en la pantalla.
 * 
 * @param taskParmPtr 
 */
static void WHACKAMOLE_ServicePrint( void* taskParmPtr );
/**
 * @brief Tarea de interrupción que gestiona los eventos de presionado de teclas durante el juego.
 * 
 * @param event_data 
 * @param context 
 */
static void WHACKAMOLE_ISRKeyPressedInGame (t_key_isr_signal* event_data , uintptr_t context);
/**
 * @brief Tarea de interrupción que gestiona los eventos de presionado y soltado de tecla para iniciar el juego.
 * 
 * @param event_data 
 * @param context 
 */
static void WHACKAMOLE_ISRKeyPressedOrReleasedInStart (t_key_isr_signal* event_data , uintptr_t context);
 

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
    /*Inicialización de los recursos permanentes del juego*/
    static wack_a_mole_t wam;
    static mole_t mole[WAM_MOLE_QTY];
    static TaskHandle_t task_mole[WAM_MOLE_QTY];
    static TaskHandle_t task_time_out = NULL;
    static TaskHandle_t task_end_game = NULL;
    
    print_info_t print_info = {EVENT_WAKE_UP, 0};
    BaseType_t task_return;
    
    wam.state = WAM_STATE_BOOTING;
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
            case WAM_STATE_BOOTING: //inicialización del juego
                KEYS_LoadPressHandler( WHACKAMOLE_ISRKeyPressedOrReleasedInStart, (uintptr_t) &wam.event_queue ); // carga de la interrupción de presionado de tecla
                KEYS_LoadReleaseHandler( WHACKAMOLE_ISRKeyPressedOrReleasedInStart, (uintptr_t) &wam.event_queue ); // carga de la interrupción de soltado de tecla
                xQueueSend(wam.print_queue, &print_info, portMAX_DELAY); // impresión de mensaje de inicio
                wam.state = WAM_STATE_INIT; // cambio de estado
            case WAM_STATE_INIT: //!< Estado donde espera que se presione alguna tecla.
                xQueueReceive( wam.event_queue, &print_info, portMAX_DELAY ); //Con un semaforo era suficiente pero la cola ya la tenia instanciada y no quería crear otro recurso exclusivo para esto.
                if(xQueueReceive( wam.event_queue, &print_info, WAM_GAME_INIT_DELAY ) == pdFALSE) { //Si no se solto la tecla en WAM_GAME_INIT_DELAY se inicia el juego
                    xQueueReceive( wam.event_queue, &print_info, portMAX_DELAY ); //Se espera a que se solte la tecla definitivamente
                    random_seed_freertos(); //Se inicializa la semilla del generador de números aleatorios
                    xQueueSend(wam.print_queue, &print_info, portMAX_DELAY); //Se envia el evento de inicio de juego para imprimir por UART
                    KEYS_LoadReleaseHandler( NULL, 0 ); //Se desactiva la interrupción de teclado de Release
                    KEYS_LoadPressHandler( WHACKAMOLE_ISRKeyPressedInGame, (uintptr_t) &mole[0] ); //Se actualiza la interrupción de teclado de Press
                    task_return = xTaskCreate( // Se Crea tarea que controla el Time Out por no presionar teclas
                        WHACKAMOLE_TimeOutControl,
                        (const char *)"TimeOut",
                        configMINIMAL_STACK_SIZE,
                        (void *) &wam,
                        tskIDLE_PRIORITY + 1,
                        &task_time_out
                    );
                    task_return = xTaskCreate( // Se Crea tarea que controla el tiempo de juego
                        WHACKAMOLE_EndGame,
                        (const char *)"End Control",
                        configMINIMAL_STACK_SIZE,
                        (void *) &wam,
                        tskIDLE_PRIORITY + 1,
                        &task_end_game
                    );
                    configASSERT(task_return == pdPASS);

                    for (mole_index_t i = 0; i < WAM_MOLE_QTY; i++) { //Se inicializan las moles
                        mole[i].key = MOLE_KEY(i);
                        mole[i].led = MOLE_LED(i);
                        
                        mole[i].semaphore = xSemaphoreCreateBinary();
                        configASSERT(mole[i].semaphore != NULL);
                        
                        mole[i].report_queue = wam.event_queue; //Observar que aca se igualan estas colas. Para lograr encapsular y evitar la sentencia "extern".
                        mole[i].last_time = 0;
                    
                        task_return = xTaskCreate( //Se crea tarea que controla cada mole
                            MOLE_ServiceLogic,
                            (const char *)"MOLE",
                            configMINIMAL_STACK_SIZE ,
                            (void *) &mole[i],
                            tskIDLE_PRIORITY + 1,
                            &task_mole[i]
                        );
                        configASSERT(task_return == pdPASS);
                    }
                    wam.semph_time_out = xSemaphoreCreateBinary(); //Se crea cola para controlar el tiempo de timeout
                    configASSERT(wam.semph_time_out != NULL);
                    wam.state = WAM_STATE_GAMEPLAY;
                }
                break;

            case WAM_STATE_GAMEPLAY:
                xQueueReceive(wam.event_queue, &print_info ,portMAX_DELAY);
                wam.points += print_info.points; //Se suman los puntos del mole que se acaba de aparecer
                print_info.points = wam.points; //Se actualiza el punto para imprimir
                if (print_info.event != EVENT_MISS) { //Reset del contador de timeout
                	xSemaphoreGive(wam.semph_time_out);
                }
                xQueueSend(wam.print_queue, &print_info, portMAX_DELAY); //Se envia el evento para imprimir por UART
                break;
            case WAM_STATE_END: //!< Estado final del juego. Se borran los recursos utilizados durante el juego
                if (wam.semph_time_out != NULL) {
                    vSemaphoreDelete(wam.semph_time_out);
                    wam.semph_time_out = NULL;
                }
                if (task_time_out != NULL) {
                    vTaskDelete(task_time_out);
                    task_time_out = NULL;
                }
                if (task_end_game != NULL) {
                    vTaskDelete(task_end_game);
                    task_end_game = NULL;
                }
                for (mole_index_t i = 0; i < WAM_MOLE_QTY; i++) {
                    if (task_mole[i] != NULL) {
                        vTaskDelete(task_mole[i]);
                        task_mole[i] = NULL;
                    }
                    if(mole[i].semaphore != NULL) {
                        vSemaphoreDelete(mole[i].semaphore);
                        mole[i].semaphore = NULL;
                    }
                }

                wam.state = WAM_STATE_BOOTING;
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
        // time_out_check = xQueueReceive(wam->semph_time_out, &print_info ,WAM_GAMEPLAY_TIMEOUT);
        time_out_check = xSemaphoreTake(wam->semph_time_out, portMAX_DELAY);
        if(time_out_check != pdTRUE) {
            print_info.event = EVENT_GAME_OVER;
            print_info.points = wam->points;
            wam->state = WAM_STATE_END;
            xQueueReset(wam->print_queue);
            xQueueSend(wam->print_queue, &print_info, portMAX_DELAY);
        }
    }
}

void WHACKAMOLE_EndGame(void* taskParmPtr) {
    print_info_t print_info;
    while (1) {
        vTaskDelay(WAM_GAME_END_DELAY);
        wack_a_mole_t* wam = (wack_a_mole_t*) taskParmPtr;
        print_info.event = EVENT_GAME_OVER;
        print_info.points = wam->points;
        wam->state = WAM_STATE_END;
        xQueueReset(wam->print_queue);
        xQueueSend(wam->print_queue, &print_info, portMAX_DELAY);
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


void WHACKAMOLE_ISRKeyPressedInGame (t_key_isr_signal* event_data , uintptr_t context) {
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


void WHACKAMOLE_ISRKeyPressedOrReleasedInStart (t_key_isr_signal* event_data , uintptr_t context) {
    QueueHandle_t* queue = (QueueHandle_t*) context;
    print_info_t print_info;
    print_info.event = EVENT_GAME_START;
    print_info.points = 0;
    xQueueSend(*queue, &print_info, portMAX_DELAY);
}

