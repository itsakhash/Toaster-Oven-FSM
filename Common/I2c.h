/*
 * File:   I2C.h
 * Author: Adam Korycki
 *
 * Created on October 3, 2023
 */

#ifndef I2C_H
#define	I2C_H

/**
 * @Function I2C_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief  Initializes the I2C System at standard speed (1Kbps)
 * @author Adam Korycki, 2023.10.03 */
extern int8_t I2C_Init(void);


/**
 * @Function I2C_ReadRegister(unsigned char I2CAddress,unsigned char deviceRegisterAddress)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit address of register on device
 * @return Value at Register or 0
 * @brief  Reads one device register on chosen I2C device
 * @author Adam Korycki, 2023.10.03 */
extern unsigned char I2C_ReadRegister(unsigned char I2CAddress,unsigned char deviceRegisterAddress);


/**
 * @Function I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, char data)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit address of register on device
 * @param data - data wished to be written to register
 * @return ERROR or SUCCESS
 * @brief  Writes one device register on chosen I2C device
 * @author Adam Korycki, 2023.10.03 */
extern unsigned char I2C_WriteReg(unsigned char I2CAddress, unsigned char deviceRegisterAddress, uint8_t data);


/**
 * @Function I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char isBigEndian)
 * @param I2CAddresss - 7-bit address of I2C device wished to interact with
 * @param deviceRegisterAddress - 8-bit lower address of register on device
 * @param isBigEndian - Boolean determining if device is big or little endian
 * @return ERROR or SUCCESS
 * @brief  Reads two sequential registers to build a 16-bit value. isBigEndian
 * whether the first bits are either the high or low bits of the value
 * @author Adam Korycki, 2023.10.04*/
extern int I2C_ReadInt(char I2CAddress, char deviceRegisterAddress, char isBigEndian);



#endif
