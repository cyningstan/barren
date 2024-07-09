/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Touch utility to replace the broken one in OpenWatcom.
 *
 * Please note that the utime() function doesn't change the file's time
 * stamp either, regardless of what the manual says.
 */

#include <stdio.h>
#include <sys/utime.h>

int main (int argc, char **argv)
{
    FILE *fh;
    int c;
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
    return 0;
}
