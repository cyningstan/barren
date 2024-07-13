/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Demonstration Program. Loads and plays Hall of the Mountain King.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "tune.h"
#ifdef KEYLIB
#include "keylib.h"
#endif

/*----------------------------------------------------------------------
 * Data definitions.
 */

/** @var tune is the tune to play */
static Tune *tune;

/** @var keys is the key handler */
#ifdef KEYLIB
static KeyHandler *keys = NULL;
#else
static void *keys = NULL;
#endif

/*----------------------------------------------------------------------
 * Level 1 routines.
 */

/**
 * Initialise the program by loading the named tune.
 * @param argc is the argument count.
 * @param argv is the argument array.
 * @return 1 on success, 0 on error.
 */
static int initialise (int argc, char **argv)
{
    FILE *input; /* the input file */
    char header[8]; /* header read from file */
    int success = 1;

    /* create and load the tune */
    if (! (tune = new_Tune ())) {
	printf ("Not enough memory to load tune.\n");
	return 0;
    } else if (! (input = fopen ("demo.tun", "rb"))) {
	printf ("Cannot open file DEMO.TUN.\n");
	return 0;
    } else if (! fread (header, 8, 1, input)) {
	printf ("Invalid file DEMO.TUN.\n");
	success = 0;
    } else if (strcmp (header, "SPK100T")) {
	printf ("Invalid file DEMO.TUN.\n");
	success = 0;
    } else if (! tune->read (tune, input)) {
	printf ("Invalid file DEMO.TUN.\n");
	success = 0;
    }

    /* initialise the key handler */
    #ifdef KEYLIB
    keys = new_KeyHandler ();
    #endif

    /* ready to play! */
    fclose (input);
    return success;
}

/**
 * Clean up after playing a tune.
 */
void cleanup (void)
{
    #ifdef KEYLIB
    keys->destroy ();
    #endif
}

/*----------------------------------------------------------------------
 * Top level routines.
 */

/**
 * Main program.
 * @param argc is the argument count.
 * @param argv is the argument array.
 * @return 0 on success, >0 on error.
 */
int main (int argc, char **argv)
{
    if (! initialise (argc, argv))
	return 1;
    tune->play (tune, keys);
    cleanup ();
    return 0;
}
