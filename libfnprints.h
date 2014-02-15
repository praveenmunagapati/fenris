/*
   fenris - program execution path analysis tool
   ---------------------------------------------

   Copyright (C) 2001, 2002 by Bindview Corporation
   Portions copyright (C) 2001, 2002 by their respective contributors
   Developed and maintained by Michal Zalewski <lcamtuf@coredump.cx>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Extracted from fenris.c by Marcin Gozdalik.

 */

#ifndef _HAVE_LIBFNPRINTS_H
#define _HAVE_LIBFNPRINTS_H

#ifdef USE_OPENSSL
#include <openssl/md5.h>
#else 
#include <md5global.h>
#include <md5.h>
#define MD5_Init   MD5Init
#define MD5_Final  MD5Final
#define MD5_Update MD5Update
#endif /* USE_OPENSSL */

#include "config.h"


extern struct fenris_fndb *fndb[];

struct fenris_fndb {
  char* name;
  unsigned short a;
  struct fenris_fndb* next;
};


/* find all the names of funtions that match _fnprint value
 * found is called when a match is found:
 * 
 * void found(int count, struct fenris_fndb *cur, unsigned int fnprint, void *user)
 * count is the consecutive number of name that matched the given fingerprint
 * cur points the the record describing function
 * fnprint is the fingerprint itself
 * user is any user data passed to find_fnprints
 * 
 * finish is called after all the matching functions have been found
 * void finish(int count, usigned int fnprint, void *user)
 * count is the total number of functions that matched the given fingerprint
 * fnprint and user the same as in found
 */

#define find_fnprints(_fnprint, found, finish, user) \
do {  \
  int count = 0; \
  unsigned int __fnprint = (_fnprint);  \
  unsigned short sht = __fnprint & 0xffff; \
  struct fenris_fndb *cur;  \
  \
  cur=fndb[__fnprint>>16]; \
  \
  while (cur) { \
    if (cur->a == sht) {\
      found(count, cur, __fnprint, user); \
      count++; \
    } \
    cur=cur->next;  \
  } \
  \
  finish(count, __fnprint, user);  \
} while(0)


/* load the fingerprints database given in the argument */
int load_fnbase(const char* x);


/* return the number of loaded fingerprints */
int fnprints_count();

/* compute fingerprint for bytes given in sig
 * codeseg is the code segment used to detect relocations
 * sig has to be at last SIGNATSIZE+4 bytes long */

static inline unsigned long fnprint_compute(unsigned char *sig, int codeseg) {
    unsigned int result[4];
    MD5_CTX kuku;
    int i,tagme=0;

    for (i=2;i<SIGNATSIZE;i++) {
      // Three NOPs? That ain't no stinkin' code!
      if ((sig[i-2]==0x90) && (sig[i-1] == 0x90) && (sig[i] == 0x90)) {
        sig[i-2]=sig[i-1]=0;
        tagme=1;
      }

      if (tagme) sig[i]=0;
    }

    // I suck. TODO: parse relocs in signatures. For now,
    // just carefully exterminate anything interesting ;)
    for (i=0;i<SIGNATSIZE;i++)
     if (sig[i]==codeseg) bzero(&sig[i-3],4);

    for (i=0;i<SIGNATSIZE;i++)
     if (sig[i]==0xe8) bzero(&sig[i+1],4);

    MD5_Init(&kuku);
    MD5_Update(&kuku,sig,SIGNATSIZE);
    MD5_Final((char*)result,&kuku);

    result[0] ^= result[2];
    result[1] ^= result[3];

    return (result[0] ^ result[1]);
}

#endif /* not _HAVE_LIBFNPRINTS_H */
