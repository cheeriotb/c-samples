/*
 *  Copyright (C) 2021 Cheerio <cheerio.the.bear@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the MIT license.
 *  See the license information described in LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char BASE64_ALPHABETS[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char BASE64_DECODE_TABLE[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const char PADDING_CHAR = '=';

size_t base64_encoded_size(const size_t size) {
    /*
       The encoding process represents 24-bit groups of input bits
       as output strings of 4 encoded characters.
       A block of 4 encoded characters can contain up to 3 input characters.
     */
    return ((size + 2) / 3) * 4;
}

size_t base64_encode(char *destination, const char *source, size_t size) {
    const char *eos = source + size;
    int padding = (3 - (size % 3)) % 3;
    int input;
    int i;

    if (destination == NULL || source == NULL || size == 0) {
        return 0;
    }

    do {
        input = 0;
        for (i = 0; i < 3; i++) {
            input = input << 8;
            if (source < eos) {
                input += *source++;
            }
        }
        for (i = 0; i < 4; i++) {
            *destination++ = BASE64_ALPHABETS[(input >> (3 - i) * 6) % 64];
        }
    } while (source < eos);

    for (i = 0; i < padding; i++) {
        *(destination - (i + 1)) = PADDING_CHAR;
    }

    return base64_encoded_size(size);
}

size_t base64_decoded_size(const char *source, const size_t size) {
    int padding = 0;
    size_t length = (size / 4) * 3;
    char *found = NULL;

    if ((size == 0) || (size % 4 != 0)) {
        return 0;
    }

    if ((found = memchr(source, PADDING_CHAR, size)) != NULL) {
        size_t padding = size - (found - source);
        if (padding > 2) {
            return 0;
        }
        length -= padding;
    }

    return length;
}

size_t base64_decode(char *destination, const char *source, size_t size) {
    int i;
    char encoded;
    unsigned char decoded;
    int index = 0;
    int input = 0;
    size_t length = 0;

    if ((length = base64_decoded_size(source, size)) == 0) {
        return 0;
    }

    do {
        for (i = 0; i < 4; i++) {
            encoded = *source;
            if ((encoded < 0x00) || (0x7F < encoded)) {
                return 0;
            }
            if ((decoded = BASE64_DECODE_TABLE[encoded]) == 0xFF) {
                return 0;
            }
            input = (input << 6) + decoded;
            source++;
        }
        for (i = 2; i >= 0; i--) {
            if ((index + i) < length) {
                destination[index + i] = (char)(input & 0xFF);
            }
            input = input >> 8;
        }
        index += 3;
    } while (index < length);

    return length;
}

char *copy_string(char *source) {
    char *destination = (char *) malloc(strlen(source) + 1);
    strcpy(destination, source);
    return destination;
}

int main(int argc, char *argv[]) {
    int option;
    int result = 0;
    char *decode = NULL;
    char *encode = NULL;
    char *buffer = NULL;
    size_t size = 0;

    while ((option = getopt(argc, argv, "d:e:")) != -1) {
        switch (option) {
            case 'd':
                decode = copy_string(optarg);
                break;
            case 'e':
                encode = copy_string(optarg);
                break;
            default:
                result = 1;
                break;
        }
    }

    if (result == 1 || (decode != NULL && encode != NULL) || (decode == NULL && encode == NULL)) {
        printf("Usage: %s [-d <base64-encoded string>] | [-e <string to be base64-encoded>]\n",
                argv[0]);
    } else if (encode != NULL) {
        size = base64_encoded_size(strlen(encode));
        buffer = (char *) malloc(size + 1);
        if (base64_encode(buffer, encode, strlen(encode)) == size) {
            buffer[size] = '\0';
            printf("Encoded: %s\n", buffer);
        } else {
            result = 1;
        }
    } else if (decode != NULL) {
        size = base64_decoded_size(decode, strlen(decode));
        buffer = (char *) malloc(size + 1);
        if (base64_decode(buffer, decode, strlen(decode)) == size) {
            buffer[size] = '\0';
            printf("Decoded: %s\n", buffer);
        } else {
            result = 1;
        }
    }

    if (buffer != NULL) {
        free(buffer);
    }
    if (encode != NULL) {
        free(encode);
    }
    if (decode != NULL) {
        free(decode);
    }

    exit(result);
}
