/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 02-Sep-2020.
 *
 * Map Module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"
#include "map.h"

/*----------------------------------------------------------------------
 * Public Methods.
 */

/**
 * Destroy a map when it is no longer needed.
 * @param map is the map to destroy.
 */
static void destroy (Map *map)
{
    if (map) {
	if (map->terrain)
	    free (map->terrain);
	if (map->units)
	    free (map->units);
	if (map->points)
	    free (map->points);
	free (map);
    }
}

/**
 * Clone the current map.
 * @param map is the map to clone.
 * @return the cloned map.
 */
static Map *clone (Map *map)
{
    /* local variables */
    Map *newmap; /* the new map to return */
    int s; /* square counter for copying */

    /* reserve memory for the new map */
    if (! map)
	return NULL;
    if (! (newmap = new_Map ()))
	return NULL;

    /* set the width and height */
    if (! newmap->size (newmap, map->width, map->height)) {
	free (newmap);
	return NULL;
    }

    /* copy the map contents */
    for (s = 0; s < newmap->width * newmap->height; ++s) {
	newmap->terrain[s] = map->terrain[s];
	newmap->units[s] = map->units[s];
	newmap->points[s] = map->points[s];
    }

    /* return the new map */
    return newmap;
}

/**
 * Write the map to an already open file.
 * @param map is the map to write.
 * @param output is the output file.
 * @return 1 on success, 0 on failure.
 */
static int write (Map *map, FILE *output)
{
    int s = 1; /* success flat */
    Cwg *cwg; /* pointer to the Cwg object */

    /* get the Cwg object */
    cwg = get_Cwg ();
    
    /* write the map dimensions */
    s &= cwg->writeint (&map->width, output);
    s &= cwg->writeint (&map->height, output);

    /* write the terrain, units and points */
    if (map->width && map->height) {
	s &= fwrite (map->terrain, map->width * map->height, 1, output);
	s &= fwrite (map->units, map->width * map->height, 1, output);
	s &= fwrite (map->points, map->width * map->height, 1, output);
    }

    /* return */
    return s;
}

/**
 * Read the map from an already open file.
 * @param map is the map to read.
 * @param input is the input file.
 * @return 1 on success, 0 on failure.
 */
static int read (Map *map, FILE *input)
{
    int s = 1; /* success flag */
    Cwg *cwg; /* pointer to the Cwg object */

    /* get the Cwg object */
    cwg = get_Cwg ();
    
    /* read the map dimensions */
    s &= cwg->readint (&map->width, input);
    s &= cwg->readint (&map->height, input);
    map->size (map, map->width, map->height);

    /* read the terrain, units and points */
    if (map->width && map->height) {
	s &= fread (map->terrain, map->width * map->height, 1, input);
	s &= fread (map->units, map->width * map->height, 1, input);
	s &= fread (map->points, map->width * map->height, 1, input);
    }

    /* return */
    return s;
}

/**
 * Set the size of the map, clearing any current content.
 * @param map is the map to resize.
 * @param width is the new width.
 * @param height is the new height.
 * @return 0 on failure or 1 on success.
 */
static int size (Map *map, int width, int height)
{
    /* local variables */
    int size, /* total size of the map */
	s; /* square counter */

    /* calculate and validate size */
    size = width * height;
    if (width < 9 || height < 9 || size > 256)
	return 0;

    /* allocate memory */
    if (map->terrain)
	free (map->terrain);
    if (! (map->terrain = calloc (width * height, 1)))
	return 0;
    if (map->units)
	free (map->units);
    if (! (map->units = calloc (width * height, 1)))
	return 0;
    if (map->points)
	free (map->points);
    if (! (map->points = calloc (width * height, 1)))
	return 0;

    /* initialise the arrays */
    for (s = 0; s < width * height; ++s) {
	map->terrain[s] = 0;
	map->units[s] = CWG_NO_UNIT;
	map->points[s] = 0;
    }

    /* set the map dimensions */
    map->width = width;
    map->height = height;

    /* return success */
    return 1;
}

/**
 * Return the distance between two points.
 * @param  ox   The origin x coordinate.
 * @param  oy   The origin y coordinate.
 * @param  dx   The destination x coordinate.
 * @param  dy   The destination y coordinate.
 * @param  taxi 1 for taxicab distance
 * @return      The distance between the two points.
 */
static int distance (int ox, int oy, int dx, int dy, int taxi)
{
    int hd, /* horizontal distance */
	vd; /* vertical distance */
    hd = abs (ox - dx);
    vd = abs (oy - dy);
    if (taxi)
	return hd + vd;
    else if (hd > vd)
	return hd;
    else
	return vd;
}

/*----------------------------------------------------------------------
 * Constructor and any other class methods.
 */

/**
 * Default constructor.
 * @returns a new Map object.
 */
Map *new_Map (void)
{
    /* local variables */
    Map *map; /* the map to create */

    /* reserve memory */
    if (! (map = malloc (sizeof (Map))))
	return NULL;

    /* initialise attributes */
    map->width = 0;
    map->height = 0;
    map->terrain = NULL;
    map->units = NULL;
    map->points = NULL;

    /* initialise methods */
    map->destroy = destroy;
    map->clone = clone;
    map->write = write;
    map->read = read;
    map->size = size;
    map->distance = distance;

    /* return the new map */
    return map;
}
