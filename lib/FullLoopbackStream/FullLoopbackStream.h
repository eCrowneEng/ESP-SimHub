#pragma once

#include <LoopbackStream.h>

/**
 * Same as LoopbackStream, but it includes extra methods to handle what we need from it
 *  basically we're just completing the existing usages of Serial
 */
class FullLoopbackStream : public LoopbackStream
{
public:
    FullLoopbackStream(uint16_t buffer_size = LoopbackStream::DEFAULT_SIZE);

    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *buffer, size_t size);
    size_t write(const char *str);
    using LoopbackStream::write;
};
