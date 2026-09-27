#ifndef UART_H
#define	UART_H
/**
 * @file    Uart.h
 *
 * Library for two seperate UART interafaces (UART1 & UART6).
 *
 * @author  HARE Lab
 * @author  jLab
 * @author  Adam Korycki
 * @version 1.0.0
 * 
 * @date    2 Nov 2023
 */
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart6_rx;

/** UART1_Init(rate)
 *
 * Initializes UART1 interface at specified baudrate.
 *
 * @param   rate        (int)       Baudrate (must be between 9600 and 115200).
 * @param   rxBuffer    (uint8_t *) Pointer to receiver buffer for USART1.
 * @return              (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_Init(int rate, uint8_t* rx1Buffer);

/** UART1_ReadData(data, size)
 *
 * Reads data from the RX buffer for UART1.
 *
 * @param data  (uint8_t *) Pointer to received buffer.
 * @param size  (uint16_t)  Number of bytes to read from buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_ReadData(uint8_t* data, uint16_t size);

/** UART1_WriteData(data, size)
 *
 * Transmits <size> number of bytes on UART1
 * 
 * @param data  (uint8_t *) Byte buffer to transmit.
 * @param size  (uint16_t)  Number of bytes to send to buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_WriteData(uint8_t* data, uint16_t size);

/** UART6_Init(rate)
 *
 * Initializes UART6 interface at specified baudrate.
 *
 * @param   rate        (int)       Baudrate (must be between 9600 and 115200).
 * @param   rxBuffer    (uint8_t *) Pointer to receiver buffer for USART6.
 * @return              (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_Init(int rate, uint8_t* rx6Buffer);

/** UART6_ReadData(data, size)
 *
 * Reads data from the RX buffer for UART6.
 *
 * @param data  (uint8_t *) Pointer to received buffer.
 * @param size  (uint16_t)  Number of bytes to read from buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_ReadData(uint8_t* data, uint16_t size);

/** UART6_WriteData(data, size)
 *
 * Transmits <size> number of bytes on UART6.
 * 
 * @param data  (uint8_t *) Byte buffer to transmit.
 * @param size  (uint16_t)  Number of bytes to send to buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_WriteData(uint8_t* data, uint16_t size);

#endif
