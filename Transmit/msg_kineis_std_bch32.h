// -------------------------------------------------------------------------- //
//! @file   msg_kineis_std_bch32.h
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

#ifndef MSG_KINEIS_STD_BCH32_H
#define MSG_KINEIS_STD_BCH32_H


#include <stdint.h>
#include <stdbool.h>


#define BCH32_WIDTH			32
#define BCH32_TOPBIT		(1UL << (BCH32_WIDTH - 1))
#define BCH32_POLYNOMIAL	0x1EE5B42FDUL

// -------------------------------------------------------------------------- //
//! \brief Calculate the BCH32
//!
//! This function calculates the BCH32 from the MSB of the pointed byte by ptr
//!	to the last bit : ptr+lengthBit
//! \note You can find details here : https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
//!
//! \param[in] ptr Pointer
//! \param[in] lengthBit Length of the array in byte
//!
//! \return BCH32 value
// -------------------------------------------------------------------------- //

uint32_t u32MSGKINEIS_STDV1_calcBCH32(uint8_t *ptr, int16_t lengthBit);

#endif
