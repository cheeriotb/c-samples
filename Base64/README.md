# Base64

## Introduction

This is a simple sample code for Base64 encoding and decoding.

## Usage

You have to choose -d for decoding or -e for encoding followed by an input data.

```
./sample [-d <base64-encoded string>] | [-e <string to be base64-encoded>]
```

Here is usage examples for both options.

```
$ ./sample -e foobar
Encoded: Zm9vYmFy
$ ./sample -d Zm9vYmFy
Decoded: foobar
```

## Dependency

There is no special dependency on other external libraries.

## Setup

There is nothing special about building step.

```
$ gcc -o sample *.c
```

## Licence

This software is released under the MIT License, see LICENSE.

## Author

Cheerio (cheerio.the.bear@gmail.com)
