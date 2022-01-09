// -------------------------------------------------------------------------- //
//! @file   msg_kineis_std.h
//! @brief  Argos message MSGKINEIS_STDV1 formatting algorithms defines
//!			As an example this format is used in kineis adventure.
//!			It contains date, GPS localisation, user data and CRC16
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

#ifndef MSG_KINEIS_STD_H

#define MSG_KINEIS_STD_H

#ifdef __cplusplus
extern "C" {
#endif


// -------------------------------------------------------------------------- //
// Includes
// -------------------------------------------------------------------------- //

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "msg_kineis_std_bch32.h"

#pragma GCC visibility push(default)

// -------------------------------------------------------------------------- //
// Defines values
// -------------------------------------------------------------------------- //
//!<
#define CRC16_WIDTH		16
#define CRC16_TOPBIT		(1UL << (CRC16_WIDTH - 1))
#define CRC16_POLYNOMIAL	0x11021UL

//!< Length of the 'user data' part (max 20 bytes)
#define USER_DATA_LENGTH	20

//!< length of an Argos frame containing GPS data and raw data (bytes)
#define ARGOS_FRAME_LENGTH	((11) + (USER_DATA_LENGTH))
#define ARGOS_FRAME_LENGTH_BIT	(ARGOS_FRAME_LENGTH*8)

//!< Position of the different parts in the payload (in bit)
#define POSITION_STD_CRC			0
#define POSITION_STD_ACQ_PERIOD		16
#define POSITION_STD_DATE			19
#define POSITION_STD_LOC			35
#define POSITION_STD_USER_DATA		88
#define POSITION_STD_BCH32			216


// -------------------------------------------------------------------------- //
// Defines functions
// -------------------------------------------------------------------------- //

//! Returns the absolute value of a number.
#define ABS(N) ((N) >= 0 ? (N) : -(N))


// -------------------------------------------------------------------------- //
//! Argos message MSGKINEIS_STDV1 payload
// -------------------------------------------------------------------------- //

typedef struct ArgosMsgTypeDef_t {
	uint8_t payload[ARGOS_FRAME_LENGTH];
} ArgosMsgTypeDef_t;


//! Time between two successive GPS positions
enum PeriodAcqGPS_t {
	PRD_1_MIN	= 0b000,
	PRD_5_MIN	= 0b001,
	PRD_10_MIN	= 0b010,
	PRD_15_MIN	= 0b011,
	PRD_30_MIN	= 0b100,
	PRD_60_MIN	= 0b101,
	LOC_INST	= 0b110,
	USER_MSG	= 0b111
};


// -------------------------------------------------------------------------- //
//! \brief Add 'acqPeriod' to Argos message MSGKINEIS_STDV1
//!
//! This function adds the acquisition period information to the Argos payload.
//!
//! \param[out] ArgosMsgHandle
//!		Argos message
//! \param[in] acqPeriod
//!		-- See PeriodAcqGPS_t
//! \param[in] position
//!		Position in bit in the Argos message payload
//!
//! \return Last occupied bit
// -------------------------------------------------------------------------- //

uint16_t
u16MSGKINEIS_STDV1_setAcqPeriod
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	enum PeriodAcqGPS_t acqPeriod,
	uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Add date information to Argos message MSGKINEIS_STDV1
//!
//! This function adds the given day, hour, minute information to the Argos
//!	payload.
//!
//! \param[out] ArgosMsgHandle
//!		Argos message
//! \param[in] day
//! \param[in] hour
//! \param[in] min
//! \param[in] position
//!		Position in bit in the Argos message payload
//!
//! \return Last occupied bit
// -------------------------------------------------------------------------- //

uint16_t
u16MSGKINEIS_STDV1_setDate
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint8_t day,
	uint8_t hour,
	uint8_t min,
	uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Add location information to Argos message MSGKINEIS_STDV1
//!
//! This function adds the given latitude, longitude and altitude information to
//!	the Argos payload.
//!
//! \param[out] ArgosMsgHandle
//!		Argos message
//! \param[in] lon
//!		1 bit: +"0"(Est)/-"1"(Ouest)
//!		21 bits: de 0 (0.0000°) à 1 800 000 (180.0000°)
//! \param[in] lat
//!		1 bit: +"0"(Nord)/-"1"(Sud)
//!		20 bits: de 0 (0.0000°) à  900 000 (90.0000°)
//!
//! \param[in] alt
//!		altitude (m) : from -500m to 9730m
//! \param[in] position
//!		Position in bit in the Argos message payload
//!
//! \return Last occupied bit
// -------------------------------------------------------------------------- //

uint16_t
u16MSGKINEIS_STDV1_setLocation
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	int32_t lon,
	int32_t lat,
	int16_t alt,
	uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Add user data to Argos message MSGKINEIS_STDV1
//!
//! This function adds day, hour, minutes information to the Argos payload.
//!
//! \param[out] ArgosMsgHandle
//!		Argos message pointer
//! \param[in] data
//!		data user array
//! \param[in] len
//!		length of the array (max 20)
//! \param[in] position
//!		Position in bit in the Argos message payload
//!
//! \return Last occupied bit
// -------------------------------------------------------------------------- //

uint16_t
u16MSGKINEIS_STDV1_setUserData
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint8_t data[],
	uint8_t len,
	uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Set CRC16 in the MSGKINEIS_STDV1 payload
//!
//! This function call ArgosMsg_calcCRC16 and adds the CRC to the payload.
//!	The message is ready for sending.
//!
//! \param[out] ArgosMsgHandle
//!		Argos message
//! \param[in] position
//!		Position of CRC bits in bit in the Argos message payload
// -------------------------------------------------------------------------- //

void
vMSGKINEIS_STDV1_setCRC16
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Set an uint32_t value at the wanted position in the payload
//!
//! \param[out] ArgosMsgHandle Argos message pointer
//! \param[in] value uint32_t value
//! \param[in] position Position in bit in the Argos message payload
//! \param[in] length Required length for this data in the payload
//!
//! \return Last occupied bit
// -------------------------------------------------------------------------- //

uint16_t
u16MSGKINEIS_STDV1_setValue
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint32_t value,
	uint16_t position,
	uint16_t length
);


