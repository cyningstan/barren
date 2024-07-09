/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker 2021
 * Game Controls Header
 */

/* Controls type definition */
typedef struct controls Controls;

#ifndef __CONTROLS_H__
#define __CONTROLS_H__

/* required headers */
#include "keylib.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** 
 * @struct controls
 * The game controls.
 */
struct controls {

    /*
     * Methods
     */

    /**
     * Destroy the keyboard handler when it is no longer needed.
     */
    void (*destroy) (void);

    /**
     * Expose the KeyHandler for other libraries that need it.
     * @return A pointer to the KeyHandler object.
     */
    KeyHandler *(*getkeyhandler) (void);

    /**
     * Key left indicator.
     * @return 1 if the left control is activated, 0 if not.
     */
    int (*left) (void);

    /**
     * Key right indicator.
     * @return 1 if the right control is activated, 0 if not.
     */
    int (*right) (void);

    /**
     * Up key indicator.
     * @return 1 if the up control is activated, 0 if not.
     */
    int (*up) (void);

    /**
     * Down key indicator.
     * @return 1 if the down control is activated, 0 if not.
     */
    int (*down) (void);

    /**
     * Fire key indicator.
     * @return 1 if the fire control is activated, 0 if not.
     */
    int (*fire) (void);

    /**
     * Wait for a key and return its ASCII value.
     * @return The ASCII value of the key.
     */
    int (*key) (void);

    /**
     * Wait for a key but leave its value to be checked later.
     */
    void (* wait) (void);

    /**
     * Wait for a key release or repeat delay.
     * @param msecs Number of milliseconds to wait, 0 forever.
     */
    void (*release) (int msecs);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct new game controls handler.
 * @return The new game controls handler.
 */
Controls *new_Controls (void);

#endif
