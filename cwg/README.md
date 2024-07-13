The Cyningstan Wargame Engine
=============================

CWG, or the Cyningstan Wargame Engine, is a library that controls the
battle section of a war game. It is designed for building turn-based
strategy games around it, games that involve units moving around a
battlefield and firing at one another till the battle is won.

The engine does not concern itself with graphics or other
visuals. These are left to the host game. Neither does it concern
itself with action at the strategic level (the gathering of resources,
the conquering of regions, diplomacy). These are also left to the host
game.

What it does is provide data structures to store the battlefield, its
terrain and units, provides rules for resolution of movement and
combat.

Features
--------

* A battle map of up to 256 squares, various dimensions.
* Up to 8 unit types in a battle (expandable).
* Up to 8 terrain types on a battlefield (expandable).
* Up to 32 units on a battlefield (expandable).
* Movement, attacks, building and repair on the battlefield.
* Victory checking with optional victory positions.

Building
--------

The system is designed for building on a variety of operating systems,
using either OpenWatcom C or GNU C.

To build the engine in OpenWatcom C:

$ wmake -f makefile.wcc

To build the engine in GNU C:

$ make -f makefile.gcc
