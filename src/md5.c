#include <string.h>
#include "pizza/md5.h"

// compute number of bytes mod 64
#define BITS_M64(b) ((int) (((b) >> 3) & 0x3f))

// Get a uint32_t from four bytes
#define COMBINE_BYTES(b, k) \
    ((((uint32_t)b[k+3]) << 24) | \
     (((uint32_t)b[k+2]) << 16) | \
     (((uint32_t)b[k+1]) <<  8) | \
     (((uint32_t)b[k+0])))

// Shift a byte and clamp it to unit8_t
#define SHIFT_BYTE(i, s) ((uint8_t)((i >> s) & 0xff))

static inline char tohex(uint8_t b);
static void transform(uint32_t* buf, uint32_t* inp);

static uint8_t padding[64] = {
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void md5_reset(MD5 *md5) {
    memset(md5, 0, sizeof(MD5));
    md5->work[0] = 0x67452301u;
    md5->work[1] = 0xefcdab89u;
    md5->work[2] = 0x98badcfeu;
    md5->work[3] = 0x10325476u;
}

void md5_update(MD5* md5, Slice s) {
    int mdi = BITS_M64(md5->bits[0]);

    // update number of bits
    if ((md5->bits[0] + (s.len << 3)) < md5->bits[0]) {
        md5->bits[1]++;
    }
    md5->bits[0] += s.len << 3;
    md5->bits[1] += s.len >> 29;

    uint32_t inp[MD5_DIGEST_LEN];
    for (uint32_t pos = 0; pos < s.len; ++pos) {
        // add new character to buffer, increment mdi
        md5->input[mdi++] = s.ptr[pos];

        if (mdi != 0x40) continue; // no transform is necessary

        for (uint32_t j = 0, k = 0; j < MD5_DIGEST_LEN; j++, k += 4) {
            inp[j] = COMBINE_BYTES(md5->input, k);
        }
        transform(md5->work, inp);
        mdi = 0;
    }
}

void md5_finalize(MD5* md5) {
    int mdi = BITS_M64(md5->bits[0]);

    // save number of bits
    uint32_t inp[MD5_DIGEST_LEN];
    inp[14] = md5->bits[0];
    inp[15] = md5->bits[1];

    // pad out to 56 mod 64
    uint32_t padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
    Slice pad = slice_from_memory((const char*) padding, padLen);
    md5_update(md5, pad);

    // append length in bits and transform
    for (uint32_t j = 0, k = 0; j < 14; j++, k += 4) {
        inp[j] = COMBINE_BYTES(md5->input, k);
    }
    transform(md5->work, inp);

    // store buffer in digest
    for (uint32_t j = 0, k = 0; j < 4; j++, k += 4) {
        md5->digest[k+0] = SHIFT_BYTE(md5->work[j],  0);
        md5->digest[k+1] = SHIFT_BYTE(md5->work[j],  8);
        md5->digest[k+2] = SHIFT_BYTE(md5->work[j], 16);
        md5->digest[k+3] = SHIFT_BYTE(md5->work[j], 24);
    }
}

void md5_format(MD5* md5, Buffer* b) {
    for (uint32_t k = 0; k < MD5_DIGEST_LEN; ++k) {
        uint8_t byte = md5->digest[k];
        buffer_append_byte(b, tohex(byte >> 4));
        buffer_append_byte(b, tohex(byte & 0xf));
    }
}

void md5_compute(MD5* md5, Slice s, Buffer* b) {
    md5_reset(md5);
    md5_update(md5, s);
    md5_finalize(md5);
    md5_format(md5, b);
}

// F, G and H are basic MD5 functions: selection, majority, parity
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// R1, R2, R3, and R4 transformations for rounds 1, 2, 3, and 4
#define R1(a, b, c, d, x, s, ac) \
    do { \
        (a) += F ((b), (c), (d)) + (x) + (ac); \
        (a) = ROTATE_LEFT ((a), (s)); \
        (a) += (b); \
    } while (0)

#define R2(a, b, c, d, x, s, ac) \
    do { \
        (a) += G ((b), (c), (d)) + (x) + (ac); \
        (a) = ROTATE_LEFT ((a), (s)); \
        (a) += (b); \
    } while (0)

#define R3(a, b, c, d, x, s, ac) \
    do { \
        (a) += H ((b), (c), (d)) + (x) + (ac); \
        (a) = ROTATE_LEFT ((a), (s)); \
        (a) += (b); \
    } while (0)

#define R4(a, b, c, d, x, s, ac) \
    do { \
        (a) += I ((b), (c), (d)) + (x) + (ac); \
        (a) = ROTATE_LEFT ((a), (s)); \
        (a) += (b); \
    } while (0)

// constants used for round 1
#define R1_1 7
#define R1_2 12
#define R1_3 17
#define R1_4 22

// constants used for round 2
#define R2_1 5
#define R2_2 9
#define R2_3 14
#define R2_4 20

// constants used for round 3
#define R3_1 4
#define R3_2 11
#define R3_3 16
#define R3_4 23

// constants used for round 4
#define R4_1 6
#define R4_2 10
#define R4_3 15
#define R4_4 21

// Basic MD5 step. Transform buf based on inp
static void transform(uint32_t* buf, uint32_t* inp) {
    uint32_t a = buf[0];
    uint32_t b = buf[1];
    uint32_t c = buf[2];
    uint32_t d = buf[3];

    R1( a, b, c, d, inp[ 0], R1_1, 3614090360u);
    R1( d, a, b, c, inp[ 1], R1_2, 3905402710u);
    R1( c, d, a, b, inp[ 2], R1_3,  606105819u);
    R1( b, c, d, a, inp[ 3], R1_4, 3250441966u);
    R1( a, b, c, d, inp[ 4], R1_1, 4118548399u);
    R1( d, a, b, c, inp[ 5], R1_2, 1200080426u);
    R1( c, d, a, b, inp[ 6], R1_3, 2821735955u);
    R1( b, c, d, a, inp[ 7], R1_4, 4249261313u);
    R1( a, b, c, d, inp[ 8], R1_1, 1770035416u);
    R1( d, a, b, c, inp[ 9], R1_2, 2336552879u);
    R1( c, d, a, b, inp[10], R1_3, 4294925233u);
    R1( b, c, d, a, inp[11], R1_4, 2304563134u);
    R1( a, b, c, d, inp[12], R1_1, 1804603682u);
    R1( d, a, b, c, inp[13], R1_2, 4254626195u);
    R1( c, d, a, b, inp[14], R1_3, 2792965006u);
    R1( b, c, d, a, inp[15], R1_4, 1236535329u);

    R2( a, b, c, d, inp[ 1], R2_1, 4129170786u);
    R2( d, a, b, c, inp[ 6], R2_2, 3225465664u);
    R2( c, d, a, b, inp[11], R2_3,  643717713u);
    R2( b, c, d, a, inp[ 0], R2_4, 3921069994u);
    R2( a, b, c, d, inp[ 5], R2_1, 3593408605u);
    R2( d, a, b, c, inp[10], R2_2,   38016083u);
    R2( c, d, a, b, inp[15], R2_3, 3634488961u);
    R2( b, c, d, a, inp[ 4], R2_4, 3889429448u);
    R2( a, b, c, d, inp[ 9], R2_1,  568446438u);
    R2( d, a, b, c, inp[14], R2_2, 3275163606u);
    R2( c, d, a, b, inp[ 3], R2_3, 4107603335u);
    R2( b, c, d, a, inp[ 8], R2_4, 1163531501u);
    R2( a, b, c, d, inp[13], R2_1, 2850285829u);
    R2( d, a, b, c, inp[ 2], R2_2, 4243563512u);
    R2( c, d, a, b, inp[ 7], R2_3, 1735328473u);
    R2( b, c, d, a, inp[12], R2_4, 2368359562u);

    R3( a, b, c, d, inp[ 5], R3_1, 4294588738u);
    R3( d, a, b, c, inp[ 8], R3_2, 2272392833u);
    R3( c, d, a, b, inp[11], R3_3, 1839030562u);
    R3( b, c, d, a, inp[14], R3_4, 4259657740u);
    R3( a, b, c, d, inp[ 1], R3_1, 2763975236u);
    R3( d, a, b, c, inp[ 4], R3_2, 1272893353u);
    R3( c, d, a, b, inp[ 7], R3_3, 4139469664u);
    R3( b, c, d, a, inp[10], R3_4, 3200236656u);
    R3( a, b, c, d, inp[13], R3_1,  681279174u);
    R3( d, a, b, c, inp[ 0], R3_2, 3936430074u);
    R3( c, d, a, b, inp[ 3], R3_3, 3572445317u);
    R3( b, c, d, a, inp[ 6], R3_4,   76029189u);
    R3( a, b, c, d, inp[ 9], R3_1, 3654602809u);
    R3( d, a, b, c, inp[12], R3_2, 3873151461u);
    R3( c, d, a, b, inp[15], R3_3,  530742520u);
    R3( b, c, d, a, inp[ 2], R3_4, 3299628645u);

    R4( a, b, c, d, inp[ 0], R4_1, 4096336452u);
    R4( d, a, b, c, inp[ 7], R4_2, 1126891415u);
    R4( c, d, a, b, inp[14], R4_3, 2878612391u);
    R4( b, c, d, a, inp[ 5], R4_4, 4237533241u);
    R4( a, b, c, d, inp[12], R4_1, 1700485571u);
    R4( d, a, b, c, inp[ 3], R4_2, 2399980690u);
    R4( c, d, a, b, inp[10], R4_3, 4293915773u);
    R4( b, c, d, a, inp[ 1], R4_4, 2240044497u);
    R4( a, b, c, d, inp[ 8], R4_1, 1873313359u);
    R4( d, a, b, c, inp[15], R4_2, 4264355552u);
    R4( c, d, a, b, inp[ 6], R4_3, 2734768916u);
    R4( b, c, d, a, inp[13], R4_4, 1309151649u);
    R4( a, b, c, d, inp[ 4], R4_1, 4149444226u);
    R4( d, a, b, c, inp[11], R4_2, 3174756917u);
    R4( c, d, a, b, inp[ 2], R4_3,  718787259u);
    R4( b, c, d, a, inp[ 9], R4_4, 3951481745u);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

// Get a nibble as a printable char
static inline char tohex(uint8_t b) {
    return b + ((b <= 9) ? '0' : ('a' - 10));
}
