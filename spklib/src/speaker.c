/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Main Speaker Object.
 */

/*----------------------------------------------------------------------
 * Includes
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* project headers */
#include "speaker.h"

/*----------------------------------------------------------------------
 * Data definitions.
 */

/** @var speaker The SPEAKER object. */
static Speaker *speaker = NULL;

/** @var frequencies The frequency table. */
static int frequencies[] = {
    16, 17, 18, 19, 21, 22, 23, 25, 26, 28, 29, 31,
    33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62,
    65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123,
    131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247,
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397,
    1480, 1568, 1661, 1760, 1865, 1976,
    2093, 2217, 2349, 2489, 2637, 2794,
    2960, 3136, 3322, 3520, 3729, 3951,
    4186, 4435, 4699, 4978, 5274, 5588,
    5920, 6272, 6645, 7040, 7459, 7902
};

/*----------------------------------------------------------------------
 * Public Method Definitions.
 */

/**
 * Destroy SPEAKER when it is no longer needed.
 */
static void destroy (void)
{
    if (speaker)
	free (speaker);
}

/**
 * Write an integer as a byte to an already open output file.
 * @param  value  A pointer to the integer variable to write.
 * @param  output The output file handle.
 * @return        1 if successful, 0 if not.
 */
static int writeint (int *value, FILE *output)
{
    unsigned char c; /* character to read */
    c = (char) (*value & 0xff);
    return fwrite (&c, 1, 1, output);
}

/**
 * Read a byte from an already open input file
 * and store it in an integer variable.
 * @param  value A pointer to the integer variable to store into.
 * @param  input The input file handle.
 * @return       1 if successful, 0 if not.
 */
static int readint (int *value, FILE *input)
{
    unsigned char c; /* character to read */
    if (! (fread (&c, 1, 1, input)))
	return 0;
    *value = (int) c;
    return 1;
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Constructor for the SPEAKER object.
 * @return A pointer to the new SPEAKER.
 */
Speaker *get_Speaker (void)
{
    /* only one Speaker should be in existence at once. */
    if (speaker)
	return speaker;

    /* try to reserve memory for the Speaker */
    if (! (speaker = malloc (sizeof (Speaker))))
	return NULL;

    /* initialise the attributes */
    speaker->frequencies = &frequencies;

    /* initialise the methods */
    speaker->destroy = destroy;
    speaker->readint = readint;
    speaker->writeint = writeint;

    /* return the new Speaker */
    return speaker;
}
