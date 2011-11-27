/*
** A example program to interactively insert, delete, search items to
** hash table. Written for libghthash by Simon Kagstrom, ska@bth.se.
**
** This example is written by: Muhammad A Muquit, muquit@muquit.com
**
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "ght_hash_table.h"

/* print menu to stdout */
static void print_menu(void)
{
    static char
	**p;

    static char *menu[]=
    {
	"i  - Insert data to hash table",
	"r  - Replace an entry in the hash table",
	"l  - Search data in hash table",
	"n  - Show number of elements in hash table",
	"s  - Show hash table size",
	"t  - Print content of hash table",
	"d  - Remove item with Key from hash table",
	"h  - Print this menu",
	"q  - Quit",
	NULL,
    };

    for (p=menu; *p; p++)
    {
	(void) fprintf(stdout,"%s\n",*p);
    }
    (void) fflush(stdout);
}

static void print_it(char *fmt,...)
{
    va_list
	args;
    va_start(args,fmt);

    vfprintf(stdout,fmt,args);
    (void)fflush(stdout);

    va_end(args);
}

static int chop_nl(char *string)
{
    int
	c;

    char
	*ptr;

    c='\0';

    for (ptr = string; *ptr; ptr++);

    if (ptr != string)
    {
	c = *(--ptr);

	if (c == '\n')
	{
	    *ptr='\0';
	}
    }
    return(c);
}

static void print_prompt(void)
{
    (void) fprintf(stdout,"\n> ");
    (void) fflush(stdout);
}

/**
 * read data from stdin, remove new line.
 * strdup it and return pointer to it. NULL otherwise
 * if not NULL, caller must free the memory
 */
char *read_data(char *prompt)
{
    char
	*data,
	*l,
	buf[BUFSIZ];

    (void) fprintf(stdout,"%s",prompt);
    (void) fflush(stdout);

    l=fgets(buf,sizeof(buf)-1,stdin);
    if (l && *l != '\n')
    {
	data=strdup(l);
	chop_nl(data);
	return(data);
    }

    return(NULL);

}

int main(int argc,char *argv[])
{
    int
	rc;

    char
	*k,
	*v,
	*line,
	buf[BUFSIZ];

    ght_hash_table_t
	*p_table=NULL;

    /* create hash table of size 256 */
    p_table=ght_create(256);
    if (p_table == NULL)
    {
	print_it("Error: Could not create hash table\n");
	return(1);
    }

    print_menu();
    for (;;)
    {
	print_prompt();


	line=fgets(buf,sizeof(buf)-1,stdin);
	if (line == NULL)
	    break;

	switch(*line)
	{
	    case 'i':
	    {
		k=read_data("Enter key: ");
		v=read_data("Enter value: ");
		if (k && v)
		{
		    /* insert to hash table */
		    rc=ght_insert(p_table,
			    v,
			    strlen(k),
			    k);
		    if (rc == 0)
		    {
			print_it("Inserted: Key=%s, Value=%s\n",
				k,v);
			free(k);
		    }
		    else
		    {
			print_it("Error: ght_insert() failed\n");
			(void) free((char *) k);
			(void) free((char *) v);
		    }
		}
		else
		{
		    if (k)
			(void) free((char *) k);
		    if (v)
			(void) free((char *) v);

		}

		break;
	    }

	    case 'r': /* replace */
	    {
		k=read_data("Enter key: ");
		v=read_data("Enter new value: ");
		if (k && v)
		{
		    char *
			 old_val;
		    /* Replace a key in the hash table */
		    old_val = (char*)ght_replace(p_table,
						 v,
						 strlen(k),
						 k);
		    if (old_val != NULL)
		    {
			print_it("Replaced: Key=%s, Old Value=%s, Value=%s\n",
				k,old_val, v);
			free(old_val);
			free(k);
		    }
		    else
		    {
			print_it("Error: ght_replace() failed\n");
			(void) free((char *) k);
			(void) free((char *) v);
		    }
		}
		else
		{
		    if (k)
			(void) free((char *) k);
		    if (v)
			(void) free((char *) v);

		}
		break;
	    }

	    case 'h':
	    {
		print_menu();
		break;
	    }

	    case 'n': /* number of elements */
	    {
		print_it("Number elements in hash table: %d\n",
			ght_size(p_table));
		break;
	    }

	    case 's': /* size of hash table */
	    {
		print_it("Hash table size: %d\n",
			ght_table_size(p_table));

		break;
	    }

	    case 't': /* dump */
	    {
		const void
		    *pk,
		    *pv;

		ght_iterator_t
		    iterator;

		for (pv=(char *) ght_first(p_table,&iterator, &pk);
		     pv;
		     pv=(char *) ght_next(p_table,&iterator, &pk))
		{
		    print_it("%s => %s\n",(char *) pk,(char *) pv);
		}
		break;
	    }

	    case 'd':
	    {
		k=read_data("Enter key to delete: ");
		if (k)
		{
		    v=ght_remove(p_table,
			    strlen(k),
			    k);
		    if (v)
		    {
			print_it("Removed %s => %s",k,v);
			(void) free((char *) v);
		    }
		    else
		    {
			print_it("Error: could not find data for key %s\n",
				k);
		    }
		    (void) free((char *) k);
		}

		break;
	    }

	    case 'l':
	    {
		k=read_data("Enter key: ");
		if (k)
		{
		    v=(char *) ght_get(p_table,strlen(k),k);
		    if (v)
		    {
			print_it("Found: %s => %s\n",k,v);
		    }
		    else
		    {
			print_it("No data found for key: %s\n",k);
		    }
		    (void) free((char *) k);

		}
		break;
	    }

	    case 'q':
	    {
		if (p_table)
		{
		    const void
			*pv,
			*pk;

		    ght_iterator_t
			iterator;

		    for (pv=(char *) ght_first(p_table,&iterator, &pk);
			 pv;
			 pv=(char *) ght_next(p_table,&iterator, &pk))
		    {
			(void) free((char *) pv);
		    }
		    ght_finalize(p_table);
		}

		return(0);
		break;
	    }

	    default:
	    {
		print_it("Unknown option\n");
		break;
	    }
	}
    }
    return(0);
}
