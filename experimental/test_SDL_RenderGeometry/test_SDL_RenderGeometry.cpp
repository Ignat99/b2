#define _USE_MATH_DEFINES
#include <SDL.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <vector>
#include <memory>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define STATIC_ASSERT(EXPR) extern const char check_STATIC_ASSERT[(EXPR) ? 1 : -1]

#define MAX_NUM_PIXEL_FORMATS (100)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
static const char DEFAULT_RENDER_DRIVER[] = "opengl";
#else
static const char DEFAULT_RENDER_DRIVER[] = "opengles2";
#endif

static Uint32 g_window_timer_event_type;
static int g_wait_for_key = 1;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static const uint8_t FIXEDFONT[][13] = {
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 32 (0x20) ' '
    {
        0x00,
        0x00,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x00,
        0x04,
        0x00,
        0x00,
    }, // 33 (0x21) '!'
    {
        0x00,
        0x00,
        0x0A,
        0x0A,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 34 (0x22) '"'
    {
        0x00,
        0x00,
        0x00,
        0x0A,
        0x0A,
        0x1F,
        0x0A,
        0x1F,
        0x0A,
        0x0A,
        0x00,
        0x00,
        0x00,
    }, // 35 (0x23) '#'
    {
        0x00,
        0x00,
        0x04,
        0x1E,
        0x05,
        0x05,
        0x0E,
        0x14,
        0x14,
        0x0F,
        0x04,
        0x00,
        0x00,
    }, // 36 (0x24) '$'
    {
        0x00,
        0x00,
        0x12,
        0x15,
        0x0A,
        0x08,
        0x04,
        0x02,
        0x0A,
        0x15,
        0x09,
        0x00,
        0x00,
    }, // 37 (0x25) '%'
    {
        0x00,
        0x00,
        0x00,
        0x02,
        0x05,
        0x05,
        0x02,
        0x05,
        0x19,
        0x09,
        0x16,
        0x00,
        0x00,
    }, // 38 (0x26) '&'
    {
        0x00,
        0x00,
        0x0C,
        0x04,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 39 (0x27) '''
    {
        0x00,
        0x08,
        0x04,
        0x04,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x04,
        0x04,
        0x08,
        0x00,
    }, // 40 (0x28) '('
    {
        0x00,
        0x02,
        0x04,
        0x04,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x04,
        0x04,
        0x02,
        0x00,
    }, // 41 (0x29) ')'
    {
        0x00,
        0x00,
        0x04,
        0x15,
        0x0E,
        0x15,
        0x04,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 42 (0x2A) '*'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x04,
        0x1F,
        0x04,
        0x04,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 43 (0x2B) '+'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0C,
        0x04,
        0x02,
        0x00,
    }, // 44 (0x2C) ','
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x1F,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 45 (0x2D) '-'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x0E,
        0x04,
        0x00,
    }, // 46 (0x2E) '.'
    {
        0x00,
        0x00,
        0x10,
        0x10,
        0x08,
        0x08,
        0x04,
        0x02,
        0x02,
        0x01,
        0x01,
        0x00,
        0x00,
    }, // 47 (0x2F) '/'
    {
        0x00,
        0x00,
        0x04,
        0x0A,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0A,
        0x04,
        0x00,
        0x00,
    }, // 48 (0x30) '0'
    {
        0x00,
        0x00,
        0x04,
        0x06,
        0x05,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x1F,
        0x00,
        0x00,
    }, // 49 (0x31) '1'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x10,
        0x08,
        0x04,
        0x02,
        0x01,
        0x1F,
        0x00,
        0x00,
    }, // 50 (0x32) '2'
    {
        0x00,
        0x00,
        0x1F,
        0x10,
        0x08,
        0x04,
        0x0E,
        0x10,
        0x10,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 51 (0x33) '3'
    {
        0x00,
        0x00,
        0x08,
        0x08,
        0x0C,
        0x0A,
        0x0A,
        0x09,
        0x1F,
        0x08,
        0x08,
        0x00,
        0x00,
    }, // 52 (0x34) '4'
    {
        0x00,
        0x00,
        0x1F,
        0x01,
        0x01,
        0x0D,
        0x13,
        0x10,
        0x10,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 53 (0x35) '5'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x01,
        0x01,
        0x0F,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 54 (0x36) '6'
    {
        0x00,
        0x00,
        0x1F,
        0x10,
        0x08,
        0x08,
        0x04,
        0x04,
        0x02,
        0x02,
        0x02,
        0x00,
        0x00,
    }, // 55 (0x37) '7'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 56 (0x38) '8'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x1E,
        0x10,
        0x10,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 57 (0x39) '9'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x0E,
        0x04,
        0x00,
        0x00,
        0x04,
        0x0E,
        0x04,
        0x00,
    }, // 58 (0x3A) ':'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x0E,
        0x04,
        0x00,
        0x00,
        0x0C,
        0x04,
        0x02,
        0x00,
    }, // 59 (0x3B) ';'
    {
        0x00,
        0x00,
        0x10,
        0x08,
        0x04,
        0x02,
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x00,
        0x00,
    }, // 60 (0x3C) '<'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x1F,
        0x00,
        0x00,
        0x1F,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 61 (0x3D) '='
    {
        0x00,
        0x00,
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x08,
        0x04,
        0x02,
        0x01,
        0x00,
        0x00,
    }, // 62 (0x3E) '>'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x10,
        0x08,
        0x04,
        0x04,
        0x00,
        0x04,
        0x00,
        0x00,
    }, // 63 (0x3F) '?'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x19,
        0x15,
        0x15,
        0x0D,
        0x01,
        0x1E,
        0x00,
        0x00,
    }, // 64 (0x40) '@'
    {
        0x00,
        0x00,
        0x04,
        0x0A,
        0x11,
        0x11,
        0x11,
        0x1F,
        0x11,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 65 (0x41) 'A'
    {
        0x00,
        0x00,
        0x0F,
        0x12,
        0x12,
        0x12,
        0x0E,
        0x12,
        0x12,
        0x12,
        0x0F,
        0x00,
        0x00,
    }, // 66 (0x42) 'B'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 67 (0x43) 'C'
    {
        0x00,
        0x00,
        0x0F,
        0x12,
        0x12,
        0x12,
        0x12,
        0x12,
        0x12,
        0x12,
        0x0F,
        0x00,
        0x00,
    }, // 68 (0x44) 'D'
    {
        0x00,
        0x00,
        0x1F,
        0x01,
        0x01,
        0x01,
        0x0F,
        0x01,
        0x01,
        0x01,
        0x1F,
        0x00,
        0x00,
    }, // 69 (0x45) 'E'
    {
        0x00,
        0x00,
        0x1F,
        0x01,
        0x01,
        0x01,
        0x0F,
        0x01,
        0x01,
        0x01,
        0x01,
        0x00,
        0x00,
    }, // 70 (0x46) 'F'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x01,
        0x01,
        0x01,
        0x19,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 71 (0x47) 'G'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x11,
        0x1F,
        0x11,
        0x11,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 72 (0x48) 'H'
    {
        0x00,
        0x00,
        0x0E,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x0E,
        0x00,
        0x00,
    }, // 73 (0x49) 'I'
    {
        0x00,
        0x00,
        0x1C,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x09,
        0x06,
        0x00,
        0x00,
    }, // 74 (0x4A) 'J'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x09,
        0x05,
        0x03,
        0x05,
        0x09,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 75 (0x4B) 'K'
    {
        0x00,
        0x00,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x1F,
        0x00,
        0x00,
    }, // 76 (0x4C) 'L'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x1B,
        0x15,
        0x15,
        0x11,
        0x11,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 77 (0x4D) 'M'
    {
        0x00,
        0x00,
        0x11,
        0x13,
        0x13,
        0x15,
        0x15,
        0x19,
        0x19,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 78 (0x4E) 'N'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 79 (0x4F) 'O'
    {
        0x00,
        0x00,
        0x0F,
        0x11,
        0x11,
        0x11,
        0x0F,
        0x01,
        0x01,
        0x01,
        0x01,
        0x00,
        0x00,
    }, // 80 (0x50) 'P'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x15,
        0x0E,
        0x10,
        0x00,
    }, // 81 (0x51) 'Q'
    {
        0x00,
        0x00,
        0x0F,
        0x11,
        0x11,
        0x11,
        0x0F,
        0x05,
        0x09,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 82 (0x52) 'R'
    {
        0x00,
        0x00,
        0x0E,
        0x11,
        0x01,
        0x01,
        0x0E,
        0x10,
        0x10,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 83 (0x53) 'S'
    {
        0x00,
        0x00,
        0x1F,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x00,
        0x00,
    }, // 84 (0x54) 'T'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 85 (0x55) 'U'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0A,
        0x0A,
        0x0A,
        0x04,
        0x04,
        0x00,
        0x00,
    }, // 86 (0x56) 'V'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x11,
        0x15,
        0x15,
        0x15,
        0x15,
        0x0A,
        0x00,
        0x00,
    }, // 87 (0x57) 'W'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x0A,
        0x0A,
        0x04,
        0x0A,
        0x0A,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 88 (0x58) 'X'
    {
        0x00,
        0x00,
        0x11,
        0x11,
        0x0A,
        0x0A,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x00,
        0x00,
    }, // 89 (0x59) 'Y'
    {
        0x00,
        0x00,
        0x1F,
        0x10,
        0x08,
        0x08,
        0x04,
        0x02,
        0x02,
        0x01,
        0x1F,
        0x00,
        0x00,
    }, // 90 (0x5A) 'Z'
    {
        0x00,
        0x0E,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x02,
        0x0E,
        0x00,
    }, // 91 (0x5B) '['
    {
        0x00,
        0x00,
        0x01,
        0x01,
        0x02,
        0x02,
        0x04,
        0x08,
        0x08,
        0x10,
        0x10,
        0x00,
        0x00,
    }, // 92 (0x5C) '\'
    {
        0x00,
        0x0E,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x0E,
        0x00,
    }, // 93 (0x5D) ']'
    {
        0x00,
        0x00,
        0x04,
        0x0A,
        0x11,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 94 (0x5E) '^'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x1F,
        0x00,
    }, // 95 (0x5F) '_'
    {
        0x00,
        0x00,
        0x0C,
        0x08,
        0x10,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 96 (0x60) '`'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x10,
        0x1E,
        0x11,
        0x19,
        0x16,
        0x00,
        0x00,
    }, // 97 (0x61) 'a'
    {
        0x00,
        0x00,
        0x01,
        0x01,
        0x01,
        0x0F,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0F,
        0x00,
        0x00,
    }, // 98 (0x62) 'b'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x11,
        0x01,
        0x01,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 99 (0x63) 'c'
    {
        0x00,
        0x00,
        0x10,
        0x10,
        0x10,
        0x1E,
        0x11,
        0x11,
        0x11,
        0x11,
        0x1E,
        0x00,
        0x00,
    }, // 100 (0x64) 'd'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x11,
        0x1F,
        0x01,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 101 (0x65) 'e'
    {
        0x00,
        0x00,
        0x0C,
        0x12,
        0x02,
        0x02,
        0x0F,
        0x02,
        0x02,
        0x02,
        0x02,
        0x00,
        0x00,
    }, // 102 (0x66) 'f'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x1E,
        0x10,
        0x11,
        0x0E,
    }, // 103 (0x67) 'g'
    {
        0x00,
        0x00,
        0x01,
        0x01,
        0x01,
        0x0D,
        0x13,
        0x11,
        0x11,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 104 (0x68) 'h'
    {
        0x00,
        0x00,
        0x00,
        0x04,
        0x00,
        0x06,
        0x04,
        0x04,
        0x04,
        0x04,
        0x0E,
        0x00,
        0x00,
    }, // 105 (0x69) 'i'
    {
        0x00,
        0x00,
        0x00,
        0x08,
        0x00,
        0x0C,
        0x08,
        0x08,
        0x08,
        0x08,
        0x09,
        0x09,
        0x06,
    }, // 106 (0x6A) 'j'
    {
        0x00,
        0x00,
        0x01,
        0x01,
        0x01,
        0x09,
        0x05,
        0x03,
        0x05,
        0x09,
        0x11,
        0x00,
        0x00,
    }, // 107 (0x6B) 'k'
    {
        0x00,
        0x00,
        0x06,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x0E,
        0x00,
        0x00,
    }, // 108 (0x6C) 'l'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0B,
        0x15,
        0x15,
        0x15,
        0x15,
        0x11,
        0x00,
        0x00,
    }, // 109 (0x6D) 'm'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0D,
        0x13,
        0x11,
        0x11,
        0x11,
        0x11,
        0x00,
        0x00,
    }, // 110 (0x6E) 'n'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x11,
        0x11,
        0x11,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 111 (0x6F) 'o'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0F,
        0x11,
        0x11,
        0x11,
        0x0F,
        0x01,
        0x01,
        0x01,
    }, // 112 (0x70) 'p'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x1E,
        0x11,
        0x11,
        0x11,
        0x1E,
        0x10,
        0x10,
        0x10,
    }, // 113 (0x71) 'q'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0D,
        0x13,
        0x01,
        0x01,
        0x01,
        0x01,
        0x00,
        0x00,
    }, // 114 (0x72) 'r'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x0E,
        0x11,
        0x06,
        0x08,
        0x11,
        0x0E,
        0x00,
        0x00,
    }, // 115 (0x73) 's'
    {
        0x00,
        0x00,
        0x00,
        0x02,
        0x02,
        0x0F,
        0x02,
        0x02,
        0x02,
        0x12,
        0x0C,
        0x00,
        0x00,
    }, // 116 (0x74) 't'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x11,
        0x19,
        0x16,
        0x00,
        0x00,
    }, // 117 (0x75) 'u'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x0A,
        0x0A,
        0x04,
        0x00,
        0x00,
    }, // 118 (0x76) 'v'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x11,
        0x15,
        0x15,
        0x15,
        0x0A,
        0x00,
        0x00,
    }, // 119 (0x77) 'w'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x0A,
        0x04,
        0x04,
        0x0A,
        0x11,
        0x00,
        0x00,
    }, // 120 (0x78) 'x'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x11,
        0x11,
        0x19,
        0x16,
        0x10,
        0x11,
        0x0E,
    }, // 121 (0x79) 'y'
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x1F,
        0x08,
        0x04,
        0x02,
        0x01,
        0x1F,
        0x00,
        0x00,
    }, // 122 (0x7A) 'z'
    {
        0x00,
        0x18,
        0x04,
        0x04,
        0x04,
        0x04,
        0x03,
        0x04,
        0x04,
        0x04,
        0x04,
        0x18,
        0x00,
    }, // 123 (0x7B) '{'
    {
        0x00,
        0x00,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x04,
        0x00,
        0x00,
    }, // 124 (0x7C) '|'
    {
        0x00,
        0x03,
        0x04,
        0x04,
        0x04,
        0x04,
        0x18,
        0x04,
        0x04,
        0x04,
        0x04,
        0x03,
        0x00,
    }, // 125 (0x7D) '}'
    {
        0x00,
        0x00,
        0x12,
        0x15,
        0x09,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    }, // 126 (0x7E) '~'
};

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct SDL_Deleter {
    void operator()(SDL_Window *w) const {
        SDL_DestroyWindow(w);
    }

    void operator()(SDL_Renderer *r) const {
        SDL_DestroyRenderer(r);
    }

    void operator()(SDL_Texture *t) const {
        SDL_DestroyTexture(t);
    }

    void operator()(SDL_Surface *s) const {
        SDL_FreeSurface(s);
    }

    void operator()(SDL_PixelFormat *p) const {
        SDL_FreeFormat(p);
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void FatalError(const char *fmt, ...) {
    va_list v;
    va_start(v, fmt);
    vfprintf(stderr, fmt, v);
    va_end(v);

    exit(1);
}

static void FatalSDLError(const char *fmt, ...) {
    va_list v;
    va_start(v, fmt);
    vfprintf(stderr, fmt, v);
    va_end(v);

    fprintf(stderr, " failed: %s\n", SDL_GetError());

    exit(1);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void Finish(void) {
#ifdef _WIN32
    if (IsDebuggerPresent()) {
        if (g_wait_for_key) {
            fprintf(stderr, "press enter to exit.\n");
            (void)getchar();
        }
    }
#endif
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static Uint32 UpdateWindowTimer(Uint32 interval, void *param) {
    (void)param;

    SDL_Event event = {};
    event.user.type = g_window_timer_event_type;

    SDL_PushEvent(&event);

    return interval;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void SetSurfacePixel(SDL_Surface *surface, int x, int y, SDL_Color c) {
    if (x < 0 || x >= surface->w) {
        return;
    }

    if (y < 0 || y >= surface->h) {
        return;
    }

    if (surface->pitch <= 0) {
        return;
    }

    SDL_LockSurface(surface);

    Uint32 *dest = (Uint32 *)((uint8_t *)surface->pixels + (size_t)x * SDL_BYTESPERPIXEL(surface->format->format) + (size_t)(y * surface->pitch));

    *dest = SDL_MapRGBA(surface->format, c.r, c.g, c.b, c.a);

    SDL_UnlockSurface(surface);
}

static void DrawChar(SDL_Surface *surface, int x, int y, SDL_Color c, char ch) {
    if (ch < 32 || ch >= 127) {
        return;
    }

    const uint8_t *data = FIXEDFONT[ch - 32];

    for (int dy = 0; dy < 13; ++dy) {
        for (int dx = 0; dx < 6; ++dx) {
            if (data[dy] & (1 << dx)) {
                SetSurfacePixel(surface, x + dx, y + dy, c);
            }
        }
    }
}

static void DrawString(SDL_Surface *surface, int x, int y, SDL_Color c, const char *str) {
    for (const char *ch = str; *ch != 0; ++ch) {
        DrawChar(surface, x, y, c, *ch);
        x += 6;
    }
}

static SDL_Color GetColour(int n) {
    SDL_Color c;

    c.r = n & 1 ? 255 : 0;
    c.g = n & 2 ? 255 : 0;
    c.b = n & 4 ? 255 : 0;
    c.a = 255;

    return c;
}

static std::unique_ptr<SDL_Texture, SDL_Deleter> CreateTestTexture(SDL_Renderer *renderer, int width, int height, Uint32 format) {
    printf("%s: %dx%d, %s\n", __func__, width, height, SDL_GetPixelFormatName(format));

    std::unique_ptr<SDL_Surface, SDL_Deleter> surface(SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32));
    if (!surface) {
        printf("    SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        return std::unique_ptr<SDL_Texture, SDL_Deleter>();
    }

    for (int i = 0; i < 3; ++i) {
        uint8_t rgb[3] = {0, 0, 0};
        rgb[i] = 255;

        SDL_Rect rect;
        rect.x = (int)(i / 3.f * width);
        rect.y = 0;
        rect.w = width - rect.x;
        rect.h = height;

        SDL_FillRect(surface.get(), &rect, SDL_MapRGBA(surface->format, rgb[0], rgb[1], rgb[2], 255));
    }

    const char *name = SDL_GetPixelFormatName(format);

    DrawString(surface.get(), width / 2 - (int)(strlen(name) * 6 / 2), height / 2 - 13 / 2, GetColour(7), name);

    //DrawString(surface.get(),width/2,height/2,GetColour(7),"Hello");

    //
    std::unique_ptr<SDL_Surface, SDL_Deleter> surface2(SDL_ConvertSurfaceFormat(surface.get(), format, 0));
    if (!surface2) {
        printf("    SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return std::unique_ptr<SDL_Texture, SDL_Deleter>();
    }

    std::unique_ptr<SDL_Texture, SDL_Deleter> texture(SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STATIC, width, height));
    if (!texture) {
        printf("    SDL_CreateTexture failed: %s\n", SDL_GetError());
        return std::unique_ptr<SDL_Texture, SDL_Deleter>();
    }

    SDL_LockSurface(surface.get());

    printf("    Desired format is: %s\n", SDL_GetPixelFormatName(format));
    {
        std::unique_ptr<SDL_PixelFormat, SDL_Deleter> tmp(SDL_AllocFormat(format));
        printf("      (R=0x%x G=0x%x B=0x%x A=0x%x)\n", tmp->Rmask, tmp->Gmask, tmp->Bmask, tmp->Amask);
    }

    printf("    Actual format is: %s\n", SDL_GetPixelFormatName(surface2->format->format));
    {
        std::unique_ptr<SDL_PixelFormat, SDL_Deleter> tmp(SDL_AllocFormat(surface2->format->format));
        printf("      (R=0x%x G=0x%x B=0x%x A=0x%x)\n", tmp->Rmask, tmp->Gmask, tmp->Bmask, tmp->Amask);
    }

    //if(format!=surface->format->format) {
    //    printf("    (skipping)\n");
    //    return std::unique_ptr<SDL_Texture,SDL_Deleter>();;
    //}

    SDL_UpdateTexture(texture.get(), NULL, surface2->pixels, surface2->pitch);

    SDL_UnlockSurface(surface.get());

    return texture;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct TestRenderGeometryState {
    double a = 0;
    std::vector<std::unique_ptr<SDL_Texture, SDL_Deleter>> textures;
    int do_RenderGeometry;
};
typedef struct TestRenderGeometryState TestRenderGeometryState;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void Update(TestRenderGeometryState *s) {
    s->a += M_PI / 1000.f;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define NUM_POINTS (3)
STATIC_ASSERT(NUM_POINTS >= 3);
STATIC_ASSERT(NUM_POINTS * 3 <= 65535);

static void SetVertex(SDL_Vertex *v, float x, float y, uint8_t r, uint8_t g, uint8_t b, uint8_t a, float s, float t) {
    v->position.x = x;
    v->position.y = y;
    //v->position.z=0.f;

    v->color.r = r;
    v->color.g = g;
    v->color.b = b;
    v->color.a = a;

    v->tex_coord.x = s;
    v->tex_coord.y = t;
}

// static void Render2(TestRenderGeometryState *s,SDL_Window *window,SDL_Renderer *renderer) {
//     (void)s,(void)window;

//     int w,h;
//     SDL_GetRendererOutputSize(renderer,&w,&h);

//     SDL_SetRenderDrawColor(renderer,0,64,0,255);
//     SDL_RenderClear(renderer);

//     // SDL_Color colour={255,255,255,255};
//     // SDL_Vector2f uv={0,0};

//     //SDL_Vertex vertices[]={
//     //    {{0,0,0},colour,uv},
//     //    {{w,0,0},colour,uv},
//     //    {{w,h,0},colour,uv},
//     //    {{0,h,0},colour,uv},
//     //};

//     SDL_RenderPresent(renderer);
// }

static void Render(TestRenderGeometryState *state, SDL_Window *window, SDL_Renderer *renderer) {
    (void)window;

    int w, h;
    //SDL_GetWindowSize(window,&w,&h);
    SDL_GetRendererOutputSize(renderer, &w, &h);

    SDL_SetRenderDrawColor(renderer, 64, 0, 0, 255);
    SDL_RenderClear(renderer);

    // uint8_t r=rand()&0xff,g=rand()&0xff,b=rand()&0xff,a=255;

    // b=g=r=200;

    SDL_Vertex vertices[1 + NUM_POINTS], *vertex = vertices;
    uint16_t indices[NUM_POINTS * 3], *index = indices; //stoopid

    SDL_Vector2f mini, maxi;

    mini.x = maxi.x = w * .5f;
    mini.y = maxi.y = h * .5f;

    for (Uint16 i = 0; i < NUM_POINTS; ++i) {
        double angle = state->a + (i / (double)NUM_POINTS) * 2. * M_PI;
        double c = cos(angle), s = sin(angle);

        float x = (float)(w * .5 + c * w * .4);
        float y = (float)(h * .5 - s * h * .4);

        mini.x = SDL_min(mini.x, x);
        mini.y = SDL_min(mini.y, y);

        maxi.x = SDL_max(maxi.x, x);
        maxi.y = SDL_max(maxi.y, y);

        SetVertex(vertex++, x, y, 255, 255, 255, 255, (float)(.5 + c * .5), (float)(.5 - s * .5));

        *index++ = NUM_POINTS;
        *index++ = i;
        *index++ = (i + 1) % NUM_POINTS;
    }

    SetVertex(vertex++, w * .5f, h * .5f, 255, 255, 255, 255, .5f, .5f);

    SDL_Rect rect;

    rect.x = (int)mini.x;
    rect.y = (int)mini.y;
    rect.w = (int)(maxi.x - mini.x);
    rect.h = (int)(maxi.y - mini.y);

    int num_vertices = (int)(vertex - vertices);
    assert(num_vertices >= 0);
    assert((size_t)num_vertices <= sizeof vertices / sizeof vertices[0]);

    int num_indices = (int)(index - indices);
    assert(num_indices >= 0);
    assert((size_t)num_indices <= sizeof indices / sizeof indices[0]);

    SDL_Texture *texture = state->textures[0].get();

    // TODO - fix this up to use the RenderGeometry caps flag...
    if (state->do_RenderGeometry) {
        if (SDL_RenderGeometry(renderer, texture, vertices, (Uint16)num_vertices, indices, (Uint16)num_indices, NULL) != 0) {
            printf("renderer probably doesn't support RenderGeometry...\n");
            state->do_RenderGeometry = 0;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);

    int size = 50;

    SDL_Vector2f offset = {w * .5f, h * .5f};
    if (state->do_RenderGeometry) {
        SDL_Rect clip;
        clip.x = w - size;
        clip.y = 0;
        clip.w = size;
        clip.h = h;

        SDL_RenderSetClipRect(renderer, &clip);
        SDL_RenderGeometry(renderer, NULL, vertices, (Uint16)num_vertices, indices, (Uint16)num_indices, &offset);
        SDL_RenderSetClipRect(renderer, NULL);
    }

    {

        SDL_Vertex tri_vtxs[] = {
            {{(float)w, (float)h - size}, {255, 0, 0, 255}},
            {{(float)w, (float)h}, {255, 255, 0, 255}},
            {{(float)w - size, (float)h}, {0, 128, 255, 255}},
        };

        //int tri_idxs[]={0,1,2};

        if (state->do_RenderGeometry) {
            SDL_RenderGeometry(renderer, NULL, tri_vtxs, 3, NULL, 0, NULL);
        }
    }

    SDL_Rect dest = {0, 0, 256, 256};
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_RenderPresent(renderer);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// A maddening enum with no name, impossible to examine in the
// debugger.
static void PrintPixelformatEnum(void) {
    printf("SDL_PIXELFORMAT_INDEX1LSB = %u (0x%x)\n", SDL_PIXELFORMAT_INDEX1LSB, SDL_PIXELFORMAT_INDEX1LSB);
    printf("SDL_PIXELFORMAT_INDEX1MSB = %u (0x%x)\n", SDL_PIXELFORMAT_INDEX1MSB, SDL_PIXELFORMAT_INDEX1MSB);
    printf("SDL_PIXELFORMAT_INDEX4LSB = %u (0x%x)\n", SDL_PIXELFORMAT_INDEX4LSB, SDL_PIXELFORMAT_INDEX4LSB);
    printf("SDL_PIXELFORMAT_INDEX4MSB = %u (0x%x)\n", SDL_PIXELFORMAT_INDEX4MSB, SDL_PIXELFORMAT_INDEX4MSB);
    printf("SDL_PIXELFORMAT_INDEX8 = %u (0x%x)\n", SDL_PIXELFORMAT_INDEX8, SDL_PIXELFORMAT_INDEX8);
    printf("SDL_PIXELFORMAT_RGB332 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB332, SDL_PIXELFORMAT_RGB332);
    printf("SDL_PIXELFORMAT_RGB444 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB444, SDL_PIXELFORMAT_RGB444);
    printf("SDL_PIXELFORMAT_RGB555 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB555, SDL_PIXELFORMAT_RGB555);
    printf("SDL_PIXELFORMAT_BGR555 = %u (0x%x)\n", SDL_PIXELFORMAT_BGR555, SDL_PIXELFORMAT_BGR555);
    printf("SDL_PIXELFORMAT_ARGB4444 = %u (0x%x)\n", SDL_PIXELFORMAT_ARGB4444, SDL_PIXELFORMAT_ARGB4444);
    printf("SDL_PIXELFORMAT_RGBA4444 = %u (0x%x)\n", SDL_PIXELFORMAT_RGBA4444, SDL_PIXELFORMAT_RGBA4444);
    printf("SDL_PIXELFORMAT_ABGR4444 = %u (0x%x)\n", SDL_PIXELFORMAT_ABGR4444, SDL_PIXELFORMAT_ABGR4444);
    printf("SDL_PIXELFORMAT_BGRA4444 = %u (0x%x)\n", SDL_PIXELFORMAT_BGRA4444, SDL_PIXELFORMAT_BGRA4444);
    printf("SDL_PIXELFORMAT_ARGB1555 = %u (0x%x)\n", SDL_PIXELFORMAT_ARGB1555, SDL_PIXELFORMAT_ARGB1555);
    printf("SDL_PIXELFORMAT_RGBA5551 = %u (0x%x)\n", SDL_PIXELFORMAT_RGBA5551, SDL_PIXELFORMAT_RGBA5551);
    printf("SDL_PIXELFORMAT_ABGR1555 = %u (0x%x)\n", SDL_PIXELFORMAT_ABGR1555, SDL_PIXELFORMAT_ABGR1555);
    printf("SDL_PIXELFORMAT_BGRA5551 = %u (0x%x)\n", SDL_PIXELFORMAT_BGRA5551, SDL_PIXELFORMAT_BGRA5551);
    printf("SDL_PIXELFORMAT_RGB565 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB565, SDL_PIXELFORMAT_RGB565);
    printf("SDL_PIXELFORMAT_BGR565 = %u (0x%x)\n", SDL_PIXELFORMAT_BGR565, SDL_PIXELFORMAT_BGR565);
    printf("SDL_PIXELFORMAT_RGB24 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB24, SDL_PIXELFORMAT_RGB24);
    printf("SDL_PIXELFORMAT_BGR24 = %u (0x%x)\n", SDL_PIXELFORMAT_BGR24, SDL_PIXELFORMAT_BGR24);
    printf("SDL_PIXELFORMAT_RGB888 = %u (0x%x)\n", SDL_PIXELFORMAT_RGB888, SDL_PIXELFORMAT_RGB888);
    printf("SDL_PIXELFORMAT_RGBX8888 = %u (0x%x)\n", SDL_PIXELFORMAT_RGBX8888, SDL_PIXELFORMAT_RGBX8888);
    printf("SDL_PIXELFORMAT_BGR888 = %u (0x%x)\n", SDL_PIXELFORMAT_BGR888, SDL_PIXELFORMAT_BGR888);
    printf("SDL_PIXELFORMAT_BGRX8888 = %u (0x%x)\n", SDL_PIXELFORMAT_BGRX8888, SDL_PIXELFORMAT_BGRX8888);
    printf("SDL_PIXELFORMAT_ARGB8888 = %u (0x%x)\n", SDL_PIXELFORMAT_ARGB8888, SDL_PIXELFORMAT_ARGB8888);
    printf("SDL_PIXELFORMAT_RGBA8888 = %u (0x%x)\n", SDL_PIXELFORMAT_RGBA8888, SDL_PIXELFORMAT_RGBA8888);
    printf("SDL_PIXELFORMAT_ABGR8888 = %u (0x%x)\n", SDL_PIXELFORMAT_ABGR8888, SDL_PIXELFORMAT_ABGR8888);
    printf("SDL_PIXELFORMAT_BGRA8888 = %u (0x%x)\n", SDL_PIXELFORMAT_BGRA8888, SDL_PIXELFORMAT_BGRA8888);
    printf("SDL_PIXELFORMAT_ARGB2101010 = %u (0x%x)\n", SDL_PIXELFORMAT_ARGB2101010, SDL_PIXELFORMAT_ARGB2101010);
    printf("SDL_PIXELFORMAT_YV12 = %u (0x%x)\n", SDL_PIXELFORMAT_YV12, SDL_PIXELFORMAT_YV12);
    printf("SDL_PIXELFORMAT_IYUV = %u (0x%x)\n", SDL_PIXELFORMAT_IYUV, SDL_PIXELFORMAT_IYUV);
    printf("SDL_PIXELFORMAT_YUY2 = %u (0x%x)\n", SDL_PIXELFORMAT_YUY2, SDL_PIXELFORMAT_YUY2);
    printf("SDL_PIXELFORMAT_UYVY = %u (0x%x)\n", SDL_PIXELFORMAT_UYVY, SDL_PIXELFORMAT_UYVY);
    printf("SDL_PIXELFORMAT_YVYU = %u (0x%x)\n", SDL_PIXELFORMAT_YVYU, SDL_PIXELFORMAT_YVYU);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    PrintPixelformatEnum();

    atexit(&Finish);

    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        FatalSDLError("SDL_Init");
    }

    SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_VERBOSE);

    std::string requested_render_driver = DEFAULT_RENDER_DRIVER;
    if (argc > 1) {
        requested_render_driver = argv[1];
    }

    printf("Available render drivers:\n");
    SDL_RendererInfo renderer_info{};
    int render_driver_idx = -1;

    for (int i = 0; i < SDL_GetNumRenderDrivers(); ++i) {
        SDL_RendererInfo info;
        if (SDL_GetRenderDriverInfo(i, &info) != 0) {
            FatalSDLError("SDL_GetRenderDriverInfo for render driver %d", i);
        }

        printf("    %s", info.name);
        if (info.name == requested_render_driver) {
            render_driver_idx = i;
            renderer_info = info;
            printf(" <-- selected");
        }

        printf("\n");
    }

    if (render_driver_idx == -1) {
        FatalError("unknown render driver: %s\n", requested_render_driver.c_str());
    }

    std::unique_ptr<SDL_Window, SDL_Deleter> window(SDL_CreateWindow("RenderGeometry test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
    if (!window) {
        FatalSDLError("SDL_CreateWindow");
    }

    std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer(SDL_CreateRenderer(window.get(), render_driver_idx, 0));
    if (!renderer) {
        FatalSDLError("SDL_CreateRenderer");
    }

    g_window_timer_event_type = SDL_RegisterEvents(1);
    SDL_AddTimer(20, &UpdateWindowTimer, NULL);

    TestRenderGeometryState state;

    state.do_RenderGeometry = 1;

    for (size_t i = 0; i < renderer_info.num_texture_formats; ++i) {
        std::unique_ptr<SDL_Texture, SDL_Deleter> texture = CreateTestTexture(renderer.get(), 256, 64, renderer_info.texture_formats[i]);
        if (!!texture) {
            state.textures.emplace_back(std::move(texture));
        }
    }

    for (;;) {
        SDL_Event event;
        if (!SDL_WaitEvent(&event)) {
            FatalSDLError("SDL_WaitEvent");
        }

        switch (event.type) {
        case SDL_QUIT:
            goto done;

        default:
            if (event.type == g_window_timer_event_type) {
                Update(&state);
                Render(&state, window.get(), renderer.get());
            }
            break;
        }
    }
done:;

    g_wait_for_key = 0;

    return 0;
}
