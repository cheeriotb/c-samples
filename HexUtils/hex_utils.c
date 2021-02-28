/*
 *  Copyright (C) 2021 Cheerio <cheerio.the.bear@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the MIT license.
 *  See the license information described in LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *UTIL_CONVERT_TO_HEX_CHAR = "0123456789ABCDEF";

unsigned char *util_hex_string_to_byte_array(const char *string, size_t length)
{
    unsigned char *array;
    int index;
    unsigned char octet;
    char nibble;

    if ((array = (unsigned char *) malloc(length / 2)) == NULL) {
        return NULL;
    }

    for (index = 0; index < length; index++) {
        nibble = string[index];

        if (('0' <= nibble) && (nibble <= '9')) {
            nibble = nibble - '0';
        } else if (('a' <= nibble) && (nibble <= 'f')) {
            nibble = 0x0A + (nibble - 'a');
        } else if (('A' <= nibble) && (nibble <= 'F')) {
            nibble = 0x0A + (nibble - 'A');
        } else {
            free(array);
            return NULL;
        }

        if ((index % 2) == 0) {
            octet = nibble;
        } else {
            array[index / 2] = (octet << 4) + nibble;
        }
    }

    return array;
}

char *util_byte_array_to_hex_string(const unsigned char* array, size_t length)
{
    char *string;
    char *destination;
    int index;
    unsigned char octet;

    if (length == 0) {
        return NULL;
    }

    if ((string = destination = (char *) malloc(length * 2 + 1)) == NULL) {
        return NULL;
    }

    for (index = 0; index < length; index++) {
        octet = array[index];
        *destination++ = UTIL_CONVERT_TO_HEX_CHAR[octet / 0x10];
        *destination++ = UTIL_CONVERT_TO_HEX_CHAR[octet % 0x10];
    }

    *destination = '\0';
    return string;
}

int main(int argc, char *argv[]) {

    const char *data = "00017F8081FEFF";
    unsigned char *array;
    char *string;

    array = util_hex_string_to_byte_array(data, strlen(data));
    assert(array[0] == 0x00);
    assert(array[1] == 0x01);
    assert(array[2] == 0x7F);
    assert(array[3] == 0x80);
    assert(array[4] == 0x81);
    assert(array[5] == 0xFE);
    assert(array[6] == 0xFF);

    string = util_byte_array_to_hex_string(array, strlen(data) / 2);
    assert(strcmp(data, string) == 0);

    free(array);
    free(string);

    printf("OK\n");

    exit(0);
}
