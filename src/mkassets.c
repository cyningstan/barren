/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Asset generation program.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "cgalib.h"
#include "speaker.h"
#include "fatal.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var output is the output file handle */
static FILE *output;

/** @var scr is the screen */
static Screen *scr;

/*----------------------------------------------------------------------
 * Level 1 Functions.
 */

/**
 * Open a new DAT file.
 */
static void open_dat_file (void)
{
    if (! (output = fopen("barren/barren.dat", "wb")))
	fatalerror (FATAL_NODATA);
    fwrite ("BAR100D", 8, 1, output);
}

/**
 * Initialise the screen.
 */
static void initialise_screen (void)
{
    if (! (scr = scr_create (5)))
	fatalerror (FATAL_DISPLAY);
    scr_palette (scr, 5, 8);
}

/**
 * Load the assets from the PIC file.
 */
static void load_assets (int filenum)
{
    /* local variables */
    FILE *input; /* input file handle */
    char header[7], /* header from input file */
	*pic, /* pointer to picture buffer */
	filename[80]; /* name of input file */

    /* attempt to open the input file and read the header */
    sprintf (filename, "assets/barren%d.pic", filenum);
    if (! (input = fopen (filename, "rb")))
	fatalerror (FATAL_NODATA);
    if (! fread (header, 7, 1, input))
	fatalerror (FATAL_NODATA);

    /* attempt to reserve memory for the picture */
    if (! (pic = malloc (16192)))
	fatalerror (FATAL_NODATA);
    if (! (fread (pic, 16192, 1, input)))
	fatalerror (FATAL_NODATA);

    /* put the picture on the screen and free the memory */
    _fmemcpy ((void far *) 0xb8000000, pic, 16192);
    free (pic);
}

/**
 * Copy the logo bitmap.
 */
static void copy_logo_bitmap (void)
{
    Bitmap *bit; /* temporary reusable bitmap variable */

    /* copy Cyningstan logo */
    bit = bit_create (128, 16);
    scr_get (scr, bit, 8, 8);
    bit_write (bit, output);
    bit_destroy (bit);
}

/**
 * Copy the title bitmaps to the output file.
 */
static void copy_title_bitmap (void)
{
    /* local variables */
    Bitmap *bit; /* temporary reusable bitmap variable */

    /* copy title screen picture */
    bit = bit_create (320, 200);
    scr_get (scr, bit, 0, 0);
    bit_write (bit, output);
    bit_destroy (bit);
}

/**
 * Copy the game bitmaps to the output file.
 */
static void copy_game_bitmaps (void)
{
    /* local variables */
    Bitmap *bit; /* temporary reusable bitmap variable */

    /* copy unit panel */
    bit = bit_create (128, 48);
    scr_get (scr, bit, 184, 8);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy terrain panel */
    bit = bit_create (128, 48);
    scr_get (scr, bit, 184, 104);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy build panel */
    bit = bit_create (128, 48);
    scr_get (scr, bit, 184, 56);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the neutral victory position */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 72, 24);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the player #1 victory position */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 88, 24);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the player #2 victory position */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 104, 24);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the victory position mask */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 120, 24);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the map cursor */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 8);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the map cursor mask */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 24);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the flash */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 40);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the flash mask */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 56);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the blast */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 72);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the blast mask */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 88);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the spanner */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 104);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the spanner mask */
    bit = bit_create (16, 16);
    scr_get (scr, bit, 136, 120);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the menu blank scroll arrow */
    bit = bit_create (8, 8);
    scr_get (scr, bit, 128, 40);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the menu up scroll arrow */
    bit = bit_create (8, 8);
    scr_get (scr, bit, 128, 48);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the menu up down arrow */
    bit = bit_create (8, 8);
    scr_get (scr, bit, 128, 56);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the building blank scroll arrow */
    bit = bit_create (16, 8);
    scr_get (scr, bit, 120, 64);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the building left scroll arrow */
    bit = bit_create (16, 8);
    scr_get (scr, bit, 120, 72);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the building right scroll arrow */
    bit = bit_create (16, 8);
    scr_get (scr, bit, 120, 80);
    bit_write (bit, output);
    bit_destroy (bit);

    /* copy the game screen background */
    scr_ink (scr, 0);
    scr_box (scr, 8, 8, 144, 144);
    scr_box (scr, 184, 8, 128, 144);
    bit = bit_create (320, 200);
    scr_get (scr, bit, 0, 0);
    bit_write (bit, output);
    bit_destroy (bit);
}

