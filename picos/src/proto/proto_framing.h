#ifndef _PROTO_FRAMING_H_
#define _PROTO_FRAMING_H_

#include <stdint.h>
#include <stddef.h>

int frame_bytes(const uint8_t *data, size_t len, uint8_t *out, size_t out_len);

const uint8_t* unframe_bytes(const uint8_t *data, size_t len, size_t *out_len);

#endif
