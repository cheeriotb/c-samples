/*
 *  Copyright (C) 2021 Cheerio <cheerio.the.bear@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the MIT license.
 *  See the license information described in LICENSE file.
 */

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT_HTTPS 443

int connect_socket(const char* fqdn) {
	struct hostent *host;
	struct sockaddr_in remote;
    struct servent *serv;
	int fd;

	if ((host = gethostbyname(fqdn)) == NULL) {
        printf("Error: The specified host name could not be resolved.\n");
		return -1;
	}
	if (host->h_addrtype != AF_INET) {
        printf("Error: Unexpected address family %d was obtained.\n", host->h_addrtype);
		return -1;
	}

	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	memcpy(&remote.sin_addr, host->h_addr, sizeof(remote.sin_addr));

    serv = getservbyname("https", "tcp");
    if (serv != NULL){
        remote.sin_port = serv->s_port;
    } else {
        remote.sin_port = htons(DEFAULT_PORT_HTTPS);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) > -1) {
        if (connect(fd, (struct sockaddr *) &remote, sizeof(remote)) > -1) {
	        return fd;
        }
        close(fd);
    }

    return -1;
}

SSL_CTX *init_tls_context(char *certificate) {
    SSL_CTX *context = NULL;

	SSL_load_error_strings();
	SSL_library_init();

    context = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_options(context, SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
	if (context == NULL) {
		ERR_print_errors_fp(stderr);
		return NULL;
	}
    if (certificate != NULL) {
        if (SSL_CTX_load_verify_locations(context, certificate, NULL) == 0) {
	    	ERR_print_errors_fp(stderr);
            return NULL;
        }
    }
    return context;
}

SSL *establish_tls_connection(SSL_CTX *context, int fd)
{
    SSL *tls = SSL_new(context);
    if (tls != NULL) {
        if (SSL_set_fd(tls, fd) == 1) {
            if (SSL_connect(tls) == 1) {
                return tls;
            }
        }
    }
    ERR_print_errors_fp(stderr);
    return NULL;
}

void release_tls_connection(SSL *tls) {
    if (SSL_shutdown(tls) != 1) {
        ERR_print_errors_fp(stderr);
    }
    SSL_free(tls); 
}

void release_tls_context(SSL_CTX *context) {
    SSL_CTX_free(context);
    ERR_free_strings();
}

int execute(char* fqdn, char* certificate) {
    SSL_CTX *context = NULL;
    SSL *tls = NULL;
    int fd = -1;
    int ret = 1;

    if ((fd = connect_socket(fqdn)) > -1) {
        if ((context = init_tls_context(certificate)) != NULL) {
            tls = establish_tls_connection(context, fd);
            /*
               SSL_get_verify_result() is only useful in connection with SSL_get_peer_certificate().
            */
            if (tls != NULL && SSL_get_peer_certificate(tls) != NULL) {
                printf("Verify Result: %ld\n", SSL_get_verify_result(tls));
                release_tls_connection(tls);
                ret = 0;
            }
            release_tls_context(context);
        }
        close(fd);
    }

    return ret;
}

char *copy_string(char *source) {
    char *destination = (char *) malloc(strlen(source) + 1);
    strcpy(destination, source);
    return destination;
}

int main(int argc, char *argv[]) {
    char *fqdn = NULL;
    char *certificate = NULL;
    int option;
    int usage = 0;
    int result = 1;

    while ((option = getopt(argc, argv, "n:c:")) != -1) {
        switch (option) {
            case 'n':
                fqdn = copy_string(optarg);
                break;
            case 'c':
                certificate = copy_string(optarg);
                break;
            case '?':
                usage = 1;
                break;
            default:
                break;
        }
    }

    if (fqdn == NULL) {
        printf("Error: -n FQDN is the mandatory option.\n");
    } else if (usage != 0) {
        printf("Usage: %s -n FQDN [-c PEM file]\n", argv[0]);
    } else {
        result = execute(fqdn, certificate);
    }

    if (fqdn != NULL) {
        free(fqdn);
    }
    if (certificate != NULL) {
        free(certificate);
    }
    exit(result);
}