/**
 * Copy the Future font into the output file.
 */
static void copy_font (void)
{
    /* local variables */
    FILE *input; /* input file handle */
    char header[8]; /* header read from input file */
    Font *fnt; /* font read from input file */

    /* open input file, read and verify header */
    if (! (input = fopen ("cgalib/cgalib/fnt/future.fnt", "rb")))
	fatalerror (FATAL_NODATA);
    if (! (fread (header, 8, 1, input)))
	fatalerror (FATAL_NODATA);
    if (strcmp (header, "CGA100F"))
	fatalerror (FATAL_NODATA);

    /* read the font and copy it to the output file */
    if (! (fnt = fnt_read (input)))
	fatalerror (FATAL_NODATA);
    fclose (input);
    fnt_write (fnt, output);
    fnt_destroy (fnt);
}

/**
 * Close the screen.
 */
static void close_screen (void)
{
    scr_destroy (scr);
}

/**
 * Add music and sound effects to the asset files.
 */
static void add_sounds (void)
{
    Effect *effect; /* a sound effect object */
    Tune *tune; /* a tune object */
    FILE *input; /* input file handle for the tune */
    char header[8]; /* header of the tune input file */

    /* create the tune and sound effect objects */
    if (! (tune = new_Tune ()))
	fatalerror (FATAL_MEMORY);
    if (! (effect = new_Effect ()))
	fatalerror (FATAL_MEMORY);

    /* load the tune and save it to the asset file */
    if (! (input = fopen ("assets/music.tun", "rb")))
	fatalerror (FATAL_NODATA);
    if (! (fread (header, 8, 1, input)))
	fatalerror (FATAL_NODATA);
    if (strcmp (header, "SPK100T"))
	fatalerror (FATAL_NODATA);
    if (! tune->read (tune, input))
	fatalerror (FATAL_NODATA);
    fclose (input);
    tune->write (tune, output);

    /* generate and save the pew-pew noise */
    effect->pattern = EFFECT_FALL;
    effect->repetitions = 2;
    effect->low = 0;
    effect->high = 60;
    effect->duration = 3;
    effect->pause = 0;
    effect->write (effect, output);

    /* generate and save the blast noise */
    effect->pattern = EFFECT_NOISE;
    effect->repetitions = 1;
    effect->low = 12;
    effect->high = 36;
    effect->duration = 3;
    effect->pause = 0;
    effect->write (effect, output);

    /* generate and save the hammer noise */
    effect->pattern = EFFECT_RISE;
    effect->repetitions = 3;
    effect->low = 12;
    effect->high = 24;
    effect->duration = 2;
    effect->pause = 1;
    effect->write (effect, output);

    /* generate and save the comms noise */
    effect->pattern = EFFECT_RISE;
    effect->repetitions = 4;
    effect->low = 60;
    effect->high = 60;
    effect->duration = 1;
    effect->pause = 1;
    effect->write (effect, output);

    /* clean up */
    tune->destroy (tune);
    effect->destroy (effect);
}

/**
 * Close the DAT file.
 */
static void close_dat_file (void)
{
    fclose (output);
}

/*----------------------------------------------------------------------
 * Top Level Function.
 */

/**
 * Main Program.
 * @return 0 on success, >0 on failure.
 */
int main (void)
{
    open_dat_file ();
    initialise_screen ();
    load_assets (0);
    copy_logo_bitmap ();
    load_assets (1);
    copy_title_bitmap ();
    load_assets (0);
    copy_game_bitmaps ();
    copy_font ();
    close_screen ();
    add_sounds ();
    close_dat_file ();
    printf ("BARREN\\BARREN.DAT generated successfully.\n");
    return 0;
}
