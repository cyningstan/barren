/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 02-Sep-2020.
 *
 * Map Header.
 */

/* types defined in this header */
typedef struct map Map;

#ifndef __MAP_H__
#define __MAP_H__

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct map defines everything about the map.
 */
struct map {

    /*
     * Attributes
     */

    /** @var width is the width of the map. */
    int width;

    /** @var height is the height of the map. */
    int height;

    /** @var terrain is a grid of terrain types. */
    unsigned char *terrain;

    /** @var units is a grid of units. */
    unsigned char *units;

    /** @var points is a grid of victory points */
    unsigned char *points;

    /*
     * Methods
     */

    /**
     * Destroy a map when it is no longer needed.
     * @param map is the map to destroy.
     */
    void (*destroy) (Map *map);

    /**
     * Clone the current map.
     * @param map is the map to clone.
     * @return the cloned map.
     */
    Map *(*clone) (Map *map);

    /**
     * Write the map to an already open file.
     * @param map is the map to write.
     * @param output is the output file.
     * @return 1 on success, 0 on failure.
     */
    int (*write) (Map *map, FILE *output);
    
    /**
     * Read the map from an already open file.
     * @param map is the map to read.
     * @param input is the input file.
     * @return 1 on success, 0 on failure.
     */
    int (*read) (Map *map, FILE *input);

    /**
     * Set the size of the map, clearing any current content.
     * @param map is the map to resize.
     * @param width is the new width.
     * @param height is the new height.
     * @return 0 on failure or 1 on success.
     */
    int (*size) (Map *map, int width, int height);

    /**
     * Return the distance between two points.
     * @param  ox   The origin x coordinate.
     * @param  oy   The origin y coordinate.
     * @param  dx   The destination x coordinate.
     * @param  dy   The destination y coordinate.
     * @param  taxi 1 for taxicab distance
     * @return      The distance between the two points.
     */
    int (*distance) (int ox, int oy, int dx, int dy, int taxi);

};

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Default constructor.
 * @return a new Map object.
 */
Map *new_Map (void);

#endif
