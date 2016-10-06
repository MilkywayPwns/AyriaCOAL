/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-7
    Notes:
*/

#ifndef _CRYPT_GENSALT_H
#define _CRYPT_GENSALT_H

unsigned char _crypt_itoa64[];
char *_crypt_gensalt_traditional_rn(const char *prefix,
	unsigned long count,
	const char *input, int size, char *output, int output_size);
char *_crypt_gensalt_extended_rn(const char *prefix,
	unsigned long count,
	const char *input, int size, char *output, int output_size);
char *_crypt_gensalt_md5_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size);

#endif
