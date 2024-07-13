/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Tune Handler Header.
 */

#ifndef __TUNE_H__
#define __TUNE_H__

/* required headers */
#include "speaker.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* forward declarations of types. */
typedef struct tune Tune;
typedef struct note Note;
typedef struct playback Playback;
typedef struct keyhandler KeyHandler;

/** @struct tune is a piece of music. */
struct tune {

    /** @var notes is a linked list of notes. */
    Note *notes;

    /** @var note is the next note to play. */
    Note *note;

    /**
     * Destroy a tune when no longer needed.
     * @param tune is the tune to destroy.
     */
    void (*destroy) (Tune *tune);

    /**
     * Add a note to the tune.
     * @param tune is the tune to modify.
     * @param note is the note to add.
     * @return 1 if successful, 0 if not.
     */
    void (*add) (Tune *tune, Note *note);

    /**
     * Read the tune in from an already open file.
     * @param tune is the tune to read.
     * @param input is the input file.
     * @return 1 if successful, 0 if failed.
     */
    int (*read) (Tune *tune, FILE *input);

    /**
     * Write the present tune to an already open file.
     * @param tune is the tune to write.
     * @param output is the output file.
     * @return 1 if successful, 0 if failed.
     */
    int (*write) (Tune *tune, FILE *output);

    /**
     * Play the present tune till a key is pressed.
     * If no keyhandler is supplied, the whole tune will play.
     * Playback will stop if a key is pressed or the tune ends.
     * Playback will resume after interruption on the next call.
     * @param tune is the tune to play.
     * @param keys is a keyboard handler or NULL.
     */
    void (*play) (Tune *tune, KeyHandler *keys);

};

/** @struct note is a note in a piece of music */
struct note {

    /** @var pitch is the pitch, 0 being C0. */
    int pitch;

    /** @var duration is the time of the note in 1/20 seconds. */
    int duration;

    /** @var next is a pointer to the next note. */
    Note *next;

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new tune.
 * @returns the new tune, or NULL if unsuccessful.
 */
Tune *new_Tune (void);

/**
 * Construct a new note.
 * @param pitch is the note's pitch.
 * @param duration is the note's duration.
 */
Note *new_Note (int pitch, int duration);

#endif
