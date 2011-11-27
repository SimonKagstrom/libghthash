/*********************************************************************
 *
 * Copyright (C) 2002-2005,  Simon Kagstrom
 *
 * Filename:      alloc_example.c
 * Description:   A program that demonstrates the use of custom
 *                allocation functions.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * $Id: alloc_example.c 8461 2006-06-04 08:13:06Z ska $
 *
 ********************************************************************/

#include <stdlib.h>    /* malloc */
#include <string.h>    /* strncpy */
#include <unistd.h>    /* ssize_t */
#include <stdio.h>     /* File reading etc. */
#include <sys/types.h> /* stat */
#include <sys/stat.h>  /* stat */
#include <assert.h>    /* assert */
#include <stddef.h>

#include "ght_hash_table.h"

/* Delimiters between words */
#define DELIMS " \".,;:?!-\'/*()=+&%[]#$\n\r"

#define NR_ELEMS 30
#define ELEM_SIZE (sizeof(ght_hash_entry_t)+sizeof(int))

/*
 * The allocation function calls the normal malloc to get a large
 * allocation chunks of many elements, and then hands these out on
 * alloc() calls.
 *  1               NR_ELEMS
 *  ____________..___
 * |  |XX|  |  |  |XX|
 * |__|XX|__|__|..|XX|
 *        ...   _____\____ Already used elements
 *  ___________/..___
 * |  |  |XX|XX|  |  |
 * |__|__|XX|XX|..|__|
 *
 * The trick we use here is that we know the size of the allocations,
 * which is just the entries plus the size of the key, the allocation
 * function can therefore be made simple and fast.
 *
 * Please note that this example might be pretty unrealistic, since it
 * does not really use the table in a meaningful way. Also, this is a
 * demonstration of custom allocators, not a thought-through malloc
 * implementation. I'm actually not even sure it really works ;-)
 */

/* A structure for the returned data from malloc */
typedef struct
{
  char elem[ELEM_SIZE];
  int  nr;
} elem_t;

/* One allocation chunk */
typedef struct s_alloc
{
  elem_t          elems[NR_ELEMS];
  ght_uint32_t    bitmask;
  struct s_alloc *p_next;
  struct s_alloc *p_prev;
} alloc_t;

alloc_t *p_freelist=NULL;


/* The allocate function to use */
void *my_alloc(size_t size)
{
  int i;

  assert(size == ELEM_SIZE);

  /* No free chunks, allocate a new one */
  if ( !p_freelist )
    {
      if ( !(p_freelist = (alloc_t*)malloc(sizeof(alloc_t))) )
	return (void*)NULL;
      memset(p_freelist, 0, sizeof(alloc_t));
    }

  /* Find a free entry */
  for (i=0; i<NR_ELEMS; i++)
    {
      if ( (p_freelist->bitmask & (1<<i)) == 0)
	{
	  void *p_ret = (void*)&p_freelist->elems[i].elem;

	  /* Found a free spot */
	  p_freelist->bitmask |= (1<<i);
	  p_freelist->elems[i].nr = i;

	  if ( p_freelist->bitmask == (1<<NR_ELEMS)-1 )
	    p_freelist = p_freelist->p_next; /* This entry is full */

	  return p_ret;
	}
    }

  /* We cannot be here */
  return NULL;
}

/* The free function to use */
void my_free(void *p)
{
  elem_t *p_elem = (elem_t *)p;
  alloc_t *p_alloc = (alloc_t*) (p_elem-p_elem->nr);

  if ( (p_alloc->bitmask & ((1<<NR_ELEMS)-1) ) == ((1<<NR_ELEMS)-1) )
    {
      /* There were no free elements here before, insert first into freelist */
      p_alloc->p_next = p_freelist;
      p_alloc->p_prev = NULL;
      p_freelist->p_prev = p_alloc;
      p_freelist = p_alloc;
    }
  else if ( p_alloc->bitmask == ((unsigned int)1<<p_elem->nr) )
    {
      alloc_t *p_prev = p_alloc->p_prev;
      alloc_t *p_next = p_alloc->p_next;

      /* There was only one entry (now removed), free this chunk */
      if (p_prev)
	p_prev->p_next = p_next;
      else
	p_freelist = p_next; /* First in list */

      if (p_next)
	p_next->p_prev = p_prev;

      free(p_alloc);

      return;
    }

  /* There are some free, some non-free entries. Mark this as free. */
  p_alloc->bitmask &= ~(1<<p_elem->nr);
}

/*
 * This is an example program that reads words from a text-file (a
 * book or something like that) and uses those as data in a hash
 * table. The words are case sensitive.
 *
 * The meaning with this program is to test the speed of the table and
 * to provide an example of its use.
 */
int main(int argc, char *argv[])
{
  FILE *p_file;
  char *p_dict_name;
  ght_hash_table_t *p_table;
  ght_iterator_t iterator;
  struct stat stat_struct;
  int i_std_malloc=0;
  int i_cnt;
  char *p_buf;
  char *p_tok;
  const void *p_key;
  void *p_e;

  /* Create a new hash table */
  if ( !(p_table = ght_create(1000)) )
    {
      return 1;
    }
  ght_set_rehash(p_table, TRUE);

  /* Parse the arguments */
  if (argc < 2)
    {
      printf("Usage: alloc_example [-m|-t|-s] textfile\n");
      return 0;
    }
  else if (argc > 2)
    {
      if(strcmp(argv[1], "-m"))
	ght_set_heuristics(p_table, GHT_HEURISTICS_MOVE_TO_FRONT);
      else if(strcmp(argv[1], "-t"))
	ght_set_heuristics(p_table, GHT_HEURISTICS_TRANSPOSE);
      p_dict_name = argv[2];
    }
  else
    {
      /* 2 arguments */
      p_dict_name = argv[1];
    }

  /* Set the allocation/deallocation functions for the table */
  if (!i_std_malloc)
    {
      ght_set_alloc(p_table, my_alloc, my_free);
    }

  /* Open the dictionary file (first check its size) */
  if (stat(p_dict_name, &stat_struct) < 0)
    {
      perror("stat");
      return 1;
    }
  if (!(p_buf = (char*)malloc(stat_struct.st_size)))
    {
      perror("malloc");
      return 1;
    }

  /* Open the dictionary file and read that into the buffer. */
  if (! (p_file = fopen(p_dict_name, "r")) )
    {
      perror("fopen");
      return 1;
    }
  fread(p_buf, sizeof(char), stat_struct.st_size, p_file);
  fclose(p_file);

  /* For each word in the dictionary file, insert it into the hash table. */
  p_tok = strtok(p_buf, DELIMS);
  i_cnt = 0;
  while (p_tok)
    {
      char *p_data;

      if ( !(p_data = (char*)malloc( strlen(p_tok)+1 )) )
	{
	  perror("malloc");
	  return 1;
	}
      strcpy(p_data, p_tok);
      p_data[strlen(p_tok)] = '\0';

      /* Insert the word into the table */
      if (ght_insert(p_table,
		     p_data,
		     sizeof(int), &i_cnt) < 0)
	{
	  free(p_data); /* Could not insert the item (already exists), free it. */
	}

      p_tok = strtok(NULL, DELIMS);
      i_cnt++;
    }
  printf("Done reading %d words from the wordlist.\n", i_cnt);

  free(p_buf);

  /* Free the entry data in the table */
  for(p_e = ght_first(p_table, &iterator, &p_key); p_e; p_e = ght_next(p_table, &iterator, &p_key))
    {
      free(p_e);
    }

  /* Free the table */
  ght_finalize(p_table);

  return 0;
}
