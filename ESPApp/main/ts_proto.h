#ifndef TS_PROTO_H
#define TS_PROTO_H

#include <inttypes.h>

#define tsProto_MSG_DATA_LEN 30

#define tsProto_Version (uint8_t) 0xEF

#pragma pack (1)

typedef enum t_tsProtoCmds {
    timeSyncReq = 1,
    timeSyncResponse,
    dataIn,
    dataOut,
} __attribute__((packed)) tsProtoCmds_t;



typedef struct t_tsTime
{
    uint64_t tv_sec;
    uint64_t tv_usec;
} __attribute__((packed)) tsTime_t;




typedef struct t_tsMsg
{
    uint8_t version;
	tsTime_t timestamp;
    uint8_t cmd;
    uint8_t data[tsProto_MSG_DATA_LEN];
    uint8_t sign;
} __attribute__((packed)) tsMsg_t;

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

tsTime_t get_ts_time();

uint32_t get_ts_delta_time(tsTime_t *greater, tsTime_t *lesser);

uint8_t sign_msg(tsMsg_t *msg);

void prepare_msg(tsMsg_t *msg);

tsMsg_t *parse_raw_data(uint8_t *data);


#ifdef __cplusplus
}
#endif


#endif // TS_PROTO_H
