/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Graphical Display Handler Header.
 */

/* Types defined here */
typedef struct display Display;

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/* project module declarations needed */
#include "cwg.h"
#include "game.h"

/**
 * @enum display_control is a control code.
 */
typedef enum display_control {
    DISPLAY_UP,
    DISPLAY_DOWN,
    DISPLAY_LEFT,
    DISPLAY_RIGHT,
    DISPLAY_FIRE
} DisplayControl;

/**
 * @enum display_sound Enumerates the sound effects.
 */
typedef enum display_sound {
    DISPLAY_PEWPEW,
    DISPLAY_EXPLOSION,
    DISPLAY_HAMMER,
    DISPLAY_COMMS
} DisplaySound;

/**
 * @struct display is the display handler.
 */
typedef struct display_data DisplayData;
struct display {

    /** @var data is the private display data */
    DisplayData *data;

    /**
     * Destroy the display when no longer needed.
     */
    void (*destroy) ();

    /**
     * Give display routines access to the game data.
     * @param ingame The game being played.
     */
    void (*setgame) (Game *ingame);

    /**
     * Update the screen from the buffer.
     */
    void (*update) (void);

    /**
     * Display a message in the prompt box.
     * @param message The message to display.
     */
    void (*prompt) (char *message);

    /**
     * Display a menu and get an option from it.
     * @param count   The number of options in the menu.
     * @param options The option names in an array.
     * @param initial The initial option selected.
     * @return        The number of the option selected.
     */
    int (*menu) (int count, char **options, int initial);

    /**
     * Show the title screen.
     * @param display is the display to affect.
     */
    void (*showtitlescreen) ();

    /**
     * Await the fire key at the title screen.
     * @param display is the display to affect.
     */
    void (*titlekey) ();

    /**
     * Make a sound.
     * @param id The ID of the sound.
     */
    void (*playsound) (int id);

    /**
     * Display a screenful of text in the right-hand window.
     * @param text is the text to display.
     */
    void (*fulltext) (char *text);

    /**
     * Scroll one or more lines of text into the right-hand window.
     * @param text  The text to add.
     * @param lines The number of lines to scroll.
     */
    void (*linetext) (char *text, int lines);

    /**
     * Update a single square on the display map.
     * @param campaign The campaign containing the bitmaps.
     * @param battle   The battle to display.
     * @param x        The x coordinate of the map square to update.
     * @param y        The y coordinate of the map square to update.
     */
    void (*updatemap) (Campaign *campaign, Battle *battle, int x,
		       int y);

    /**
     * Prepare a battle map.
     * @param campaign The campaign containing the bitmaps.
     * @param battle   The battle to display.
     */
    void (*preparemap) (Campaign *campaign, Battle *battle);

    /**
     * Show the battle map.
     * @param x The x coordinate shown at the top left.
     * @param y The y coordinate shown at the top left.
     */
    void (*showmap) (int x, int y);

    /**
     * Show the battle map cursor.
     * @param xcursor The x coordinate of the cursor.
     * @param ycursor The y coordinate of the cursor.
     * @param xview The x coordinate of the map view.
     * @param yview The y coordinate of the map view.
     */
    void (*showmapcursor) (int xcursor, int ycursor, int xview,
			   int yview);

    /**
     * Hide the battle map cursor.
     * @param xcursor The x coordinate of the cursor.
     * @param ycursor The y coordinate of the cursor.
     * @param xview The x coordinate of the map view.
     * @param yview The y coordinate of the map view.
     */
    void (*hidemapcursor) (int xcursor, int ycursor, int xview,
			   int yview);

    /**
     * Show information on empty terrain.
     * @param terrainid The ID of the terrain to show.
     * @param unitid    The ID of the last selected friendly unit.
     */
    void (*showterraininfo) (int terrainid, int unitid);

    /**
     * Show information on a friendly unit.
     * @param unit The unit to show.
     */
    void (*showunitinfo) (Unit *unit);

    /**
     * Show information on an enemy unit.
     * @param unit The unit to show.
     */
    void (*showenemyinfo) (Unit *unit);

    /**
     * Show the map square information panel.
     * @param x The x coordinate to show.
     * @param y The y coordinate to show.
     * @param u The ID of the last selected friendly unit.
     */
    void (*showmapinfo) (int x, int y, int u);

    /**
     * Show the new game screen.
     * @param gamename The game name to display.
     * @param campaignname The campaign name to display.
     * @param playertypes  An array of playertypes to display.
     */
    void (*shownewgame) (char *gamename, char *campaignname,
			 char corpnames[2][9], int *playertypes);

