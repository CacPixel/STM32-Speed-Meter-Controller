#include "meter.h"
#include "tim.h"
#include "stdbool.h"
#include "string.h"
#include "gpio.h"
#include "stdlib.h"
#include "stdio.h"

uint8_t rxbuf[RX_LENGTH] = { 0 };
uint8_t command[RX_LENGTH] = { 0 };
bool isRxCompleted = false;
uint8_t rxbufPos = 0;
bool shouldSaveCommand = false;
float speed;
CommandTypeDef cmdtype = CMD_NULL;

void SetMeterWithSpeed(void) {
    htim4.Instance->CCR1 = getCCR(speed);

}

uint16_t getCCR(float speed) {
    return (float)(CCR_END_VAL - CCR_START_VAL) / MAX_SPEED * speed + CCR_START_VAL;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART1) {
        isRxCompleted = true;
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

    }
}

void UART_IDLE_Callback(UART_HandleTypeDef* huart) {
    //if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET)
    if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) == RESET)
        return;

    uint8_t msg[32] = { 0 };


    if (isRxCompleted) {
        uint8_t data = rxbuf[0];
        if (data == '/') {
            rxbufPos = 0;
            shouldSaveCommand = true;
            cmdtype = CMD_NULL;
        }
        if (shouldSaveCommand) {
            if (rxbufPos < RX_LENGTH) {
                command[rxbufPos] = data;
                rxbufPos++;
                if (data == ';') {
                    command[rxbufPos] = 0;
                    clearMsg(msg, sizeof(msg));
                    strcat((char*)msg, "/issue_cmd:\"");
                    strcat((char*)msg, (char*)command);
                    strcat((char*)msg, "\";\r\n");
                    //sprintf((char*)msg, "/issue_cmd:\"%s\";\r\n", command);
                    //HAL_UART_Transmit(huart, msg, strlen((char*)msg), 5);
                    HAL_Delay(10);

                    cmdtype = runUARTCommand(huart, command);
                    shouldSaveCommand = false;
                }
            } else {
                cmdtype = CMD_INVALID;
            }
        }
        if (cmdtype == CMD_INVALID) {
            //command invalid
            clearMsg(msg, sizeof(msg));
            strcat((char*)msg, "/cmd_invalid;\r\n");
            //sprintf((char*)msg, "/cmd_invalid;\r\n");
            HAL_UART_Transmit(huart, msg, strlen((char*)msg), 20);
            shouldSaveCommand = false;
            cmdtype = CMD_NULL;
        }
        isRxCompleted = false;
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
        HAL_UART_Receive_IT(huart, rxbuf, 1);
    }
}

CommandTypeDef runUARTCommand(UART_HandleTypeDef* huart, uint8_t* command) {
    uint8_t proc_str[RX_LENGTH];
    if (strstr((char*)command, "/get") != NULL) {
        cmdtype = CMD_GET;

        char msg[128] = { 0 };
        getOutputMsg(msg, "get");

        /*
        sprintf((char*)msg, "/get:speed=%.2f,CCR=%lu,PSC=%lu,ARR=%lu;\r\n", \
             speed, TIM4->CCR1, TIM4->PSC, TIM4->ARR);
        */
        HAL_UART_Transmit(huart, (uint8_t*)msg, strlen((char*)msg), 10);


    } else if (strstr((char*)command, "/set:") != NULL) {
        cmdtype = CMD_SET;
        char* speedPos = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, speedPos);
        speed = (float)atof((char*)proc_str);
        SetMeterWithSpeed();

        //sprintf((char*)msg, "/speed_set:%.2f;\r\n", speed);
        //HAL_UART_Transmit(huart, msg, strlen((char*)msg), 20);

    } else if (strstr((char*)command, "/inc:") != NULL) {
        cmdtype = CMD_INC;
        char* speedPos = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, speedPos);
        speed += (float)atof((char*)proc_str);
        SetMeterWithSpeed();
    } else if (strstr((char*)command, "/dec:") != NULL) {
        cmdtype = CMD_DEC;
        char* speedPos = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, speedPos);
        speed -= (float)atof((char*)proc_str);
        SetMeterWithSpeed();
    } else if (strstr((char*)command, "/light_on;") != NULL) {
        cmdtype = CMD_LIGHTON;
        LIGHT_ON;
    } else if (strstr((char*)command, "/light_off;") != NULL) {
        cmdtype = CMD_LIGHTOFF;
        LIGHT_OFF;
    } else if (strstr((char*)command, "/led_on;") != NULL) {
        cmdtype = CMD_LEDON;
        LED_ON;
    } else if (strstr((char*)command, "/led_off;") != NULL) {
        cmdtype = CMD_LEDOFF;
        LED_OFF;
    } else if (strstr((char*)command, "/ARR:") != NULL) {
        cmdtype = CMD_SET_ARR;
        char* p = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, p);
        htim4.Instance->ARR = (uint16_t)atoi((char*)proc_str);
    } else if (strstr((char*)command, "/PSC:") != NULL) {
        cmdtype = CMD_SET_PSC;
        char* p = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, p);
        htim4.Instance->PSC = (uint16_t)atoi((char*)proc_str);
    } else if (strstr((char*)command, "/CCR:") != NULL) {
        cmdtype = CMD_SET_CCR;
        char* p = strstr((char*)command, ":") + 1;
        strcpy((char*)proc_str, p);
        htim4.Instance->CCR1 = (uint16_t)atoi((char*)proc_str);
    }

    else {
        cmdtype = CMD_INVALID;
    }
    return cmdtype;
}

