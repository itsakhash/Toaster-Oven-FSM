#ifndef BOARD_H
#define	BOARD_H
/**
 * @file    BOARD.h
 * 
 * Generic driver set up and clean up for functional testing for the UCSC Nucleo
 * I/O Shield connected to a Nucleo STM32F411RE development kit.
 *
 * @author  HARE Lab
 * @author  Adam Korycki
 *
 * @date    18 Dec 2024
 */

#include <stdint.h>
#ifdef STM32F4
#include "stm32f4xx_hal.h"
#endif  /*  STM32F4 */


/*  MODULE-LEVEL DEFINITIONS, MACROS    */
// Boolean defines for TRUE, FALSE, SUCCESS and ERROR.
#ifndef FALSE
#define FALSE ((int8_t) 0)
#endif  /*  FALSE   */
#ifndef TRUE
#define TRUE ((int8_t) 1)
#endif  /*  TRUE    */
#ifndef ERROR
#define ERROR ((int8_t) -1)
#endif  /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t) 1)
#endif  /*  SUCCESS */

// Helper macro definitions.
#ifdef STM32F4
#define SW1_STATE() HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)
#define SW2_STATE() HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)
#define SWITCH_STATES() (((SW1_STATE() << 1) + SW2_STATE()) & 0x03)
#endif  /*  STM32F4 */

// Define standard error codes.
enum {
  SIZE_ERROR = -1,
  STANDARD_ERROR
};

#ifdef STM32F4
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
#ifndef UART_RX_CIRCULAR_BUFFER_SIZE
#define UART_RX_CIRCULAR_BUFFER_SIZE 1000
#endif
#if defined(UART_RX_CIRCULAR_BUFFER_SIZE) && (UART_RX_CIRCULAR_BUFFER_SIZE > (0xFFFF))
#error UART_RX_CIRCULAR_BUFFER_SIZE may not exceed the DMA NDTR register 16-bit maximum value of 65535 (0xFFFF).
#endif
#define GETCHAR_LOOPBACK_ENABLE
// #define GETCHAR_ERASE_LAST_READ_BYTE_ENABLE
#endif


/*  PROTOTYPES  */
/** BOARD_Init()
 *
 * 1. Initialize the HAL framework.
 * 2. Configure the clocks and pins used by our Nucleo kit's setup for serial
 * communications, onboard LED statuses, and resetting with the Big Blue Button.
 *
 */
extern int8_t BOARD_Init(void);

/** BOARD_End()
 *
 * Shuts down all peripherals; turns all pins into input.
 *
 * @return  [SUCCESS, ERROR]  (int8_t)
 */
extern int8_t BOARD_End(void);

/** ErrorHandler()
 * 
 * Enter a state of blinking the status LED on-board the Nucleo, indefinitely.
 */
extern void ErrorHandler(void);

#endif	/* BOARD_H */
