// -------------------------------------------------------------------------- //
//! @file   msg_kineis_std.h
//! @brief  Argos message formatting algorithms defines
//! @author Kineis
//! @date   2020-04-01
// -------------------------------------------------------------------------- //

// -------------------------------------------------------------------------- //
//! * Argos message MSGKINEIS_STDV1 structure :
//!
//! | CRC  | AcqPeriod | Day | Hour | Minute | Longitude | Latitude | Altitude | User data |
//! |      |           |     |      |        |           |          |          |           |
//! |  16  |     3     |  5  |  5   |   6    |    22     |    21    |    10    |    160    |
//!
// -------------------------------------------------------------------------- //


// -------------------------------------------------------------------------- //
// Includes
// -------------------------------------------------------------------------- //
#include "msg_kineis_std.h"


// -------------------------------------------------------------------------- //
//! \brief Set one bit to 0 or 1 at the wanted position in the payload
//!
//! \param[out] *ArgosMsgHandle
//!	Argos message pointer
//! \param[in] value
//!	Value of the bit : 0 or 1
//! \param[in] position
//!	Position in bit in the Argos message payload
// -------------------------------------------------------------------------- //

void vMSGKINEIS_STDV1_setBit(
		ArgosMsgTypeDef_t *ArgosMsgHandle,
		bool value,
		uint16_t position)
{
	uint8_t byte_position = position >> 3;

	uint8_t bit_position = 7 - (position & 0x7);

	//!< Set to 0
	ArgosMsgHandle->payload[byte_position] &= ~(1 << bit_position);

	//!< Set value
	ArgosMsgHandle->payload[byte_position] |= (value << bit_position);
}


// -------------------------------------------------------------------------- //
//! \brief Set an uint32_t value at the wanted position in the payload
//!
//! \param[out] *ArgosMsgHandle
//!	Argos message pointer
//! \param[in] *value
//!	Pointer to the uint32_t value
//! \param[in] position
//!	Position in bit in the Argos message payload
//! \param[in] length
//!	Required length in the payload
//!
//! \return Last bit position
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setValue(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint32_t value,
	uint16_t position,
	uint16_t length)
{
	uint8_t j = 0;
	bool val_bit;

	position = position + length - 1;

	while (j < length) {
		if ((position-j+1)%8 == 0 && length-j > 7) {
			ArgosMsgHandle->payload[(position-j) >> 3] = (value >> j) & 0xff;
			j += 8;
		} else {
			val_bit = (value >> j) & 0x01;
			vMSGKINEIS_STDV1_setBit(ArgosMsgHandle, val_bit, position-j);
			j++;
		}
	}

	return position;
}


// -------------------------------------------------------------------------- //
// Add 'acqPeriod' to Argos message
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setAcqPeriod(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	enum PeriodAcqGPS_t acqPeriod,
	uint16_t position)
{
	if (ArgosMsgHandle == NULL)
		return 0xffff;

	//!< AcqPeriod : 3 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, acqPeriod, position, 3);

	//!< Last bit occupied
	return position;
}


// -------------------------------------------------------------------------- //
// Add date information to Argos message
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setDate(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint8_t day,
	uint8_t hour,
	uint8_t min,
	uint16_t position)
{
	if (ArgosMsgHandle == NULL)
		return 0xffff;

	//!< Day	: 5 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, day, position, 5);

	//!< Hour	: 5 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, hour, position+1, 5);

	//!< Min	: 6 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, min, position+1, 6);

	//!< Last bit occupied
	return position;
}


// -------------------------------------------------------------------------- //
// Add location information to Argos message
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setLocation(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	int32_t lon,
	int32_t lat,
	int16_t alt,
	uint16_t position)
{
	if (ArgosMsgHandle == NULL)
		return 0xffff;

	alt = ABS((alt + 500) / 10);

	if (lon < 0) {
		lon = ABS(lon);
		lon |= (1 << 21);
	}

	if (lat < 0) {
		lat = ABS(lat);
		lat |= (1 << 20);
	}

	//!< Longitude : 22 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, lon, position, 22);

	//!< Latitude	: 21 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, lat, position+1, 21);

	//!< Altitude	: 10 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, alt, position+1, 10);

	//!< Last bit occupied
	return position;
}


