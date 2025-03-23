// Cezary Miłek 339746

#include "icmp_utils.h"
#include <cstring>
#include <assert.h>

uint16_t computeChecksum(const void *buff, int length)
{
    const uint16_t *ptr = static_cast<const uint16_t *>(buff);
    uint32_t sum = 0;
    assert(length % 2 == 0);
    for (; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return static_cast<uint16_t>(~sum);
}

double getTimeDiffMs(const timeval &start, const timeval &end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
}
