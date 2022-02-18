/**
 * @file    msg_kineis_utils.c
 * @brief   Utils to calculate CRC, BCH and FCS.
 * @author  KINEIS
 * @date    creation 2021/04/22
 */

/**
 * @addtogroup MSG_KINEIS_UTILS
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "msg_kineis_utils.h"

/* Private define ------------------------------------------------------------*/
#define REMAINDER32_WIDTH		32
#define REMAINDER32_TOPBIT		(1UL << (REMAINDER32_WIDTH - 1))

#define REMAINDER16_WIDTH		16
#define REMAINDER16_TOPBIT		(1UL << (REMAINDER16_WIDTH - 1))


/* Functions -----------------------------------------------------------------*/

uint32_t u32MSG_KINEIS_UTILS_calcCrcBch32(
		uint8_t *ptr,
		int16_t lengthBit,
		uint32_t u32Polynomial)
{
	uint32_t remainder = 0;

	uint8_t leftShift = lengthBit % 8;
	uint8_t rightShift = 8 - leftShift;

	bool firstShift = true;

	while (lengthBit > 0) {
		if (leftShift == 0) {
			remainder ^= ((uint32_t)*ptr << (REMAINDER32_WIDTH - 8));
		} else {
			if (firstShift) {
				firstShift = false;
				remainder ^= ((uint32_t)(*ptr >> rightShift) << (REMAINDER32_WIDTH - 8));
			} else {
				remainder ^= (((uint32_t)(*(ptr-1) << leftShift) |
					(*(ptr) >> rightShift)) << (REMAINDER32_WIDTH - 8));
			}
		}
		ptr++;

		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & REMAINDER32_TOPBIT)
				remainder = (remainder << 1) ^ u32Polynomial;
			else
				remainder = (remainder << 1);

			lengthBit--;
		}
	}
	return remainder;
}

uint16_t u16MSG_KINEIS_UTILS_calcCrcBch16(
		uint8_t *ptr,
		int16_t lengthBit,
		uint16_t u16Polynomial)
{
	uint16_t remainder = 0;

	uint8_t leftShift = lengthBit % 8;
	uint8_t rightShift = 8 - leftShift;

	bool firstShift = true;

	while (lengthBit > 0) {
		if (leftShift == 0) {
			remainder ^= ((uint16_t)*ptr << (REMAINDER16_WIDTH - 8));
		} else {
			if (firstShift) {
				firstShift = false;
				remainder ^= ((uint16_t)(*ptr >> rightShift) << (REMAINDER16_WIDTH - 8));
			} else {
				remainder ^= (((uint16_t)(*(ptr-1) << leftShift) |
					(*(ptr) >> rightShift)) << (REMAINDER16_WIDTH - 8));
			}
		}
		ptr++;

		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & REMAINDER16_TOPBIT)
				remainder = (remainder << 1) ^ u16Polynomial;
			else
				remainder = (remainder << 1);

			lengthBit--;
		}
	}
	return remainder;
}

/**
 * @}
 */
