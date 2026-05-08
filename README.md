This is an implementation of Set.

You will need Bazel installed to use this.

You will also need to modify all of the hardcoded directories and filenames. There hopefully aren't too many.

The various executables/scripts are:

* draw.py -> run this before playing the game if you want to visualize what's happening
* cli_main -> this runs a verison of the game you can play via the CLI
* replay_state -> given a game state id, you can play using any player starting at that state. Useful for debugging players.
* main.cc -> Used to run experiments and record metrics