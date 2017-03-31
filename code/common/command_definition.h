#ifndef __COMMAND_DEFINITION_H__
#define __COMMAND_DEFINITION_H__

#include <stdint.h>

#define CD_MAXIMUM_SIZE 65535
#define CD_HEADER_SIZE 	sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint16_t)
#define CD_PAYLOAD_MTU  CD_MAXIMUM_SIZE - CD_HEADER_SIZE
/* Packets are bounded to 65 KBytes bytes */

#define CD_HELLO 				0
#define CD_START				1
#define CD_STOP					2	
#define CD_STATUS				3
#define CD_SET 					4
#define CD_GET 					5
#define CD_SET_SAT_TLE 			6
#define CD_SET_GPS_TLE 			7
#define CD_SET_GAL_TLE			8
#define CD_SET_MANAGER_CONF		9
#define CD_SET_GNSS_CONF		10
#define CD_GET_GNSS				11
#define CD_GET_RAD				12
#define CD_GET_AIS				13

typedef union __attribute__ ((__packed__)) command_def_u{
	uint8_t 	raw[CD_MAXIMUM_SIZE];
	struct __attribute__ ((__packed__)){
		/* Command ID uint8_t sized */
		uint32_t 	timestamp;
		uint8_t 	command_id;
		/* Length of the Payload */
		/* Length can be 0 */
		uint16_t 	len;
		uint8_t 	payload[CD_PAYLOAD_MTU];
	}fields;
}command_def_t;

#define FIL_HEADER_SIZE CD_PAYLOAD_MTU - 256 - 2

typedef union __attribute__ ((__packed__)) file_command_u{
	uint8_t 	raw[CD_PAYLOAD_MTU];
	struct __attribute__ ((__packed__)){
		/* Command ID uint8_t sized */
		char 		file_path[256];
		uint16_t    file_length;
		uint8_t		file_contents[CD_PAYLOAD_MTU - 256];
	}fields;
}file_command_t;

#endif
