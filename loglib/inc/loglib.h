/*======================================================================
 * LOGLIB
 * A small library for logging debug information in C projects.
 * Copyright (C) Damian Gareth Walker 2022
 * 
 * Main library header.
 */

/* types defined in this header */
typedef struct log Log;

#ifndef __LOGLIB_H__
#define __LOGLIB_H__

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new log.
 * @param file The name of the source file.
 */
void log_open (char *file);

/**
 * Close the log file and destroy the instance.
 */
void log_close (void);

/**
 * Write to the log file.
 * @param format The format of the output.
 * @param ...    The arguments to print with the format.
 */
void log_printf (char *format, ...);

#endif
