#include "proto_framing.h"

#include <string.h>

static const uint8_t FRAME_START = 0x7E;

static uint8_t calc_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0x00;
    while (len--) {
        uint8_t extract = *data++;
        for (uint8_t tempI = 8; tempI; tempI--) {
            uint8_t sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum) {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }
    return crc;
}

int frame_bytes(const uint8_t *data, size_t len, uint8_t *out, size_t out_len)
{
    if (len == 0)
    {
        return 0;
    }

    // Frame start, length, data, crc
    if (1 + 1 + len + 1 > out_len)
    {
        return -1;
    }

    out[0] = FRAME_START;
    out[1] = len;
    memcpy(out + 2, data, len);
    out[len + 2] = calc_crc8(data, len);

    return len + 3;
}

const uint8_t* unframe_bytes(const uint8_t *data, size_t len, size_t *out_len)
{
    if (len < 4)
    {
        return NULL;
    }

    if (data[0] != FRAME_START)
    {
        return NULL;
    }

    const uint8_t *payload = data + 2;
    size_t payload_len = data[1];
    const uint8_t crc = data[len - 1];

    if (payload_len + 3 != len)
    {
        return NULL;
    }

    if (calc_crc8(payload, payload_len) != crc)
    {
        return NULL;
    }

    *out_len = payload_len;
    return payload;
}
