/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
*/

#ifndef _CRYPT_BLOWFISH_H
#define _CRYPT_BLOWFISH_H

int _crypt_output_magic(const char *setting, char *output, int size);
char *_crypt_blowfish_rn(const char *key, const char *setting,
	char *output, int size);
char *_crypt_gensalt_blowfish_rn(const char *prefix,
	unsigned long count,
	const char *input, int size, char *output, int output_size);

#endif
