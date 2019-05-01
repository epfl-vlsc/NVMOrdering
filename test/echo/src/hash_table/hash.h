/* hash - hashing table processing.
   Copyright (C) 1998-1999, 2001, 2003, 2009-2012 Free Software Foundation,
   Inc.
   Written by Jim Meyering <meyering@ascend.com>, 1998.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* A generic hash table package.  */

/* Make sure USE_OBSTACK is defined to 1 if you want the allocator to use
   obstacks instead of malloc, and recompile 'hash.c' with same setting.  */

#ifndef HASH_H_
# define HASH_H_

# include <stdio.h>
# include <stdbool.h>

/* The __attribute__ feature is available in gcc versions 2.5 and later.
   The warn_unused_result attribute appeared first in gcc-3.4.0.  */
# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  define _GL_ATTRIBUTE_WUR __attribute__ ((__warn_unused_result__))
# else
#  define _GL_ATTRIBUTE_WUR /* empty */
# endif

//PJH: added this, from http://git.savannah.gnu.org/gitweb/?p=gnulib.git;a=commitdiff;h=349d7fe0e307d59d508b3579317ee8d4eacfeb9c
# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)  //PJH
#  define _GL_ATTRIBUTE_PURE __attribute__ ((__pure__))      //PJH
# else                                                       //PJH
#  define _GL_ATTRIBUTE_PURE /* empty */                     //PJH
# endif                                                      //PJH

# ifndef _GL_ATTRIBUTE_DEPRECATED
/* The __attribute__((__deprecated__)) feature
   is available in gcc versions 3.1 and newer.  */
#  if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
#   define _GL_ATTRIBUTE_DEPRECATED /* empty */
#  else
#   define _GL_ATTRIBUTE_DEPRECATED __attribute__ ((__deprecated__))
#  endif
# endif

typedef size_t (*Hash_hasher) (const void *, size_t);
typedef bool (*Hash_comparator) (const void *, const void *);
typedef void (*Hash_data_freer) (void *);
typedef bool (*Hash_processor) (void *, void *);
typedef bool (*Hash_lookup_action) (const void *entry);
/* PJH: this function performs an action on a hash table entry after
 *   it has been looked up but before the rwlock on the hash table
 *   has been released! The function should return true on success
 *   and false on error. */

struct hash_tuning
{
	/* This structure is mainly used for 'hash_initialize', see the block
	   documentation of 'hash_reset_tuning' for more complete comments.  */

	float shrink_threshold;     /* ratio of used buckets to trigger a shrink */
	float shrink_factor;        /* ratio of new smaller size to original size */
	float growth_threshold;     /* ratio of used buckets to trigger a growth */
	float growth_factor;        /* ratio of new bigger size to original size */
	bool is_n_buckets;          /* if CANDIDATE really means table size */
};

typedef struct hash_tuning Hash_tuning;

struct hash_table;

typedef struct hash_table Hash_table;

/* Information and lookup.  */
size_t hash_get_hash_table_size();  //PJH
size_t hash_get_n_buckets (const Hash_table *) _GL_ATTRIBUTE_PURE;
size_t hash_get_n_buckets_used (const Hash_table *) _GL_ATTRIBUTE_PURE;
size_t hash_get_n_entries (const Hash_table *) _GL_ATTRIBUTE_PURE;
size_t hash_get_max_bucket_length (const Hash_table *) _GL_ATTRIBUTE_PURE;
bool hash_table_ok (const Hash_table *) _GL_ATTRIBUTE_PURE;
void hash_print_statistics (const Hash_table *, FILE *);
void *hash_lookup (const Hash_table *, const void *, Hash_lookup_action);

/* Walking.  */
void *hash_get_first (const Hash_table *) _GL_ATTRIBUTE_PURE;
void *hash_get_next (const Hash_table *, const void *);
size_t hash_get_entries (const Hash_table *, void **, size_t);
size_t hash_do_for_each (const Hash_table *, Hash_processor, void *);

/* Allocation and clean-up.  */
size_t hash_string (const char *, size_t) _GL_ATTRIBUTE_PURE;
void hash_reset_tuning (Hash_tuning *);
int hash_initialize (Hash_table **, size_t, const Hash_tuning *,
		Hash_hasher, Hash_comparator, Hash_data_freer, bool use_nvm) _GL_ATTRIBUTE_WUR;
void hash_clear (Hash_table *);
void hash_free (Hash_table **);

/* Insertion and deletion.  */
//bool hash_rehash (Hash_table *, size_t) _GL_ATTRIBUTE_WUR;  //PJH: don't want this external
void *hash_insert (Hash_table *, const void *) _GL_ATTRIBUTE_WUR;

/* Deprecate this interface.  It has been renamed to hash_insert_if_absent.  */
int hash_insert0 (Hash_table *table, /* FIXME: remove in 2013 */
		const void *entry,
		const void **matched_ent) _GL_ATTRIBUTE_DEPRECATED;
int hash_insert_if_absent (Hash_table *table, const void *entry,
		const void **matched_ent);
void *hash_delete (Hash_table *, const void *);

#endif
