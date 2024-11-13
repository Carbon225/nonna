#ifndef _PROTO_FRAMING_H_
#define _PROTO_FRAMING_H_

#include <stdint.h>
#include <stddef.h>

#define MAX_PACKET_LEN 256
#define MAX_PAYLOAD_LEN (MAX_PACKET_LEN - 3)

static const uint8_t FRAME_START = 0x7E;

int frame_bytes(const uint8_t *data, size_t len, uint8_t *out, size_t out_len);

const uint8_t* unframe_bytes(const uint8_t *data, size_t len);

#endif
