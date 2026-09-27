/**
 * @file    Uart.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Uart.h>

/*  MODULE-LEVEL DEFINITIONS, MACROS    */
// Boolean defines for TRUE, FALSE, SUCCESS and ERROR.
#ifndef FALSE
#define FALSE ((int8_t)0)
#endif /*  FALSE   */
#ifndef TRUE
#define TRUE ((int8_t)1)
#endif /*  TRUE    */
#ifndef ERROR
#define ERROR ((int8_t)-1)
#endif /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t)1)
#endif /*  SUCCESS */

static uint8_t initStatusUart1 = FALSE;
static uint8_t initStatusUart6 = FALSE;

/*  PROTOTYPES  */
int8_t UART1_ConfigClk(void);
int8_t UART1_ConfigPins(void);
int8_t UART6_ConfigClk(void);
int8_t UART6_ConfigPins(void);

/*  FUNCTIONS   */
/**
 * UART1_ConfigClk()
 * @return  (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_ConfigClk(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    return SUCCESS;
}

/**
 * UART6_ConfigClk()
 * @return  (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_ConfigClk(void)
{
    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    return SUCCESS;
}

/**
 * UART1_ConfigPins()
 * @return  (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_ConfigPins(void)
{
    // PB7  --> Rx.
    // PA15 --> Tx.
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    return SUCCESS;
}

int8_t UART1_ConfigDMA(void)
{
    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
        return ERROR;
    }

    __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

    return SUCCESS;
}

/**
 * UART6_ConfigPins()
 * @return  (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_ConfigPins(void)
{
    // PC7  --> Rx.
    // PC6  --> Tx.
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    return SUCCESS;
}

int8_t UART6_ConfigDMA(void)
{
    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA2_Stream1;
    hdma_usart6_rx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
        return ERROR;
    }

    __HAL_LINKDMA(&huart6, hdmarx, hdma_usart6_rx);

    return SUCCESS;
}

/** UART1_Init(rate)
 *
 * Initializes UART1 interface at specified baudrate.
 *
 * @param   rate        (int)       Baudrate (must be between 9600 and 115200).
 * @param   rxBuffer    (uint8_t *) Pointer to receiver buffer for USART1.
 * @return              (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_Init(int rate, uint8_t *rxBuffer)
{
    if ((rate < 9600) || (rate > 115200))
    {
        printf("Baudrate must be between 9600 and 115200.\r\n");
        return ERROR;
    }
    if (initStatusUart1 == FALSE)
    {
        UART1_ConfigClk();
        UART1_ConfigPins();

        // Actual UART configuration.
        huart1.Instance = USART1;
        huart1.Init.BaudRate = rate;
        huart1.Init.WordLength = UART_WORDLENGTH_8B;
        huart1.Init.StopBits = UART_STOPBITS_1;
        huart1.Init.Parity = UART_PARITY_NONE;
        huart1.Init.Mode = UART_MODE_TX_RX;
        huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart1.Init.OverSampling = UART_OVERSAMPLING_16;

        if (HAL_UART_Init(&huart1) != HAL_OK)
        {
            return ERROR;
        }

        UART1_ConfigDMA();

        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

        HAL_UART_Receive_IT(&huart1, rxBuffer, 1);
        // // Set up DMA to move bytes from UART1 RX data register to rxBuffer
        // if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxBuffer, /* # of elements in rxBuffer */) != HAL_OK)
        // {
        //     return ERROR;
        // }
        // // Optional buffer settings and redirection.
        // setvbuf(stdin, rxBuffer, _IONBF, UART_RX_CIRCULAR_BUFFER_SIZE); // No effect?
        // setvbuf(stdout, NULL, _IONBF, 0);

        initStatusUart1 = TRUE;
    }
    return SUCCESS;
}

