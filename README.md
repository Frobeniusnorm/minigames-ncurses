# minigames-ncurses
Some arcade games implemented in ncurses

Currently included:
 * Snake
 * Tetris

There is no implementation for highscores yet.

### Build ###
You need gcc, make and the ncurses library. 
Clone and build the project:
  * ``git clone https://github.com/Frobeniusnorm/minigames-ncurses.git``
  * ``cd minigames-ncurses; make; minigames``

Theoretically this would work in windows too, if the Makefile was modified.

### Contribute ###
If you have written a small ncurses game yourself, include your project in the project structure and Makefile and file a pull request :)
The game currently only works on linux (and theoretically osx) - cross-platform support for windows would be great too.
