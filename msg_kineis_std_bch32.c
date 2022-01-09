// -------------------------------------------------------------------------- //
//! @file   msg_kineis_std_bch32.c
//! @brief  BCH32 calculation function
//! @author Kineis
//! @date   2020-08-05
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//! * Argos message MSGKINEIS_STDV1 structure with BCH32:
//!
//! | CRC  | AcqPeriod | Day | Hour | Minute | Longitude | Latitude | Altitude | User data |  BCH  |
//! |      |           |     |      |        |           |          |          |           |	   |
//! |  16  |     3     |  5  |  5   |   6    |    22     |    21    |    10    |    128    |   32  |
//!
// -------------------------------------------------------------------------- //

#include "msg_kineis_std_bch32.h"


// -------------------------------------------------------------------------- //
//! \brief Calculate the BCH32
// -------------------------------------------------------------------------- //

uint32_t
u32MSGKINEIS_STDV1_calcBCH32(
		uint8_t *ptr,
		int16_t lengthBit)
{
	uint32_t remainder = 0;

	uint8_t leftShift = lengthBit % 8;
	uint8_t rightShift = 8 - leftShift;

	bool firstShift = true;

	while (lengthBit > 0) {
		if (leftShift == 0) {
			remainder ^= ((uint32_t)*ptr << (BCH32_WIDTH - 8));
		} else {
			if (firstShift) {
				firstShift = false;
				remainder ^= ((uint32_t)(*ptr >> rightShift) << (BCH32_WIDTH - 8));
			} else {
				remainder ^= (((uint32_t)(*(ptr-1) << leftShift) |
					(*(ptr) >> rightShift)) << (BCH32_WIDTH - 8));
			}
		}
		ptr++;

		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & BCH32_TOPBIT)
				remainder = (remainder << 1) ^ BCH32_POLYNOMIAL;
			else
				remainder = (remainder << 1);

			lengthBit--;
		}
	}
	return remainder;
}
