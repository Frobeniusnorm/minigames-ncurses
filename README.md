# minigames-ncurses
Some arcade games implemented in ncurses

Currently included:
 * Snake
 * Tetris
 * Pacman

There is no implementation for highscores yet.

### Build ###
You need gcc, make and the ncurses library.
Clone and build the project:
  * ``git clone https://github.com/Frobeniusnorm/minigames-ncurses.git``
  * ``cd minigames-ncurses; make``

Theoretically this would work in windows too, if the Makefile was modified.

### Usage ###
Just execute the binary in a terminal.

### Remarks ###
The games aren't no exact implementations of the original games.
E.g. Pacman uses the A* algorithm to determine the turns of the ghosts (since selecting the tile with the shortest distance to its target may not always result in the shortest path).
It's more about implementing the "idea" and feeling of the game.

### Contribute ###
If you have written a small ncurses game yourself, include your project in the project structure and Makefile and file a pull request :)
The game currently only works on linux (and theoretically osx) - cross-platform support for windows would be great too.