/** UART1_ReadData(data, size)
 *
 * Reads data from the RX buffer for UART1.
 *
 * @param data  (uint8_t *) Pointer to received buffer.
 * @param size  (uint16_t)  Number of bytes to read from buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_ReadData(uint8_t *data, uint16_t size)
{
    if (initStatusUart1 == FALSE)
    {
        printf("UART1 not yet initialized\r\n");
        return ERROR;
    }
    HAL_UART_Receive_IT(&huart1, data, size);
    return SUCCESS;
}

/** UART1_WriteData(data, size)
 *
 * Transmits <size> number of bytes on UART1
 *
 * @param data  (uint8_t *) Byte buffer to transmit.
 * @param size  (uint16_t)  Number of bytes to send to buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART1_WriteData(uint8_t *data, uint16_t size)
{
    if (initStatusUart1 == FALSE)
    {
        printf("UART1 not yet initialized\r\n");
        return ERROR;
    }
    if (huart1.gState == HAL_UART_STATE_READY)
    {
        HAL_UART_Transmit_IT(&huart1, data, size);
        return SUCCESS;
    }
    return ERROR; // Return "ERROR" if not ready to transmit.
}

/** UART6_Init(rate)
 *
 * Initializes UART6 interface at specified baudrate.
 *
 * @param   rate        (int)       Baudrate (must be between 9600 and 115200).
 * @param   rxBuffer    (uint8_t *) Pointer to receiver buffer for USART6.
 * @return              (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_Init(int rate, uint8_t *rxBuffer)
{
    if ((rate < 9600) || (rate > 115200))
    {
        printf("Baudrate must be between 9600 and 115200.\r\n");
        return ERROR;
    }
    UART6_ConfigClk();
    UART6_ConfigPins();

    if (initStatusUart6 == FALSE)
    {
        huart6.Instance = USART6;
        huart6.Init.BaudRate = rate;
        huart6.Init.WordLength = UART_WORDLENGTH_8B;
        huart6.Init.StopBits = UART_STOPBITS_1;
        huart6.Init.Parity = UART_PARITY_NONE;
        huart6.Init.Mode = UART_MODE_TX_RX;
        huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart6.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&huart6) != HAL_OK)
        {
            return ERROR;
        }

        UART6_ConfigDMA();

        HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART6_IRQn);

        HAL_UART_Receive_IT(&huart6, rxBuffer, 1);

        // // Set up DMA to move bytes from UART6 RX data register to rxBuffer
        // if (HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rxBuffer, /* # of elements in rxBuffer */) != HAL_OK)
        // {
        //     return ERROR;
        // }
        // // Optional buffer settings and redirection.
        // setvbuf(stdin, rxBuffer, _IONBF, UART_RX_CIRCULAR_BUFFER_SIZE); // No effect?
        // setvbuf(stdout, NULL, _IONBF, 0);

        initStatusUart6 = TRUE;
    }

    return SUCCESS;
}

/** UART6_ReadData(data, size)
 *
 * Reads data from the RX buffer for UART6.
 *
 * @param data  (uint8_t *) Pointer to received buffer.
 * @param size  (uint16_t)  Number of bytes to read from buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_ReadData(uint8_t *data, uint16_t size)
{
    if (initStatusUart6 == FALSE)
    {
        printf("UART1 not yet initialized\r\n");
        return ERROR;
    }
    HAL_UART_Receive_IT(&huart6, data, size);
    return SUCCESS;
}

/** UART6_WriteData(data, size)
 *
 * Transmits <size> number of bytes on UART6.
 *
 * @param data  (uint8_t *) Byte buffer to transmit.
 * @param size  (uint16_t)  Number of bytes to send to buffer.
 * @return      (int8_t)    SUCCESS or ERROR.
 */
int8_t UART6_WriteData(uint8_t *data, uint16_t size)
{
    if (initStatusUart6 == FALSE)
    {
        printf("UART6 not yet initialized\r\n");
        return ERROR;
    }
    if (huart6.gState == HAL_UART_STATE_READY)
    {
        HAL_UART_Transmit_IT(&huart6, data, size);
        return SUCCESS;
    }
    return ERROR; // Return "ERROR" if not ready to transmit.
}

// #define UART_TEST
#ifdef UART_TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <BOARD.h>
#include <Uart.h>

