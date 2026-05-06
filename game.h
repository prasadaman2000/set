#ifndef GAME_H_
#define GAME_H_

#include <memory>

#include "absl/time/time.h"
#include "deck.h"

namespace set {

class Player;

enum ActionType { ADD_CARD, HAS_SETS, LIST_SETS, SET };

struct PlayerAction {
  ActionType action;
  std::vector<int> set_idxs;
};

struct Result {
  int score;
  absl::Duration total_game_time;
  std::vector<absl::Duration> move_times;
  std::vector<std::string> moves;
  std::vector<std::string> state_ids;
};

class Game {
 public:
  Game();
  Result Play(Player* p, int num_turns = -1);
  void WriteStateToFile(std::string fname);
  bool ReadBoardStateFromFile(std::string fname);
  std::optional<std::vector<int>> ValidateAndReturnSortedCardIndices(
      std::string idxstr);
  std::optional<std::vector<int>> ValidateAndReturnSortedCardIndices(
      std::vector<int> idxs);
  std::vector<int> IndiciesWithCardsOnTable();
  std::optional<Card> CardAtIndex(int idx);
  bool HasSet();

 private:
  std::vector<std::vector<int>> FindAllSets();

  void UpdateOrRemoveCardFromTable(int idx);
  void RemoveCardFromTable(int idx);
  std::string BoardAsString();
  std::string BoardId();

  std::unique_ptr<Deck> deck_;
  std::vector<std::optional<Card>> cards_on_table_;
};
}  // namespace set

#endif  // GAME_H_