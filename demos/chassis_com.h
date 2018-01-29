#ifndef CHASSIS_COM_
#define CHASSIS_COM_

typedef struct gps_2d_
{
    double lat;
    double lon;
}gps_2d;

typedef struct chassis_pos_
{
    double lat;
    double lon;
    double heading;
}chassis_pos;

typedef enum pi_chassis_status_type_
{
	PI_CHASSIS_STATUS_TYPE_DUMMY,
	PI_CHASSIS_STATUS_TYPE_POS,
	PI_CHASSIS_STATUS_TYPE_PATH
}pi_chassis_status_type;

typedef struct pi_chassis_status_common_
{
	// type of the status info uploaded by chassis
	int type;

	// total size of elements packaged in data block.
	int length;

	// placeholder for the start address of actual status info.
	void *data;
}pi_chassis_status_common;

typedef enum pi_command_type_
{
	PI_COMMAND_NULL,
	PI_COMMAND_SET_TARGET
}pi_command_type;

typedef struct pi_system_command_
{
	pi_command_type type;

	// command body
	unsigned char body[64];
}pi_system_command;

#endif
