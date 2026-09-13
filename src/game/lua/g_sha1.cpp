#include <bgame/impl.h>
#include <game/lua/g_lua.h>

#ifdef FEATURE_LUA

/* Public-domain SHA-1 (Steve Reid / common compact form). */

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

#define SHA1_ROL(v, b) (((v) << (b)) | ((v) >> (32 - (b))))

static void SHA1_Transform( uint32_t state[5], const unsigned char buffer[64] ) {
    uint32_t a, b, c, d, e, t, w[80];
    int i;

    for ( i = 0; i < 16; i++ ) {
        w[i] = ( (uint32_t)buffer[i * 4] << 24 )
             | ( (uint32_t)buffer[i * 4 + 1] << 16 )
             | ( (uint32_t)buffer[i * 4 + 2] << 8 )
             | ( (uint32_t)buffer[i * 4 + 3] );
    }
    for ( i = 16; i < 80; i++ ) {
        w[i] = SHA1_ROL( w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1 );
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
    for ( i = 0; i < 80; i++ ) {
        if ( i < 20 ) {
            t = SHA1_ROL( a, 5 ) + ( ( b & c ) | ( ( ~b ) & d ) ) + e + w[i] + 0x5A827999u;
        } else if ( i < 40 ) {
            t = SHA1_ROL( a, 5 ) + ( b ^ c ^ d ) + e + w[i] + 0x6ED9EBA1u;
        } else if ( i < 60 ) {
            t = SHA1_ROL( a, 5 ) + ( ( b & c ) | ( b & d ) | ( c & d ) ) + e + w[i] + 0x8F1BBCDCu;
        } else {
            t = SHA1_ROL( a, 5 ) + ( b ^ c ^ d ) + e + w[i] + 0xCA62C1D6u;
        }
        e = d; d = c; c = SHA1_ROL( b, 30 ); b = a; a = t;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void SHA1_Init( SHA1_CTX *ctx ) {
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xEFCDAB89u;
    ctx->state[2] = 0x98BADCFEu;
    ctx->state[3] = 0x10325476u;
    ctx->state[4] = 0xC3D2E1F0u;
    ctx->count[0] = ctx->count[1] = 0;
}

static void SHA1_Update( SHA1_CTX *ctx, const unsigned char *data, uint32_t len ) {
    uint32_t i, j;

    j = ( ctx->count[0] >> 3 ) & 63;
    if ( ( ctx->count[0] += len << 3 ) < ( len << 3 ) ) {
        ctx->count[1]++;
    }
    ctx->count[1] += ( len >> 29 );
    if ( ( j + len ) > 63 ) {
        i = 64 - j;
        memcpy( &ctx->buffer[j], data, i );
        SHA1_Transform( ctx->state, ctx->buffer );
        for ( ; i + 63 < len; i += 64 ) {
            SHA1_Transform( ctx->state, &data[i] );
        }
        j = 0;
    } else {
        i = 0;
    }
    memcpy( &ctx->buffer[j], &data[i], len - i );
}

static void SHA1_Final( unsigned char digest[20], SHA1_CTX *ctx ) {
    unsigned char finalcount[8];
    unsigned char c;
    int i;

    for ( i = 0; i < 8; i++ ) {
        finalcount[i] = (unsigned char)( ( ctx->count[( i >= 4 ) ? 0 : 1] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
    }
    c = 0x80;
    SHA1_Update( ctx, &c, 1 );
    while ( ( ctx->count[0] & 504 ) != 448 ) {
        c = 0;
        SHA1_Update( ctx, &c, 1 );
    }
    SHA1_Update( ctx, finalcount, 8 );
    for ( i = 0; i < 20; i++ ) {
        digest[i] = (unsigned char)( ( ctx->state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
    }
}

const char *G_SHA1( const char *string ) {
    static char hex[41];
    SHA1_CTX ctx;
    unsigned char digest[20];
    int i;
    static const char *digits = "0123456789abcdef";

    SHA1_Init( &ctx );
    if ( string ) {
        SHA1_Update( &ctx, (const unsigned char *)string, (uint32_t)strlen( string ) );
    }
    SHA1_Final( digest, &ctx );
    for ( i = 0; i < 20; i++ ) {
        hex[i * 2] = digits[( digest[i] >> 4 ) & 0xf];
        hex[i * 2 + 1] = digits[digest[i] & 0xf];
    }
    hex[40] = '\0';
    return hex;
}

#endif
