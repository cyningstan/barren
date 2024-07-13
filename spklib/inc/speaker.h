/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Main Header.
 */

#ifndef __SPEAKER_H__
#define __SPEAKER_H__

/* include module headers. */
#include "tune.h"
#include "effect.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* forward declarations of types */
typedef struct speaker Speaker;

/** @struct spreaker The overall speaker object. */
struct speaker {

    /** @var frequencies The frequency table. */
    int *frequencies;

    /**
     * Destroy the speaker object when it is no longer needed.
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
     * Read a byte from an already open input file
     * and store it in an integer variable.
     * @param  value A pointer to the integer variable to store into.
     * @param  input The input file handle.
     * @return       1 if successful, 0 if not.
     */
    int (*readint) (int *value, FILE *input);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Constructor for the Speaker object.
 * @return A pointer to the new Speaker.
 */
Speaker *get_Speaker (void);

#endif
