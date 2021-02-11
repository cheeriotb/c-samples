# TLS

## Introduction

This is a simple sample code for establishing TLS connection by using OpenSSL.

## Usage

You have to specify a FQDN of a target server like "github.com". You can also specify a certificate file in PEM format as an optional parameter.

```
./sample -n FQDN [-c PEM file]
```

## Dependency

This code has dependencies on libssl and libcrypto. You might see the following errors when trying to build this code.

```
/usr/bin/ld: cannot find -lssl
/usr/bin/ld: cannot find -lcrypto
```

In that case, please install the libraries somehow like:

```
$ sudo apt install libssl-dev
```

## Setup

There is nothing special about building step.

```
$ gcc -o sample *.c -lssl -lcrypto
```

## Licence

This software is released under the MIT License, see LICENSE.

## Author

Cheerio (cheerio.the.bear@gmail.com)
