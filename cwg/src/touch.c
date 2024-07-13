/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Touch utility to replace the broken one in OpenWatcom.
 * utime() is not used as it does not work.
 */

/* standard C headers */
#include <stdio.h>
#include <sys/utime.h>

/**
 * Main (and only function).
 * @param argc The number of arguments on the command line.
 * @param argv The arguments on the command line.
 * @return     0 for success, !0 for failure.
 */
int main (int argc, char **argv)
{
    FILE *fh; /* file handle */
    int c; /* the first character of the file, if it exists */

    /* return with usage message if no filename given;
       return an error code to alert 'make' of a potential mistake */
    if (argc != 2) {
	printf ("usage: %s FILENAME\n", argv[0]);
	return 1;
    }

    /* cannot open file - create it */
    if (! (fh = fopen (argv[1], "r+"))) {
	fh = fopen (argv[1], "a");
	fclose (fh);
	return 0;
    }

    /* attempt to rewrite the first character */
    else {
	fseek (fh, 0l, SEEK_SET);

	/* file has no character - close and recreate it */
	if ((c = fgetc (fh)) == EOF) {
	    fclose (fh);
	    fh = fopen (argv[1], "w");
	    fclose (fh);
	    return 0;
	}

	/* otherwise rewrite the first character */
	fseek (fh, 0l, SEEK_SET);
	fputc (c, fh);
	fclose (fh);
    }

    /* everything went fine */
    return 0;
}
