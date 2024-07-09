/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 24-Jan-2021.
 *
 * Declarations required throughout the project.
 */

#ifndef __BARREN_H__
#define __BARREN_H__

/* project headers */
#include "controls.h"
#include "display.h"
#include "config.h"

/*----------------------------------------------------------------------
 * Public Level Routines.
 */

/**
 * Set the quit flag from another module.
 * @param newquitflag is the new quit flag.
 */
void setquitflag (int newquitflag);

/**
 * Share the control handler with other modules.
 * @return A pointer to the game control handler.
 */
Controls *getcontrols (void);

/**
 * Share the display handler with other modules.
 * @return A pointer to the display module.
 */
Display *getdisplay (void);

/**
 * Share the config handler with other modules.
 * @return A pointer to the config module.
 */
Config *getconfig (void);

#endif
