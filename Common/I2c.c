/**
 * @file    I2c.c
 *
 * @author  HARE Lab
 * @author  jLab
 * @author  Adam Korycki
 * @author  Max Dunne
 * @version 1.0.0
 *
 * @date    30 Jul 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef STM32F4
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#endif /*  STM32F4 */
#include "I2c.h"

// Boolean defines for TRUE, FALSE, SUCCESS and ERROR
#ifndef FALSE
#define FALSE ((int8_t)0)
#define TRUE ((int8_t)1)
#endif
#ifndef ERROR
#define ERROR ((int8_t)-1)
#endif /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t)1)
#endif /*  SUCCESS */

#ifdef STM32F4
// I2C handler.
I2C_HandleTypeDef hi2c2;
// DMA handler.
DMA_HandleTypeDef hdma_i2c2_tx;
#endif /*  STM32F4 */

static uint8_t init_status = FALSE;

// /** MX_DMA_Init()
//  *
//  */
// static void MX_DMA_Init(void)
// {
//     __HAL_RCC_DMA1_CLK_ENABLE();

//     HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
//     HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
// }

/**
 * @Function I2C_Init(Rate)
 * @param None
 * @return SUCCESS or ERROR
 * @brief  Initializes the I2C System at standard speed (100Kbps)
 * @author Adam Korycki, 2023.10.03 */
int8_t I2C_Init(void)
{
    if (init_status == FALSE)
    { // if I2C interface has not been initialized
#ifdef STM32F4
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C2 GPIO Configuration
        PB10     ------> I2C2_SCL
        PB9     ------> I2C2_SDA
        */
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* I2C2 clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();

        // MX_DMA_Init();
        hi2c2.Instance = I2C2;
        hi2c2.Init.ClockSpeed = 400000;
        hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
        hi2c2.Init.OwnAddress1 = 0;
        hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hi2c2.Init.OwnAddress2 = 0;
        hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
        if (HAL_I2C_Init(&hi2c2) != HAL_OK)
        {
            return ERROR;
        }
#endif /*  STM32F4 */
        init_status = TRUE;
    }
    return SUCCESS;
}

/**
 * @Function I2C_ReadRegister(unsigned char I2CAddress,unsigned char deviceRegisterAddress)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit address of register on device
 * @return Value at Register or 0
 * @brief  Reads one device register on chosen I2C device
 * @author Adam Korycki, 2023.10.03 */
unsigned char I2C_ReadRegister(unsigned char I2CAddress, unsigned char deviceRegisterAddress)
{
#ifdef STM32F4
    HAL_StatusTypeDef ret;
#endif                            /*  STM32F4 */
    I2CAddress = I2CAddress << 1; // use 8-bit address
    uint8_t *data = &deviceRegisterAddress;

#ifdef STM32F4
    // start condition
    ret = HAL_I2C_Master_Transmit(&hi2c2, I2CAddress, data, 1, HAL_MAX_DELAY); // wait for it to end, this is internal and can not stall
    if (ret != HAL_OK)
    {
        printf("I2C Tx Error on read start condition\r\n");
        return 0;
    }

    // get byte
    ret = HAL_I2C_Master_Receive(&hi2c2, I2CAddress, data, 1, HAL_MAX_DELAY); // wait for it to end, this is internal and can not stall
    if (ret != HAL_OK)
    {
        printf("I2C Rx Error on read byte\r\n");
        return 0;
    }
#endif /*  STM32F4 */

    return *data;
}

/**
 * @Function I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, char data)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit address of register on device
 * @param data - data wished to be written to register
 * @return 0 if error and 1 if success
 * @brief  Writes one device register on chosen I2C device
 * @author Adam Korycki, 2023.10.03 */
unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, uint8_t data)
{
#ifdef STM32F4
    HAL_StatusTypeDef ret;
#endif                            /*  STM32F4 */
    I2CAddress = I2CAddress << 1; // use 8-bit address

#ifdef STM32F4
    ret = HAL_I2C_Mem_Write(&hi2c2, I2CAddress, deviceRegisterAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK)
    {
        printf("I2C Tx Error on write data\r\n");
        return ERROR;
    }
#endif /*  STM32F4 */

    return SUCCESS;
}

/**
 * @Function I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char isBigEndian)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit lower address of register on device
 * @param isBigEndian - Boolean determining if device is big or little endian
 * @return ERROR or SUCCESS
 * @brief  Reads two sequential registers to build a 16-bit value. isBigEndian
 * whether the first bits are either the high or low bits of the value
 * @author Adam Korycki, 2023.10.04*/
int I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char isBigEndian)
{
    short data = 0; // 16-bit return value

#ifdef STM32F4
    unsigned char byte = I2C_ReadRegister(I2CAddress, deviceRegisterAddress); // get first byte
    if (isBigEndian)
    {
        data = byte << 8;
    }
    else
    {
        data = byte;
    }

    byte = I2C_ReadRegister(I2CAddress, deviceRegisterAddress + 1); // get second byte
    if (isBigEndian)
    {
        data |= byte;
    }
    else
    {
        data |= byte << 8;
    }
#endif /*  STM32F4 */
    return data;
}

void DMA1_STREAM7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_i2c2_tx);
}
