/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Jul-2020.
 *
 * Unit Type Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"
#include "utype.h"

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Destroy a unit type when it is no longer needed.
 * @param utype is the unit type to destroy.
 */
static void destroy (UnitType *utype)
{
    if (utype)
	free (utype);
}

/**
 * Write the unit type to an already open file.
 * @param utype is the unit type to write.
 * @param output is the output file.
 * @return 1 on success, 0 on failure.
 */
static int write (UnitType *utype, FILE *output)
{
    /* local variables */
    int u, /* unit type counter */
	s = 1; /* success flag */
    Cwg *cwg; /* cwg object */

    /* grab the CWG object */
    cwg = get_Cwg ();

    /* write the details out, returning 0 on failure */
    s &= cwg->writestring (utype->name, output);
    s &= cwg->writeint (&utype->cost, output);
    s &= cwg->writeint (&utype->hits, output);
    s &= cwg->writeint (&utype->power, output);
    s &= cwg->writeint (&utype->range, output);
    s &= cwg->writeint (&utype->armour, output);
    s &= cwg->writeint (&utype->moves, output);

    /* write the build list */
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->writeint (&utype->builds[u], output);

    /* all successful */
    return s;
}

/**
 * Read the unit type from an already open file.
 * @param utype is the unit type to read.
 * @param input is the input file.
 * @return 1 on success, 0 on failure.
 */
static int read (UnitType *utype, FILE *input)
{
    /* local variables */
    int u, /* unit type counter */
	s = 1; /* success flag */
    Cwg *cwg; /* cwg object */

    /* grab the CWG object */
    cwg = get_Cwg ();

    /* read the details */
    s &= cwg->readstring (utype->name, input);
    s &= cwg->readint (&utype->cost, input);
    s &= cwg->readint (&utype->hits, input);
    s &= cwg->readint (&utype->power, input);
    s &= cwg->readint (&utype->range, input);
    s &= cwg->readint (&utype->armour, input);
    s &= cwg->readint (&utype->moves, input);

    /* read the build list */
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->readint (&utype->builds[u], input);

    /* all successful */
    return s;
}

/*----------------------------------------------------------------------
 * Public Constructors and Other Class Functions.
 */

/**
 * Standard constructor.
 * @returns the a newly initialised unit type.
 */
UnitType *new_UnitType (void)
{
    /* local variables */
    UnitType *utype; /* the unit type to return */
    int u; /* unit type counter to initialise builds */

    /* reserve memory */
    if (! (utype = malloc (sizeof (UnitType))))
	return NULL;

    /* initialise data */
    *utype->name = '\0';
    utype->cost = 0;
    utype->hits = 0;
    utype->power = 0;
    utype->range = 0;
    utype->armour = 0;
    utype->moves = 0;
    for (u = 0; u < CWG_UTYPES; ++u)
	utype->builds[u] = 0;

    /* initialise methods */
    utype->destroy = destroy;
    utype->write = write;
    utype->read = read;

    /* return the new object */
    return utype;
}
