/**
 * @file    msg_kineis_utils.h
 * @brief   Utils to calculate CRC, BCH and FCS.
 * @author  KINEIS
 * @date    creation 2021/04/22
 */
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef MSG_KINEIS_UTILS_H
#define MSG_KINEIS_UTILS_H

/**
 * @addtogroup MSG_KINEIS_UTILS
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Defines -------------------------------------------------------------------*/

/** CRC16 polynomial = (x16 + x12 + x5 + 1) */
#define CRC16_POLYNOMIAL		0x1021UL

/** BCH32 polynomial = (x32 + x31 + x30 + x29 + x27 + x26 + x25 + x22 + x20 + x19 + x17 + x16 +
 * x14 + x9 + x7 + x6 + x5 + x4 + x3 + x2 + 1) */
#define BCH32_POLYNOMIAL		0xEE5B42FDUL

/** FCS32 polynomial =
 * (x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1) */
#define FCS32_POLYNOMIAL		0x04C11DB7UL

/** WARNING :
 *	The most significant bit of the polynomial has been discarded as it is useless for
 *	the calculation. See https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 */

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Calculate the BCH32
 *
 * This function calculates the CRC32 using "0x1EE5B42FD" polynomial from the MSB of the pointed
 * byte by ptr to the last bit (ptr+lengthBit)
 *
 * @note You can find details here:
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @param[in] ptr: Pointer of the first byte
 * @param[in] lengthBit: Length of the data in bit
 *
 * @return BCH32 value
 */
#define u32MSG_KINEIS_UTILS_calcBCH32(ptr, lengthBit) \
			u32MSG_KINEIS_UTILS_calcCrcBch32(ptr, lengthBit, (uint32_t)BCH32_POLYNOMIAL)

/**
 * @brief Calculate the FCS32
 *
 * This function calculates the CRC32 using "0x104C11DB7" polynomial from the MSB of the pointed
 * byte by ptr to the last bit (ptr+lengthBit)
 *
 * @note You can find details here:
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @param[in] ptr: Pointer of the first byte
 * @param[in] lengthBit: Length of the data in bit
 *
 * @return FCS32 value
 */
#define u32MSG_KINEIS_UTILS_calcFCS32(ptr, lengthBit) \
			u32MSG_KINEIS_UTILS_calcCrcBch32(ptr, lengthBit, (uint32_t)FCS32_POLYNOMIAL)

/**
 * @brief Calculate the CRC16 depending on polynomial value.
 *
 * This function calculates the CRC16 from the MSB of the pointed byte by ptr
 * to the last bit (ptr+lengthBit)
 *
 * @note You can find details here:
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @param[in] ptr: Pointer of the first byte
 * @param[in] lengthBit: Length of the data in bit
 *
 * @return CRC16 value
 */
#define u16MSG_KINEIS_UTILS_calcCRC16(ptr, lengthBit) \
			u16MSG_KINEIS_UTILS_calcCrcBch16(ptr, lengthBit, (uint16_t)CRC16_POLYNOMIAL)

/**
 * @brief Calculate the CRC32, BCH32 or FCS32 depending on polynomial value.
 *
 * This function calculates the CRC32, BCH32 or FCS32 from the MSB of the pointed byte by ptr
 * to the last bit (ptr+lengthBit)
 *
 * @note You can find details here:
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @param[in] ptr: Pointer of the first byte
 * @param[in] lengthBit: Length of the data in bit
 * @param[in] u32Polynomial: Polynomial used to calculate CRC, BCH or FCS value
 *
 * @return CRC32, BCH32 or FCS32 value
 */
uint32_t u32MSG_KINEIS_UTILS_calcCrcBch32(
		uint8_t *ptr,
		int16_t lengthBit,
		uint32_t u32Polynomial);

/**
 * @brief Calculate the CRC16 or BCH16 depending on polynomial value.
 *
 * This function calculates the CRC16 or BCH16 from the MSB of the pointed byte by ptr
 * to the last bit (ptr+lengthBit)
 *
 * @note You can find details here:
 * https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
 *
 * @param[in] ptr: Pointer of the first byte
 * @param[in] lengthBit: Length of the data in bit
 * @param[in] u16Polynomial: Polynomial used to calculate CRC or BCH value
 *
 * @return CRC16 or BCH16 value
 */
uint16_t u16MSG_KINEIS_UTILS_calcCrcBch16(
		uint8_t *ptr,
		int16_t lengthBit,
		uint16_t u16Polynomial);

/**
 * @}
 */

#endif /* end MSG_KINEIS_UTILS_H */
#ifdef __cplusplus
}
#endif