// -------------------------------------------------------------------------- //
//! \brief Set one bit to 0 or 1 at the wanted position in the payload
//!
//! \param[out] ArgosMsgHandle Argos message pointer
//! \param[in] value Value of the bit : 0 or 1
//! \param[in] position Position in bit in the Argos message payload
// -------------------------------------------------------------------------- //

void
vMSGKINEIS_STDV1_setBit
(
		ArgosMsgTypeDef_t *ArgosMsgHandle,
		bool value,
		uint16_t position
);


// -------------------------------------------------------------------------- //
//! \brief Clean the entire MSGKINEIS_STDV1 payload
//!
//! This function set the payload to zero.
//!
//! \param[out] ArgosMsgHandle
//!	Argos message
// -------------------------------------------------------------------------- //

void
vMSGKINEIS_STDV1_cleanPayload
(
	ArgosMsgTypeDef_t *ArgosMsgHandle
);


// -------------------------------------------------------------------------- //
//! \brief Calculate the CRC16
//!
//! This function calculates the CRC16 from the MSB of the pointed byte by ptr
//!	to the last bit : ptr+lengthBit
//!	\note You can find details here : https://barrgroup.com/embedded-systems/how-to/crc-calculation-c-code
//!
//! \param[in] ptr
//!		Pointer of the first byte
//! \param[in] lengthBit
//!		Length of the data in bit
//!
//! \return CRC value
// -------------------------------------------------------------------------- //

uint16_t u16MSGKINEIS_STDV1_calcCRC16
(
	uint8_t *ptr,
	int16_t lengthBit
);


// -------------------------------------------------------------------------- //
//! \brief Set CRC16 and BCH32 in the payload
//!
//! This function calculates the BCH32 and the CRC.
//!	The CRC is put on the firt 16 bits of the payload.
//!	The BCH32 is put on the last 32 bits of the payload.
//!
//! \param[in,out] ArgosMsgHandle Argos message
//! \param[in] position Position in bit of BCH32 in the Argos message payload
//!
// -------------------------------------------------------------------------- //

void vMSGKINEIS_STDV1_setCRC16andBCH32
(
	ArgosMsgTypeDef_t *ArgosMsgHandle,
	uint16_t position
);

#pragma GCC visibility pop

#ifdef __cplusplus
}
#endif

#endif // end MSG_KINEIS_STD_H
