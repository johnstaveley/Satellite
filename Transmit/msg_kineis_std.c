// -------------------------------------------------------------------------- //
//! @file	msg_kineis_std.c
//! @brief	Kineis message MSGKINEIS_STDV1 formatting algorithms defines
//!			This library can be used for these formats :
//!				- Position and user data
//!				- User data only
//! @note	It is recommended to insert an integrity check in the form of a
//!			CRC16 and BCH32 inserted at the beginning and at the end of the
//!			message.
//!			In order to ensure this, use vMSGKINEIS_STDV1_setCRC16andBCH32 or
//!			vMSGKINEIS_STDV1_setCRC16 functions.
//! @author	Kineis
//! @date	2020-04-01
// -------------------------------------------------------------------------- //


// -------------------------------------------------------------------------- //
//! * Kineis message structures :
//! With position :
//! | Ext ID | CRC  | AcqPeriod | Day | Hour | Minute | Longitude | Latitude | Altitude | User data | BCH* |
//! |        |      |           |     |      |        |           |          |          |           |      |
//! |    4   |  16  |     3     |  5  |   5  |    6   |     22    |    21    |    10    |    124    |  32  |
//!
//! User data only :
//! | Ext ID | CRC  |                                   User data                                   | BCH* |
//! |        |      |                                                                               |      |
//! |    4   |  16  |                                      196                                      |  32  |
//!
//! *optional : If BCH is not used, the "User data" field is 32 bits larger.
// -------------------------------------------------------------------------- //


// -------------------------------------------------------------------------- //
// Includes
// -------------------------------------------------------------------------- //
#include "msg_kineis_std.h"
#include "msg_kineis_utils.h"

// -------------------------------------------------------------------------- //
// Defines
// -------------------------------------------------------------------------- //
#define BCH32_WIDTH		32
#define CRC16_WIDTH		16

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
		if ((position - j + 1) % 8 == 0 && length - j > 7) {
			ArgosMsgHandle->payload[(position - j) >> 3] = (value >> j) & 0xff;
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
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, hour, position + 1, 5);

	//!< Min	: 6 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, min, position + 1, 6);

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
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, lat, position + 1, 21);

	//!< Altitude	: 10 bits
	position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, alt, position + 1, 10);

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
	//User data	: 19.5 bytes
	uint8_t i;

	position--;

	if (ArgosMsgHandle == NULL)
		return 0xffff;

	if (len > USER_DATA_LENGTH)
		len = USER_DATA_LENGTH;

	for (i = 0; i < len && i < (USER_DATA_LENGTH) - 1; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, data[i], position + 1, 8);

	for (i = len; i < (USER_DATA_LENGTH) - 1; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, position + 1, 8);

	if (len == USER_DATA_LENGTH)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, data[len - 1] >> 4, position + 1, 4);
	else
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, position + 1, 4);

	//!< Last bit occupied
	return position;
}


// -------------------------------------------------------------------------- //
// Add user data ONLY to Argos message
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_setUserDataOnly(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint8_t data[],
	uint8_t len,
	uint16_t position)
{
	//User data	ONLY : 28.5 bytes
	uint8_t i;

	position--;

	if (ArgosMsgHandle == NULL)
		return 0xffff;

	if (len > USER_DATA_ONLY_LENGTH)
		len = USER_DATA_ONLY_LENGTH;

	for (i = 0; i < len && i < (USER_DATA_ONLY_LENGTH) - 1; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, data[i], position + 1, 8);

	for (i = len; i < (USER_DATA_ONLY_LENGTH) - 1; i++)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, position + 1, 8);

	if (len == USER_DATA_ONLY_LENGTH)
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, data[len - 1] >> 4, position + 1, 4);
	else
		position = u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, 0, position + 1, 4);

	//!< Last bit occupied
	return position;
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

	uint16_t crc;

	crc = u16MSG_KINEIS_UTILS_calcCRC16(ArgosMsgHandle->payload + 2,
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
	uint16_t crc;

	// Calculate the CRC till the first bit of the BCH instead of the end
	// of the payload
	crc = u16MSG_KINEIS_UTILS_calcCRC16(ArgosMsgHandle->payload + 2,
		ARGOS_FRAME_LENGTH_BIT - CRC16_WIDTH - BCH32_WIDTH);

	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, crc, POSITION_STD_CRC, CRC16_WIDTH);
	
	bch = u32MSG_KINEIS_UTILS_calcBCH32(ArgosMsgHandle->payload,
			ARGOS_FRAME_LENGTH_BIT - BCH32_WIDTH);

	u16MSGKINEIS_STDV1_setValue(ArgosMsgHandle, bch, position, BCH32_WIDTH);
}