#define UART_BUFFER_SIZE 128

static uint8_t rx1Buffer[UART_BUFFER_SIZE];
static uint8_t tx1Buffer[UART_BUFFER_SIZE];
static uint8_t rx6Buffer[UART_BUFFER_SIZE];
static uint8_t tx6Buffer[UART_BUFFER_SIZE];
volatile uint8_t rx1Index = 0;
volatile uint8_t rx6Index = 0;
volatile uint8_t rx1Complete = 0;
volatile uint8_t rx6Complete = 0;
volatile uint8_t tx1Complete = 1;
volatile uint8_t tx6Complete = 1;

uint8_t setup(void)
{
    return BOARD_Init();
}

/** Uart - test.
 * Connect UART1 and UART6 together;
 * if a classic chant is printed to std_out over uart2 then all is well.
 */
int main()
{
    setup();

    if (
        (UART1_Init(115200, rx1Buffer) == SUCCESS) &&
        (UART6_Init(115200, rx6Buffer) == SUCCESS))
    {
        printf("UARTs initialized.\r\n");
        int32_t beer = 0;
        tx1Complete = 0;
        tx6Complete = 0;

        sprintf(tx1Buffer, "%d bottles of beer on the wall\n", beer);
        beer++;
        sprintf(tx6Buffer, "%d bottles of beer on the wall\n", beer);
        UART1_WriteData(tx1Buffer, strlen(tx1Buffer));
        UART6_WriteData(tx6Buffer, strlen(tx6Buffer));
        while (1)
        {
            if (rx1Complete)
            {
                // Print received data.
                printf("UART1 (Rx): %s\r\n", rx1Buffer + 1);

                // Generate/Transmit new data.
                beer++;
                sprintf(tx6Buffer, "%d bottles of beer on the wall\n", beer);
                tx6Complete = 0;
                rx1Complete = 0;
                UART6_WriteData(tx6Buffer, strlen(tx6Buffer));
            }
            if (rx6Complete)
            {
                // Print received data.
                printf("UART6 (Rx): %s\r\n", rx6Buffer + 1);

                // Generate/Transmit new data.
                beer++;
                sprintf(tx1Buffer, "%d bottles of beer on the wall\n", beer);
                tx1Complete = 0;
                rx6Complete = 0;
                UART1_WriteData(tx1Buffer, strlen(tx1Buffer));
            }
            HAL_Delay(200);
        }
    }
    else
    {
        printf("UART initialization error.\r\n");
    }

    printf("DONE.\r\n");
    while (1)
        ;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (
            (rx1Index == 0) &&
            ((rx1Buffer[rx1Index] == 0) || (rx1Buffer[rx1Index] == 1))

        )
        {
            HAL_UART_Receive(&huart1, &rx1Buffer[rx1Index], 1, 1);
        }
        else if (
            (rx1Index >= UART_BUFFER_SIZE) ||
            ((rx1Index > 0) && (rx1Buffer[rx1Index] == '\n')))
        {
            rx1Complete = 1;
            rx1Index = 0;
        }
        else
        {
            rx1Complete = 0;
        }
        rx1Index++;
        HAL_UART_Receive_IT(&huart1, &rx1Buffer[rx1Index], 1);
    }
    else if (huart->Instance == USART6)
    {
        if ((rx6Index == 0) && (rx6Buffer[rx6Index] == 0))
        {
            HAL_UART_Receive(&huart6, &rx6Buffer[rx6Index], 1, 1);
        }
        else if (
            (rx6Index >= UART_BUFFER_SIZE) ||
            ((rx6Index > 0) && (rx6Buffer[rx6Index] == '\n')))
        {
            rx6Complete = 1;
            rx6Index = 0;
        }
        else
        {
            rx6Complete = 0;
        }
        rx6Index++;
        HAL_UART_Receive_IT(&huart6, &rx6Buffer[rx6Index], 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        tx1Complete = 1;
    }
    else if (huart == &huart6)
    {
        tx6Complete = 1;
    }
}
#endif /*  UART_TEST   */
