#include "game.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "deck.h"
#include "player.h"

namespace set {
namespace {
constexpr int kNumCardsAtStart = 12;
}  // namespace

Game::Game() {
  deck_ = Deck::Create();
  deck_->Shuffle();

  for (int i = 0; i < kNumCardsAtStart; ++i) {
    cards_on_table_.push_back(*deck_->NextCard());
  }
}

void Game::WriteStateToFile() {
  std::ofstream file("/Users/aman/CS/set/display.txt");

  if (file) {
    for (std::optional<Card> c : cards_on_table_) {
      if (c.has_value()) {
        file << c->color << " " << c->shape << " " << c->fill << " " << c->count
             << "\n";
      } else {
        file << 0 << " " << 0 << " " << 0 << " " << 0 << "\n";
      }
    }
  } else {
    std::cout << "Could not open file\n";
  }

  file.close();
}

std::vector<std::vector<int>> Game::FindAllSets() {
  std::vector<std::vector<int>> sets;
  for (int i = 0; i < cards_on_table_.size() - 2; ++i) {
    if (!cards_on_table_[i].has_value()) {
      continue;
    }
    for (int j = i + 1; j < cards_on_table_.size() - 1; ++j) {
      if (!cards_on_table_[j].has_value()) {
        continue;
      }
      for (int k = j + 1; k < cards_on_table_.size(); ++k) {
        if (!cards_on_table_[k].has_value()) {
          continue;
        }
        if (IsASet(*cards_on_table_[i], *cards_on_table_[j],
                   *cards_on_table_[k])) {
          sets.push_back({i, j, k});
        }
      }
    }
  }
  return sets;
}

std::optional<std::vector<int>> Game::ValidateAndReturnSortedCardIndices(
    std::vector<int> idxs) {
  if (idxs.size() != 3) {
    return std::nullopt;
  }

  for (int idx : idxs) {
    if (idx < 0 || idx >= cards_on_table_.size()) {
      std::cout << "index " << idx << " is invalid.\n";
      return std::nullopt;
    }

    if (!cards_on_table_[idx].has_value()) {
      std::cout << "index " << idx << " is an empty card.\n";
      return std::nullopt;
    }
  }

  std::sort(idxs.begin(), idxs.end());
  return idxs;
}

std::optional<std::vector<int>> Game::ValidateAndReturnSortedCardIndices(
    std::string idxstr) {
  std::stringstream ss(idxstr);
  std::string idx;
  std::vector<int> idxs;

  while (std::getline(ss, idx, ',')) {
    try {
      idxs.push_back(std::stoi(idx));
    } catch (const std::exception& e) {
      std::cerr << "Conversion failed: " << e.what();
      return std::nullopt;
    }
  }

  return ValidateAndReturnSortedCardIndices(idxs);
}

void Game::UpdateOrRemoveCardFromTable(int idx) {
  if (idx >= kNumCardsAtStart) {
    cards_on_table_[idx] = std::nullopt;
    return;
  }

  if (deck_->HasMoreCards()) {
    cards_on_table_[idx] = *deck_->NextCard();
  } else {
    cards_on_table_[idx] = std::nullopt;
  }
}

std::vector<int> Game::IndiciesWithCardsOnTable() {
  std::vector<int> indicies;
  for (int i = 0; i < cards_on_table_.size(); ++i) {
    if (cards_on_table_[i].has_value()) {
      indicies.push_back(i);
    }
  }
  return indicies;
}

std::optional<Card> Game::CardAtIndex(int idx) { return cards_on_table_[idx]; }

bool Game::HasSet() { return !FindAllSets().empty(); }

Result Game::Play(Player* p) {
  // std::cout << "Let's play Set! Be sure to start the visualizer.\n";

  absl::Time start_time = absl::Now();
  Result r;
  absl::Time turn_start_time = absl::Now();
  while (true) {
    WriteStateToFile();
    if (!HasSet() && !deck_->HasMoreCards()) {
      std::cout << "Game is done!\n";
      r.total_game_time = absl::Now() - start_time;
      return r;
    }

    PlayerAction action = p->GetAction();

    switch (action.action) {
      case ADD_CARD: {
        for (int i = 0; i < 3; ++i) {
          std::optional<Card> next_card = deck_->NextCard();
          if (!next_card.has_value()) {
            std::cout << "Deck exhausted!\n";
            break;
          }
          cards_on_table_.push_back(*next_card);
        }
      } break;
      case HAS_SETS: {
        auto all_sets = FindAllSets();
        std::cout << "There ARE " << (all_sets.empty() ? "NO " : "")
                  << "sets in the current view.\n";
      } break;
      case LIST_SETS: {
        for (auto set : FindAllSets()) {
          std::cout << "Set: " << set[0] << " " << set[1] << " " << set[2]
                    << "\n";
        }
      } break;
      case SET: {
        std::vector<int> idxs = action.set_idxs;
        if (IsASet(*cards_on_table_[idxs[0]], *cards_on_table_[idxs[1]],
                   *cards_on_table_[idxs[2]])) {
          idxs = *ValidateAndReturnSortedCardIndices(idxs);
          ++r.score;
          r.move_times.push_back(absl::Now() - turn_start_time);
          turn_start_time = absl::Now();
          UpdateOrRemoveCardFromTable(idxs[2]);
          UpdateOrRemoveCardFromTable(idxs[1]);
          UpdateOrRemoveCardFromTable(idxs[0]);

          while (cards_on_table_.size() > kNumCardsAtStart &&
                 !cards_on_table_.back().has_value()) {
            cards_on_table_.pop_back();
          }
        } else {
          --r.score;
          std::cout << "That was not a valid set!\n";
        }
      } break;
    }
  }
}

};  // namespace set
