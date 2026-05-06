#include "absl/flags/parse.h"
#include "game.h"
#include "player.h"

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  set::Game g;
  set::CLIPlayer p(&g);
  g.Play(&p);
}