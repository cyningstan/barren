/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 26-Sep-2020.
 *
 * Computer Player Header.
 */

/* types defined in this header */
typedef struct player ComputerPlayer;

#ifndef __COMPUTER_H__
#define __COMPUTER_H__

/*----------------------------------------------------------------------
 * Constructor Prototypes.
 */

/**
 * Constructor for a ComputerPlayer object.
 * @return a new ComputerPlayer object.
 */
ComputerPlayer *new_ComputerPlayer (void);

#endif
