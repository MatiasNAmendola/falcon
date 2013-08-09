/*
 * Copyright (c) 2012 Martin Boßlet
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SIPHASH_H
#define SIPHASH_H

extern "C" {

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

        typedef struct sip_hash_st sip_hash;

#ifndef HAVE_UINT64_T
        typedef struct {
                uint32_t u32[2];
        } sip_uint64_t;
#define uint64_t sip_uint64_t
#endif

        sip_hash *sip_hash_new(uint8_t key[16], int c, int d);
        int sip_hash_update(sip_hash *h, uint8_t *data, size_t len);
        int sip_hash_final(sip_hash *h, uint8_t **digest, size_t *len);
        int sip_hash_digest(sip_hash *h, uint8_t *data, size_t data_len, uint8_t **digest, size_t *digest_len);

        uint64_t sip_hash24(uint8_t key[16], uint8_t *data, size_t len);

}

#endif