// -------------------------------------------------------------------------- //
// Add user data to Argos message
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setUserData(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint8_t data[],
	uint8_t len,
	uint16_t position)
{
	//User data	: 20 bytes
	uint8_t i;

	position--;

	if (ArgosMsgHandle == NULL)
		return 0xffff;

	if (len > 20)
		len = 20;

	for (i = 0; i < len; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, data[i], position + 1, 8);

	for (i = len; i < USER_DATA_LENGTH; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, position + 1, 8);

	//!< Last bit occupied
	return position;
}


// -------------------------------------------------------------------------- //
//! \brief Calculate the CRC16
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_calcCRC16(
		uint8_t *ptr,
		int16_t lengthBit)
{
	uint16_t remainder = 0;

	uint8_t leftShift = lengthBit % 8;
	uint8_t rightShift = 8 - leftShift;

	bool firstShift = true;

	while (lengthBit > 0) {
		if (leftShift == 0) {
			remainder ^= ((uint16_t)*ptr << (CRC16_WIDTH - 8));
		} else {
			if (firstShift) {
				firstShift = false;
				remainder ^= ((uint16_t)(*ptr >> rightShift) << (CRC16_WIDTH - 8));
			} else {
				remainder ^= (((uint16_t)(*(ptr-1) << leftShift) |
					(*(ptr) >> rightShift)) << (CRC16_WIDTH - 8));
			}
		}
		ptr++;

		for (uint8_t bit = 0; bit < 8; bit++) {
			if (remainder & CRC16_TOPBIT)
				remainder = (remainder << 1) ^ CRC16_POLYNOMIAL;
			else
				remainder = (remainder << 1);

			lengthBit--;
		}
	}
	return remainder;
}


// -------------------------------------------------------------------------- //
//! Set CRC16 in the MSGKINEIS_STDV1 payload
// -------------------------------------------------------------------------- //

void vMSGKINEIS_STDV1_setCRC16(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint16_t position)
{
	if (ArgosMsgHandle == NULL)
		return;

	uint16_t  crc;

	crc = u16MSGKINEIS_STDV1_calcCRC16(ArgosMsgHandle->payload + 2,
		ARGOS_FRAME_LENGTH_BIT - CRC16_WIDTH);

	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, crc, position, CRC16_WIDTH);
}


// -------------------------------------------------------------------------- //
//! Clean the entire MSGKINEIS_STDV1 payload
// -------------------------------------------------------------------------- //

void
vMSGKINEIS_STDV1_cleanPayload(
	ArgosMsgTypeDef_t *ArgosMsgHandle)
{
	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, 0, ARGOS_FRAME_LENGTH_BIT);
}


// -------------------------------------------------------------------------- //
//! Add CRC16 and BCH32 to the payload
// -------------------------------------------------------------------------- //
void vMSGKINEIS_STDV1_setCRC16andBCH32(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint16_t position)
{
	if (ArgosMsgHandle == NULL)
		return;

	uint32_t bch;
	uint16_t  crc;

	// Calculate the CRC till the first bit of the BCH instead of the end
	// of the payload
	crc = u16MSGKINEIS_STDV1_calcCRC16(ArgosMsgHandle->payload + 2,
		ARGOS_FRAME_LENGTH_BIT - CRC16_WIDTH - BCH32_WIDTH);

	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, crc, POSITION_STD_CRC, CRC16_WIDTH);
	
	bch = u32MSGKINEIS_STDV1_calcBCH32(ArgosMsgHandle->payload,
			ARGOS_FRAME_LENGTH_BIT - BCH32_WIDTH);

	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, bch, position, BCH32_WIDTH);
}
