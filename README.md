# Barren Planet
Barren Planet is a turn-based strategy game for MS-DOS. It runs on an 8088 machine with CGA graphics. If you just want to play it, you can find the pre-built binary package at http://dos.cyningstan.org.uk/downloads/7/barren-planet.
# Building Requirements
To build the project, you will need the following:
* A DOS environment (MS-DOS, DOSBox).
* OpenWatcom 2.0 C compiler and tools.
# To Build the Project
Enter your DOS environment and CD to the repository's main directory. Assuming your OpenWatcom environment is correctly set up, a single command will build all the libraries and the game itself:
```
wmake
```
Once done, the ``barren`` directory will contain the built game, ready to be packaged or copied to whatever machine and directory you want to run it from.
