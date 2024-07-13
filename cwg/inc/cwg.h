/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Jul-2020.
 *
 * Header for the CWG object and the library as a whole.
 */

#ifndef __CWG_H__
#define __CWG_H__

/*----------------------------------------------------------------------
 * Defined Constants
 */

/* unit types */
#define CWG_UTYPES 8

/* terrain types */
#define CWG_TERRAIN 8

/* units */
#define CWG_UNITS 32

/* value for no unit */
#define CWG_NO_UNIT 255

/* maximum length of a string, including terminator */
#define CWG_MAXLEN 32

/*----------------------------------------------------------------------
 * Includes.
 */

/* standard C headers */
#include <stdio.h>

/* project headers */
#include "utype.h"
#include "terrain.h"
#include "map.h"
#include "unit.h"
#include "battle.h"
#include "player.h"
#include "computer.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct cwg
 * The CWG object.
 */
typedef struct cwg Cwg;
struct cwg {

    /**
     * Destroy CWG when it is no longer needed.
     */
    void (*destroy) (void);

    /**
     * Write an integer as a byte to an already open output file.
     * @param  value  A pointer to the integer variable to write.
     * @param  output The output file handle.
     * @return        1 if successful, 0 if not.
     */
    int (*writeint) (int *value, FILE *output);

    /**
     * Write a string to an already open output file.
     * @param  value  A pointer to the string buffer to write.
     * @param  output The output file handle.
     * @return        1 if successful, 0 if not.
     */
    int (*writestring) (char *value, FILE *output);

    /**
     * Read a byte from an already open input file
     * and store it in an integer variable.
     * @param  value A pointer to the integer variable to store into.
     * @param  input The input file handle.
     * @return       1 if successful, 0 if not.
     */
    int (*readint) (int *value, FILE *input);

    /**
     * Read a string from an already open input file
     * and store it in a string buffer.
     * @param  value A pointer to the string buffer to store into.
     * @param  input The input file handle.
     * @return       1 if successful, 0 if not.
     */
    int (*readstring) (char *value, FILE *input);

    /**
     * Open the debug log file.
     */
    void (*debug_open) (void);

    /**
     * Log a single line to the log file.
     * @param string The string to log.
     */
    void (*debug_log) (char *message);

    /**
     * Close the debug log file.
     */
    void (*debug_close) (void);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Constructor for the CWG object.
 * @return A pointer to the new CWG.
 */
Cwg *get_Cwg (void);

#endif
