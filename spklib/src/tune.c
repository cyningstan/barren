/*======================================================================
 * SPKLIB
 * A PC Speaker Sound Library
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Tune Handler Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* compiler-specific headers */
#include <dos.h>
#include <sys/timeb.h>

/* project headers */
#include "tune.h"
#ifdef KEYLIB
#include "keylib.h"
#endif

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* OpenWatcom has a nonsense value for CLOCKS_PER_SEC. */
#define REAL_CLOCKS_PER_SEC 18.2

/** @var speaker A pointer to the Speaker object. */
static Speaker *speaker;

/** @var keycheck is the key check function. */
static int (*keycheck) (void);

/*----------------------------------------------------------------------
 * Private Level 1 Functions.
 */

/**
 * Clear a tune's notes.
 * @param tune is the tune to clear.
 */
static void clearnotes (Tune *tune)
{
    Note *note, /* current note to destroy */
	*next; /* next note to destroy */
    note = tune->notes;
    while (note) {
	next = note->next;
	free (note);
	note = next;
    }
}

/**
 * Dummy keycheck routine.
 * Returns 0.
 */
static int defaultkeycheck (void)
{
    return 0;
}

/*----------------------------------------------------------------------
 * Public Method Functions.
 */

/**
 * Destroy a tune when no longer needed.
 * @param tune is the tune to destroy.
 */
static void destroy (Tune *tune)
{
    if (tune) {
	if (tune->notes)
	    clearnotes (tune);
	free (tune);
    }
}

/**
 * Add a note to the tune.
 * @param tune is the tune to modify.
 * @param note is the note to add.
 * @return 1 if successful, 0 if not.
 */
static void add (Tune *tune, Note *note)
{
    Note *curr; /* current note scanning the tune */

    /* add the first note to the tune */
    if (! tune->notes)
	tune->notes = note;

    /* add subsequent notes to the tune */
    else {
	for (curr = tune->notes; curr->next; curr = curr->next);
	curr->next = note;
    }
}

/**
 * Read the tune in from an already open file.
 * @param tune is the tune to read.
 * @param input is the input file.
 * @return 1 if successful, 0 if failed.
 */
static int read (Tune *tune, FILE *input)
{
    int pitch, /* pitch read from file */
	duration; /* duration read from file */

    /* clear any old tune */
    if (tune->notes)
	clearnotes (tune);

    /* load the new tune */
    if (! speaker->readint (&pitch, input))
	return 0;
    while (pitch != 0xff) {
	if (! speaker->readint (&duration, input))
	    return 0;
	tune->add (tune, new_Note (pitch, duration));
	if (! speaker->readint (&pitch, input))
	    return 0;
    }

    /* return success */
    return 1;
}

/**
 * Write the present tune to an already open file.
 * @param tune is the tune to write.
 * @param output is the output file.
 * @return 1 if successful, 0 if failed.
 */
static int write (Tune *tune, FILE *output)
{
    Note *note; /* note to write */
    int eofnote = 0xff; /* value to signify end of tune */
    for (note = tune->notes; note; note = note->next) {
	speaker->writeint (&note->pitch, output);
	speaker->writeint (&note->duration, output);
    }
    speaker->writeint (&eofnote, output);
    return 1;
}

/**
 * Play the present tune till a key is pressed.
 * If no keyhandler is supplied, the whole tune will play.
 * Playback will stop if a key is pressed or the tune ends.
 * Playback will resume after interruption on the next call.
 * @param tune is the tune to play.
 * @param keys is a keyboard handler or NULL.
 */
static void play (Tune *tune, KeyHandler *keys)
{
    int ticksdelay; /* number of ticks to delay */

    /* set up the keyboard checker */
    #ifdef KEYLIB
    keycheck = keys ? keys->anykey : defaultkeycheck;
    #else
    keycheck = defaultkeycheck;
    #endif

    // Restart the tune if it is stopped
    if (tune->notes && ! tune->note)
	tune->note = tune->notes;
 
    /* main loop */
    while (tune->note && ! keycheck ()) {

	/* if this is a timed note, set a new timer */
	if (tune->note->duration)
	    ticksdelay = tune->note->duration;

	/* play the note */
	sound (speaker->frequencies[tune->note->pitch]);
	delay (1000 / REAL_CLOCKS_PER_SEC);
	--ticksdelay;
	nosound ();
	tune->note = tune->note->next;

	/* wait for the timer to run down */
	if (ticksdelay > 0 && tune->note && tune->note->duration)
	    delay (ticksdelay * (1000 / REAL_CLOCKS_PER_SEC));
    }
}

/*----------------------------------------------------------------------
 * Top Level Functions.
 */

/**
 * Construct a new tune.
 * @returns the new tune, or NULL if unsuccessful.
 */
Tune *new_Tune (void)
{
    Tune *tune; /* the tune to return */

    /* attempt to reserve memory */
    if (! (tune = malloc (sizeof (Tune))))
	return NULL;

    /* get the speaker object */
    speaker = get_Speaker ();

    /* initialise methods */
    tune->destroy = destroy;
    tune->add = add;
    tune->read = read;
    tune->write = write;
    tune->play = play;

    /* initialise attributes */
    tune->notes = NULL;
    tune->note = NULL;

    /* return the new tune */
    return tune;
}

/**
 * Construct a new note.
 * @param pitch is the note's pitch.
 * @param duration is the note's duration.
 */
Note *new_Note (int pitch, int duration)
{
    Note *note; /* the new note to return */

    /* reserve memory for note */
    if (! (note = malloc (sizeof (Note))))
	return NULL;

    /* initialise note values */
    note->pitch = pitch;
    note->duration = duration;
    note->next = NULL;

    /* return the note */
    return note;
}
