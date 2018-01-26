#ifndef CHASSIS_COM_
#define CHASSIS_COM_

typedef struct chassis_pos_
{
    double lat;
    double lon;
    double heading;
}chassis_pos;


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
