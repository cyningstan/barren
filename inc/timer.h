/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2022.
 * Timer Header.
 */

#ifndef __TIMER_H__
#define __TIMER_H__

/* required headers */
#include <sys/timeb.h>

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* advance type definitions */
typedef struct timer Timer;

/**
 * @struct timer
 * Object to time things.
 */
struct timer {

    /** @var end When the timer finished. */
    struct timeb end;

    /**
     * Wait for the timer to complete, then destroy it.
     * @param timer The timer to destroy.
     */
    void (*destroy) (Timer *timer);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Create a new timer and set it going.
 * @param  milliseconds The length of the timer.
 * @return              The new timer.
 */
Timer *new_Timer (int milliseconds);

#endif
