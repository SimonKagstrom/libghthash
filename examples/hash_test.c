/*********************************************************************
 *
 * Copyright (C) 2001-2002,  Simon Kagstrom
 *
 * Filename:      hash_test.c
 * Description:   A sample program to demonstrate the usage of the
 *                generic hash table.
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
 * $Id: hash_test.c 8461 2006-06-04 08:13:06Z ska $
 *
 ********************************************************************/

#include <stdlib.h> /* malloc */
#include <time.h>   /* time */
#include <errno.h>  /* errno */
#include <stdio.h>  /* perror etc */

#include "ght_hash_table.h" /* Include the generic hash table */

void fn(void *data, const void *key)
{
  free(data);
}

int main(int argc, char *argv[])
{
  ght_hash_table_t *p_table;
  ght_hash_table_t *p_table2;
  ght_iterator_t iterator;
  int i_loops = 100000;
  int i_removed = 0;
  const void *p_key;
  void *p_e;
  int i;

  /* Initialise the random seed */
  srand(1000);

  if (argc > 1)
    {
      /* Create two hash table with move to front heuristics, specifying the size for one. Both use automatic rehashing  */
      p_table = ght_create(atoi(argv[1]));
      ght_set_rehash(p_table, TRUE);
      ght_set_heuristics(p_table, GHT_HEURISTICS_MOVE_TO_FRONT);

      if (argc > 2)
	i_loops = atoi(argv[2]);

      p_table2 = ght_create(5000);

      ght_set_rehash(p_table2, TRUE);
      ght_set_heuristics(p_table2, GHT_HEURISTICS_MOVE_TO_FRONT);
    }
  else
    {
      /* Create two hash table with transpose heuristics, size
	 5000. Hash function is one_at_a_time_hash (default with NULL).
      */
      p_table = ght_create(5000);
      ght_set_rehash(p_table, FALSE);
      ght_set_heuristics(p_table, GHT_HEURISTICS_TRANSPOSE);

      p_table2 = ght_create(5000);
      ght_set_rehash(p_table2, FALSE);
      ght_set_heuristics(p_table2, GHT_HEURISTICS_TRANSPOSE);
    }

  /* This is a bounded bucket implementation */
  ght_set_bounded_buckets(p_table, 3, fn);

  /* Enter lots of stuff to the tables. */
  for (i=0; i<i_loops; i++)
    {
      int *p_entry_data;

      /* Allocate data for the entry as well (in this case an integer,
	 more often some structure).
      */
      if ( !(p_entry_data = (int*)malloc(sizeof(int))) )
	{
	  perror("malloc");
	  return -errno;
	}

      /* Set the data for the key and the entry */
      *p_entry_data = i_loops-i;

      /* Insert into table1 or table2 at random */
      if (rand()%2 == 1)
	{
	  if (ght_insert(p_table2,
			 p_entry_data,
			 sizeof(int), &i) < 0)
	    fprintf(stderr, "ERROR: Could not insert into table\n");
	}
      else
	{
	  if (ght_insert(p_table,
			 p_entry_data,
			 sizeof(int), &i) < 0)
	    fprintf(stderr, "ERROR: Could not insert into table\n");
	}

      /* If we didn't use automatic rehashing here, we could do a
       * manual rehash by using something like:
       *
       * if (p_table->i_items > p_table->i_size*2)
       *   ght_he_rehash(p_he);
       */
    }
  printf("Inserted %d elements.\n", i_loops);

  /* Get the same stuff FROM the table */
  for (i=0; i<i_loops; i++)
    {
      int *p_he;
      int i_key_data;

      /* Here we create a key for temporary usage (with one of the
	 numbers used previously)
      */
      i_key_data = rand()%i_loops;

      /* Get some element from one of the hash tables */
      if (rand()%2 == 1)
	p_he = (int*)ght_get(p_table,
			     sizeof(int), &i_key_data);
      else
	p_he = (int*)ght_get(p_table2,
			     sizeof(int), &i_key_data);

      /* Check if anything was found (might be in the other table) */
      if (p_he)
	{
	  /* Check if it's correct */
	  if (*p_he != i_loops-i_key_data)
	    {
	      printf("Found %d for key %d. WRONG!\n", *p_he, i_key_data);
	      return -1;
	    }
	}
    }

#ifdef USE_PROFILING
  printf("Hash table 1:\n");
  ght_print(p_table); /* Do not rely on this function, only available when built without optimization */
  printf("Hash table 2:\n");
  ght_print(p_table2);
  printf("\n");
#endif
  printf("Fetched %d elements. All tested OK\n", i_loops);

  /* Remove the same stuff FROM the table */
  for (i=0; i<i_loops; i++)
    {
      int *p_he;
      int i_key_data;

      /* Again, use a temporary (non-allocated) key */
      i_key_data = rand()%i_loops;

      /* Remove an element from one of the hash tables */
      if (rand()%2 == 1)
	p_he = (int*)ght_remove(p_table,
				sizeof(int), &i_key_data);
      else
	p_he = (int*)ght_remove(p_table2,
				sizeof(int), &i_key_data);

      /* Check if something could be removed */
      if (p_he)
	{
	  if (*p_he != i_loops-i_key_data)
	    {
	      printf("Found %d for key %d. WRONG!\n", *p_he, i_key_data);
	      return -1;
	    }

	  free(p_he);
	  p_he = NULL;
	  i_removed++;
	}
    }
  printf("Removed %d elements. All tested OK\n", i_removed);

#ifdef USE_PROFILING
  printf("Hash table 1:\n");
  ght_print(p_table);
  printf("Hash table 2:\n");
  ght_print(p_table2);
  printf("\n");
#endif


  /* Finally, remove the hash tables and all data within them (this
     will take some time if the tables are large). */
  printf("Finalizing the hash tables (remove all data)...\n");

  /* Free the entry data in the tables */
  for(p_e = ght_first(p_table, &iterator, &p_key); p_e; p_e = ght_next(p_table, &iterator, &p_key))
    {
      void *p;

      /* Remove the current entry from the table as well (will be done
       * in ght_finalize below so this is not strictly necessary).
       */
      p = ght_remove(p_table, sizeof(int), p_key);
      if (!p)
	printf("Removing the current iterated entry failed! This is a BUG\n");

      free(p_e);
    }
  for(p_e = ght_first(p_table2, &iterator, &p_key); p_e; p_e = ght_next(p_table2, &iterator, &p_key))
    {
      free(p_e);
    }

  /* Free the tables */
  ght_finalize(p_table);
  ght_finalize(p_table2);
  printf("Finalizing done\n");

  return 0;
}