void getOutputMsg(char* msg, const char* startMsg) {

	sprintf((char*)msg, "/status:speed=%.2f,CCR=%u,PSC=%u,ARR=%u;\r\n", \
	      speed, TIM4->CCR1, TIM4->PSC, TIM4->ARR);
//    char temp[16] = { 0 };
//    int8_t speedIntergerPart = 0;
//    int16_t speedDecimalPart = 0;
//    int8_t speedDecimalPartFinal = 0;
//
//
//    //sprintf(msg + strlen(msg), "%.2f", 114.514);
//    //dont use sprintf in irqhandler
//    speedIntergerPart = (int8_t)speed;
//    speedDecimalPart = (int16_t)((speed - (float)speedIntergerPart) * 1000.0);
//    speedDecimalPartFinal = speedDecimalPart % 10;
//    speedDecimalPart = (speedDecimalPartFinal < 5) ? \
//        speedDecimalPart / 10 : \
//        speedDecimalPart / 10 + 1;
//
//    clearMsg(msg, 128);
//    strcat(msg, "/");
//    strcat(msg, startMsg);
//    strcat(msg, ":speed=");
//    itoa_user((int)speedIntergerPart, temp, 10);
//    strcat(msg, temp);
//    strcat(msg, ".");
//    itoa_user((int)speedDecimalPart, temp, 10);
//    if (speedDecimalPart < 10) {
//        temp[2] = 0;
//        temp[1] = temp[0];
//        temp[0] = '0';
//    }
//    strcat(msg, temp);
//    strcat(msg, ",CCR=");
//    itoa_user((int)TIM4->CCR1, temp, 10);
//    strcat(msg, temp);
//    strcat(msg, ",PSC=");
//    itoa_user((int)TIM4->PSC, temp, 10);
//    strcat(msg, temp);
//    strcat(msg, ",ARR=");
//    itoa_user((int)TIM4->ARR, temp, 10);
//    strcat(msg, temp);
//    strcat(msg, ";\r\n\0");
}

void itoa_user(int num_in, char* str, uint8_t useless) {
    UNUSED(useless);
    int num = num_in;
    uint8_t i = 0;
    uint8_t temp = 0;
    uint32_t div = 10000000;
    bool shouldStart = false;
    if (num == 0) {
        str[0] = '0';
        str[1] = 0;
        return;
    }
    if (num < 0) {
        str[i] = '-';
        i++;
        num = -num;
    }

    while (div != 0) {
        temp = num / div;
        if (temp != 0 || shouldStart) {
            shouldStart = true;
            str[i] = temp + '0';
            i++;
            num = num - temp * div;
        }
        div /= 10;
    }

    str[i] = 0;

}
