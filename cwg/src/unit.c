/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 02-Sep-2020.
 *
 * Unit Module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"
#include "unit.h"

/*----------------------------------------------------------------------
 * Public Methods.
 */

/**
 * Destroy a unit when no longer needed.
 * @param unit is the unit to destroy.
 */
static void destroy (Unit *unit)
{
    if (unit)
	free (unit);
}

/**
 * Clone a unit.
 * @param unit is the unit to clone.
 * @return the new unit.
 */
static Unit *clone (Unit *unit)
{
    /* local variables */
    Unit *newunit; /* the new unit to return */

    /* reserve memory */
    if (! unit)
	return NULL;
    if (! (newunit = new_Unit ()))
	return NULL;

    /* copy across the unit details */
    strcpy (newunit->name, unit->name);
    newunit->side = unit->side;
    newunit->utype = unit->utype;
    newunit->x = unit->x;
    newunit->y = unit->y;
    newunit->hits = unit->hits;
    newunit->moves = unit->moves;

    /* return the new unit */
    return newunit;
}

/**
 * Write a unit to a file.
 * @param unit is the unit to write.
 * @param output is the output file handle.
 * @return 1 if successful, 0 if not.
 */
static int write (Unit *unit, FILE *output)
{
    char c; /* character count */

    /* write the name */
    c = strlen (unit->name);
    fwrite (&c, 1, 1, output);
    fwrite (unit->name, c, 1, output);

    /* write the numeric data */
    fwrite (&unit->side, sizeof (int), 1, output);
    fwrite (&unit->utype, sizeof (int), 1, output);
    fwrite (&unit->x, sizeof (int), 1, output);
    fwrite (&unit->y, sizeof (int), 1, output);
    fwrite (&unit->hits, sizeof (int), 1, output);
    fwrite (&unit->moves, sizeof (int), 1, output);

    /* success */
    return 1;
}

/**
 * Read a unit from the file.
 * @param unit is the unit to read.
 * @param input is the input file handle.
 * @return 1 if successful, 0 if not.
 */
static int read (Unit *unit, FILE *input)
{
    unsigned char c; /* character count */

    /* read the name */
    fread (&c, 1, 1, input);
    fread (unit->name, c, 1, input);
    unit->name[(int) c] = '\0';

    /* read the numeric data */
    fread (&unit->side, sizeof (int), 1, input);
    fread (&unit->utype, sizeof (int), 1, input);
    fread (&unit->x, sizeof (int), 1, input);
    fread (&unit->y, sizeof (int), 1, input);
    fread (&unit->hits, sizeof (int), 1, input);
    fread (&unit->moves, sizeof (int), 1, input);

    /* success */
    return 1;
}

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Standard constructor.
 * @return the newly initialised unit.
 */
Unit *new_Unit (void)
{
    /* local variables */
    Unit *unit; /* the unit to return */

    /* reserve memory */
    if (! (unit = malloc (sizeof (Unit))))
	return NULL;

    /* initialise attributes */
    *unit->name = '\0';
    unit->utype = -1;
    unit->side = 0;
    unit->x = 0;
    unit->y = 0;
    unit->hits = 0;
    unit->moves = 0;

    /* initialise methods */
    unit->destroy = destroy;
    unit->clone = clone;
    unit->write = write;
    unit->read = read;

    /* return the new unit */
    return unit;
}