    /**
     * Show a single line of the new game screen.
     * @param gamename     The game name to display.
     * @param campaignname The campaign name to display.
     * @param playertypes  An array of playertypes to display.
     * @param line         The line to show.
     * @param highlight    1 if line should be highlighted.
     */
    void (*showgameoption) (char *gamename, char *campaignname,
			    char corpnames[2][9], int *playertypes,
			    int line, int highlight);

    /**
     * Show the briefing screen.
     * @param text The briefing text.
     * @param side The side to brief.
     * @param x    The location shown in the map window - x coordinate.
     * @param y    The location shown in the map window - y coordinate.
     */
    void (*showbriefing) (char *text, int side, int x, int y);

    /**
     * Show the Start Human Turn Screen (for 2-player games).
     * @param side The side who is playing.
     */
    void (*showstartturn) (int side);

    /**
     * Show the Human Turn Screen.
     * @param xcursor The x position of the map cursor.
     * @param ycursor The y position of the map cursor.
     * @param xview   The x position of the map view (top left).
     * @param yview   The y position of the map view (top left).
     */
    void (*showhumanturn) (int xcursor, int ycursor, int xview,
			       int yview);

    /**
     * Show a unit firing.
     * @param xunit The x location of the unit.
     * @param yunit The y location of the unit.
     * @param xview The x location of the top left of the map view.
     * @param yview The y location of the top left of the map view.
     */
    void (*showunitfire) (int xunit, int yunit, int xview, int yview);

    /**
     * Show a unit exploding.
     * @param xunit The x location of the unit.
     * @param yunit The y location of the unit.
     * @param xview The x location of the top left of the map view.
     * @param yview The y location of the top left of the map view.
     */
    void (*showunitblast) (int xunit, int yunit, int xview, int yview);

    /**
     * Show a unit being built or repaired.
     * @param xunit the X location of the unit.
     * @param yunit The y location of the unit.
     * @param xview The x location of the top left of the map view.
     * @param yview The y location of the top left of the map view.
     */
    void (*showspanner) (int xunit, int yunit, int xview, int yview);

    /**
     * Show the unit types interface.
     * @param bcount The number of types that can be built.
     * @param btypes An array of types that can be built by ID.
     * @param first  The first type displayed.
     */
    void (*showunittypes) (int bcount, int *btypes, int first);

    /**
     * Show a cursor on the unit types interface.
     * @param bcount  The number of types that can be built.
     * @param btypes  An array of types that can be built by ID.
     * @param first   The first type displayed.
     * @param current The current type highlighted.
     */
    void (*showunittypecursor) (int bcount, int *btypes, int first,
				int current);

    /**
     * Hide the cursor on the unit types interface.
     * @param bcount  The number of types that can be built.
     * @param btypes  An array of types that can be built by ID.
     * @param first   The first type displayed.
     * @param current The current type highlighted.
     */
    void (*hideunittypecursor) (int bcount, int *btypes, int first,
				int current);

    /**
     * Update the unit types interface on scrolling.
     * @param bcount The number of types that can be built.
     * @param btypes An array of types taht can be built by ID.
     * @param first  The first type displayed.
     */
    void (*updateunittypes) (int bcount, int *btypes, int first);

    /**
     * Show the Turn Report Screen.
     * @param xview   The x position of the map view (top left).
     * @param yview   The y position of the map view (top left).
     */
    void (*showturnreport) (int xview, int yview);

    /**
     * Show the current resources.
     * @param resources The resource figure.
     */
    void (*showresources) (int resources);

    /**
     * Show the debriefing screen.
     * @param text The briefing text.
     * @param side The side to brief.
     * @param x    The location shown in the map window - x coordinate.
     * @param y    The location shown in the map window - y coordinate.
     */
    void (*showdebriefing) (char *text, int side, int x, int y);

    /**
     * Show the Computer Turn screen.
     * @param side The side to brief.
     * @param x    The location shown in the map window - x coordinate.
     * @param y    The location shown in the map window - y coordinate.
     */
    void (*showcomputerturn) (int side, int x, int y);

    /**
     * Show the PBM Turn screen.
     * @param side The side to brief.
     * @param x    The location shown in the map window - x coordinate.
     * @param y    The location shown in the map window - y coordinate.
     */
    void (*showpbmturn) (int side, int x, int y);
    
    /**
     * Show the end game screen.
     * @param victor The winner of the game.
     * @param x      The location shown in the map window - x coordinate
     * @param y      The location shown in the map window - y coordinate
     */
    void (*showendgame) (int victor, int x, int y);

};

/*----------------------------------------------------------------------
 * Constructor Declarations.
 */

/**
 * Display constructor.
 * @param colourset = 0 for mono, 1 for colour, 2 for nice colour.
 * @param quiet = 0 for sound and music, 1 for silence.
 * @return the new display.
 */
Display *new_Display (int colourset, int quiet);

#endif
