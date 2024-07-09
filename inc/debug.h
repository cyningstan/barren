/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * By Cyningstan
 * Copyright (C) Damian Gareth Walker 2021
 *
 * Debug Logging Header
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

/**
 * Open the debug log file.
 */
void debug_open (void);

/**
 * Log a single line to the log file.
 * @param string The string to log.
 */
void debug_log (char *message);

/**
 * Close the debug log file.
 */
void debug_close (void);

#endif
