#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"
#include "game.h"
#include "player.h"

ABSL_FLAG(std::string, state_number, "", "state number to replay");
ABSL_FLAG(int, num_moves_to_play, 1, "num moves to replay");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  set::Game g;

  g.ReadBoardStateFromFile(
      absl::StrCat(std::string("/Users/aman/CS/set/game_states/"),
                   absl::GetFlag(FLAGS_state_number)));
  set::RememberTheLastTurnPlayer p(&g);
  set::Result r = g.Play(&p, absl::GetFlag(FLAGS_num_moves_to_play));

  for (int i = 0; i < r.move_times.size(); ++i) {
    std::cout << "move " << i + 1 << " move time: " << r.move_times[i]
              << " with move " << r.moves[i] << " at state " << r.state_ids[i]
              << "\n";
  }
}