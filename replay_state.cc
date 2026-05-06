#include "absl/flags/parse.h"
#include "game.h"
#include "player.h"

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  set::Game g;
  g.ReadBoardStateFromFile(
      "/Users/aman/CS/set/game_states/10919971516930967052");
  set::RememberTheLastTurnPlayer p(&g);
  set::Result r = g.Play(&p, 5);

  for (int i = 0; i < r.move_times.size(); ++i) {
    std::cout << "move " << i + 1 << " move time: " << r.move_times[i]
              << " with move " << r.moves[i] << "\n";
  }
}