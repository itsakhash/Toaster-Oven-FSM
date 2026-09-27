/**
 * @file    BOARD.c
 *
 * Generic driver set up and clean up for functional testing for the UCSC Nucleo
 * I/O Shield connected to a Nucleo STM32F411RE development kit.
 *
 * @author  HARE Lab
 * @author  jLab
 * @author  Adam Korycki
 *
 * @date    18 Dec 2024
 * @todo    Make returned statuses meaningful.
 */
#ifdef STM32F4
#include <BOARD.h>
#include <Dma.h>
#include <stdio.h>
#else
#include <stdlib.h>
#include "BOARD.h"
#endif /*  STM32F4  */

/*  PROTOTYPES  */
void SystemClock_Config(void);
static int8_t Nucleo_Init(void);
static int8_t Serial_Init(void);

/*  MODULE-LEVEL VARIABLES, MACROS  */
static int8_t initStatus = FALSE;

// Define UART2 handler and setup printf() functionality.
#ifdef STM32F4
static char rxBuffer[UART_RX_CIRCULAR_BUFFER_SIZE];
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

GETCHAR_PROTOTYPE
{
    uint8_t ch = 0;

    // Clear the Overrun flag just before receiving the first character.
    // __HAL_UART_CLEAR_OREFLAG(&huart2);

    // Wait for reception of a character on the USART RX line and echo this
    // character on console.
    // HAL_UART_Receive(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    // #ifdef GETCHAR_ECHO_BACK_ENABLE
    // HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    // #endif

    // UART DMA circular buffer:
    // Retrieve the next available character in the UART RX circular buffer.
    // If there is no new data in rxBuffer, wait.
    // Known issue: Overruns cause rxBuffer to be dumped.
    // --> stdin_index interprets dma_index overtaking it as rxBuffer being empty of unread data.
    uint32_t dma_index;
    static uint32_t stdin_index = 0;
    do
    {
        dma_index = UART_RX_CIRCULAR_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    } while (dma_index == stdin_index);
    // while ((UART_RX_CIRCULAR_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx)) == stdin_index); // Equivalent statement as above.
    ch = rxBuffer[stdin_index];
#ifdef GETCHAR_ERASE_LAST_READ_BYTE_ENABLE
    rxBuffer[stdin_index] = 0;
#endif
#ifdef GETCHAR_LOOPBACK_ENABLE
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
#endif
    stdin_index = (stdin_index + 1) % UART_RX_CIRCULAR_BUFFER_SIZE;

    return ch;
}
#endif /*  STM32F4 */

// Defaults.
#define SERIAL_BAUDRATE ((uint32_t)115200)

void SystemClock_Config(void)
{
#ifdef STM32F4
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        ErrorHandler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        ErrorHandler();
    }
#endif
}

/** Serial_Init()
 *
 * Configure USB-UART2 @ 115200 baud.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
static int8_t Serial_Init(void)
{
#ifdef STM32F4
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Set up the UART2 bus.
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Initialize UART2 for serial communications over USB.
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        return ERROR;
    }

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE; // Disable for idle line?
    // hdma_usart2_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL; // not needed?
    // hdma_usart2_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    // hdma_usart2_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
        return ERROR;
    }

    __HAL_LINKDMA(&huart2, hdmarx, hdma_usart2_rx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t *)rxBuffer, UART_RX_CIRCULAR_BUFFER_SIZE) != HAL_OK)
    {
        return ERROR;
    }
    setvbuf(stdin, rxBuffer, _IONBF, UART_RX_CIRCULAR_BUFFER_SIZE); // No effect?
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
    return SUCCESS;
}

/** Nucleo_Init()
 *
 * Configure onboard Nucleo pins, i.e. the status LED and B^3 for usage.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
static int8_t Nucleo_Init(void)
{
#ifdef STM32F4
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    // __HAL_RCC_GPIOH_CLK_ENABLE(); // PH0 and PH1 are occupied by the HSE oscillator pins.

    // Nucleo user green LED "LD2"
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    // Set shield LEDs default off
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    // PC13 -> "USER" Nucleo blue button
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // PA5  -> Nucleo green status LED.
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Switches
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // LEDs (for default off)
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
#endif
    return SUCCESS;
}

/** BOARD_Init()
 *
 * 1. Initialize the HAL framework.
 * 2. Configure the clocks and pins used by our Nucleo kit's setup for serial
 * communications, onboard LED statuses, and resetting with the Big Blue Button.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t BOARD_Init()
{
    if (initStatus == FALSE)
    {
#ifdef STM32F4
        HAL_Init();
#endif /*  STM32F4 */
        SystemClock_Config();

        if (Nucleo_Init() == ERROR)
        {
            return ERROR;
        }
#ifdef STM32F4
        DMA_Init(); // Manages RX FIFO for UART 1 2 6
#endif              /*  STM32F4 */
        if (Serial_Init() == ERROR)
        {
            return ERROR;
        }
        initStatus = TRUE;
    }
    return SUCCESS;
}

/** BOARD_End()
 *
 * Shuts down all peripherals; turns all pins into input.
 *
 * @return  initStatus  (int8_t)    [TRUE, FALSE]
 *
 * @todo    Set all ports to be digital inputs.
 */
int8_t BOARD_End(void)
{
    initStatus = FALSE;
#ifdef STM32F4
    // Kill all interrupts.
    __disable_irq();
#endif /*  STM32F4 */

    return initStatus;
}

/** ErrorHandler()
 *
 * Enter a state of blinking the status LED on-board the Nucleo, indefinitely.
 */
void ErrorHandler(void)
{
#ifdef STM32F4
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(500);
    }
#endif /*  STM32F4 */
}
