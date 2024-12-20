/* Automatically generated nanopb header */
/* Generated by nanopb-1.0.0-dev */

#ifndef PB_NONNA_PROTO_NONNA_PB_H_INCLUDED
#define PB_NONNA_PROTO_NONNA_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _nonna_proto_MotorCmd {
    int32_t left;
    int32_t right;
    bool idle;
} nonna_proto_MotorCmd;

typedef struct _nonna_proto_SensorData {
    uint32_t sensors[24];
} nonna_proto_SensorData;

typedef struct _nonna_proto_EnableCmd {
    char dummy_field;
} nonna_proto_EnableCmd;

typedef struct _nonna_proto_DisableCmd {
    char dummy_field;
} nonna_proto_DisableCmd;

typedef struct _nonna_proto_NonnaMsg {
    pb_size_t which_payload;
    union _nonna_proto_NonnaMsg_payload {
        nonna_proto_MotorCmd motor_cmd;
        nonna_proto_SensorData sensor_data;
        nonna_proto_EnableCmd enable_cmd;
        nonna_proto_DisableCmd disable_cmd;
    } payload;
} nonna_proto_NonnaMsg;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define nonna_proto_MotorCmd_init_default        {0, 0, 0}
#define nonna_proto_SensorData_init_default      {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define nonna_proto_EnableCmd_init_default       {0}
#define nonna_proto_DisableCmd_init_default      {0}
#define nonna_proto_NonnaMsg_init_default        {0, {nonna_proto_MotorCmd_init_default}}
#define nonna_proto_MotorCmd_init_zero           {0, 0, 0}
#define nonna_proto_SensorData_init_zero         {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define nonna_proto_EnableCmd_init_zero          {0}
#define nonna_proto_DisableCmd_init_zero         {0}
#define nonna_proto_NonnaMsg_init_zero           {0, {nonna_proto_MotorCmd_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define nonna_proto_MotorCmd_left_tag            1
#define nonna_proto_MotorCmd_right_tag           2
#define nonna_proto_MotorCmd_idle_tag            3
#define nonna_proto_SensorData_sensors_tag       1
#define nonna_proto_NonnaMsg_motor_cmd_tag       1
#define nonna_proto_NonnaMsg_sensor_data_tag     2
#define nonna_proto_NonnaMsg_enable_cmd_tag      3
#define nonna_proto_NonnaMsg_disable_cmd_tag     4

/* Struct field encoding specification for nanopb */
#define nonna_proto_MotorCmd_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, SINT32,   left,              1) \
X(a, STATIC,   SINGULAR, SINT32,   right,             2) \
X(a, STATIC,   SINGULAR, BOOL,     idle,              3)
#define nonna_proto_MotorCmd_CALLBACK NULL
#define nonna_proto_MotorCmd_DEFAULT NULL

#define nonna_proto_SensorData_FIELDLIST(X, a) \
X(a, STATIC,   FIXARRAY, UINT32,   sensors,           1)
#define nonna_proto_SensorData_CALLBACK NULL
#define nonna_proto_SensorData_DEFAULT NULL

#define nonna_proto_EnableCmd_FIELDLIST(X, a) \

#define nonna_proto_EnableCmd_CALLBACK NULL
#define nonna_proto_EnableCmd_DEFAULT NULL

#define nonna_proto_DisableCmd_FIELDLIST(X, a) \

#define nonna_proto_DisableCmd_CALLBACK NULL
#define nonna_proto_DisableCmd_DEFAULT NULL

#define nonna_proto_NonnaMsg_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,motor_cmd,payload.motor_cmd),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,sensor_data,payload.sensor_data),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,enable_cmd,payload.enable_cmd),   3) \
X(a, STATIC,   ONEOF,    MESSAGE,  (payload,disable_cmd,payload.disable_cmd),   4)
#define nonna_proto_NonnaMsg_CALLBACK NULL
#define nonna_proto_NonnaMsg_DEFAULT NULL
#define nonna_proto_NonnaMsg_payload_motor_cmd_MSGTYPE nonna_proto_MotorCmd
#define nonna_proto_NonnaMsg_payload_sensor_data_MSGTYPE nonna_proto_SensorData
#define nonna_proto_NonnaMsg_payload_enable_cmd_MSGTYPE nonna_proto_EnableCmd
#define nonna_proto_NonnaMsg_payload_disable_cmd_MSGTYPE nonna_proto_DisableCmd

extern const pb_msgdesc_t nonna_proto_MotorCmd_msg;
extern const pb_msgdesc_t nonna_proto_SensorData_msg;
extern const pb_msgdesc_t nonna_proto_EnableCmd_msg;
extern const pb_msgdesc_t nonna_proto_DisableCmd_msg;
extern const pb_msgdesc_t nonna_proto_NonnaMsg_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define nonna_proto_MotorCmd_fields &nonna_proto_MotorCmd_msg
#define nonna_proto_SensorData_fields &nonna_proto_SensorData_msg
#define nonna_proto_EnableCmd_fields &nonna_proto_EnableCmd_msg
#define nonna_proto_DisableCmd_fields &nonna_proto_DisableCmd_msg
#define nonna_proto_NonnaMsg_fields &nonna_proto_NonnaMsg_msg

/* Maximum encoded size of messages (where known) */
#define NONNA_PROTO_NONNA_PB_H_MAX_SIZE          nonna_proto_NonnaMsg_size
#define nonna_proto_DisableCmd_size              0
#define nonna_proto_EnableCmd_size               0
#define nonna_proto_MotorCmd_size                14
#define nonna_proto_NonnaMsg_size                147
#define nonna_proto_SensorData_size              144

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
