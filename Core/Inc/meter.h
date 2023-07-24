#ifndef __METER_H__
#define __METER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#define RX_LENGTH 16
#define CCR_START_VAL 735
#define CCR_END_VAL 3670
#define MAX_SPEED 120.0
#define PSC_VAL 8
#define ARR_VAL 4000
#define clearMsg(msg, size) memset(msg, 0, size)

    extern uint8_t rxbuf[RX_LENGTH];
    extern uint8_t command[RX_LENGTH];
    extern float speed;
    typedef enum {
        CMD_INVALID,
        CMD_NULL,
        CMD_GET,
        CMD_SET,
        CMD_INC,
        CMD_DEC,
        CMD_LIGHTON,
        CMD_LIGHTOFF,
        CMD_LEDON,
        CMD_LEDOFF,
        CMD_SET_CCR,
        CMD_SET_ARR,
        CMD_SET_PSC
    } CommandTypeDef;


    void SetMeterWithSpeed(void);
    uint16_t getCCR(float speed);
    void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);
    void UART_IDLE_Callback(UART_HandleTypeDef* huart);
    CommandTypeDef runUARTCommand(UART_HandleTypeDef* huart, uint8_t* command);
    void getOutputMsg(char* msg, const char* startMsg);
    void itoa_user(int num_in, char* str, uint8_t useless);


#ifdef __cplusplus
}
#endif

#endif
