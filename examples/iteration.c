/*********************************************************************
 *
 * Copyright (C) 2003,  Simon Kagstrom
 *
 * Filename:      iteration.c
 * Description:   An example program that shows iteration.
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
 * $Id: iteration.c 8461 2006-06-04 08:13:06Z ska $
 *
 ********************************************************************/
#include <string.h>          /* strlen */
#include <stdio.h>           /* printf */
#include <stdlib.h>          /* malloc */

/* Include the hash table (would normally be #include <ght_hash_table> but we
   want to be sure to include the one in this directory here) */
#include "ght_hash_table.h"

static void insert_entry(ght_hash_table_t *p_table, int num, char *p_key)
{
  int *p_data;

  if ( !(p_data = malloc(sizeof(int))) )
    {
      fprintf(stderr, "Malloc\n");
      exit(1);
    }
  *p_data = num;

  /* Insert the entry into the hash table */
  if (ght_insert(p_table,
		 p_data,
		 sizeof(char)*strlen(p_key), p_key) < 0)
    {
      /* Just exit if we cannot insert. */
      fprintf(stderr, "Could not insert into the hash table\n");
      exit(1);
    }
}

int main(int argc, char *argv[])
{
  ght_hash_table_t *p_table;
  ght_iterator_t iterator;
  const void *p_key;
  int *p_data;

  /* Create a new hash table */
  if ( !(p_table = ght_create(128)) )
    {
      fprintf(stderr, "Could not create hash table!\n");
      return 1;
    }

  /* Insert a number of entries */
  insert_entry(p_table, 0, "zero");
  insert_entry(p_table, 1, "one");
  insert_entry(p_table, 2, "two");
  insert_entry(p_table, 3, "three");
  insert_entry(p_table, 4, "four");
  insert_entry(p_table, 5, "five");
  insert_entry(p_table, 6, "six");
  insert_entry(p_table, 7, "seven");
  insert_entry(p_table, 8, "eight");

  for (p_data = (int*)ght_first(p_table, &iterator, &p_key); p_data;
       p_data = (int*)ght_next(p_table, &iterator, &p_key))
    {
      /* Print out the entry */
      printf("%s: \t%d\n", (char*)p_key, *p_data);
      /* Free the data (the meta-data will be removed in ght_finalize below) */
      free(p_data);
    }

  /* Remove the hash table */
  ght_finalize(p_table);

  return 0;
}
