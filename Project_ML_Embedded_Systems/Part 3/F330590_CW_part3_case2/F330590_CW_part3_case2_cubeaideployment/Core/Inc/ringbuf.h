// https://vivonomicon.com/2020/06/28/bare-metal-stm32-programming-part-10-uart-communication/

/*
 * Minimal statically-allocated ringbuffer.
 */
#ifndef __VVC_RINGBUF
#define __VVC_RINGBUF

#include <stdint.h>

// Simple ring buffer.
typedef struct
{
    int len;
    volatile uint8_t *buf;
    volatile int pos;
    volatile int ext;
} ringbuf;
// Helper macro to write to a buffer.
#define ringbuf_write(rb, x)    \
    rb.buf[rb.ext] = x;         \
    if ((rb.ext + 1) >= rb.len) \
    {                           \
        rb.ext = 0;             \
    }                           \
    else                        \
    {                           \
        rb.ext = rb.ext + 1;    \
    }
// Read from a buffer. Returns '\0' if there is nothing to read.
static inline uint8_t ringbuf_read(ringbuf *buf)
{
    if (buf->pos == buf->ext)
    {
        return 0;
    }
    uint8_t read = buf->buf[buf->pos];
    buf->pos = (buf->pos < (buf->len - 1)) ? (buf->pos + 1) : 0;
    return read;
}
#endif