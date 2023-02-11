#include <FullLoopbackStream.h>

FullLoopbackStream::FullLoopbackStream(uint16_t buffer_size) : LoopbackStream(buffer_size){};

size_t FullLoopbackStream::write(const char *str)
{
    if (str == NULL)
        return 0;
    return write((const uint8_t *)str, strlen(str));
}

size_t FullLoopbackStream::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--)
    {
        if (write(*buffer++))
            n++;
        else
            break;
    }
    return n;
}