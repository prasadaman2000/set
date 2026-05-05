#include "player.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <tuple>

#include "absl/flags/flag.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "deck.h"
#include "game.h"
#include "ttl_pointer.h"

ABSL_FLAG(bool, use_real_delays, false, "Whether or not to use real delays.");

namespace set {
PlayerAction CLIPlayer::GetAction() {
  std::cout
      << "Enter found set as comma separated string, 'a' to add up to 3 new "
         "cards to the table, 'v' to validate if there are valid sets "
         "on the table, 'l' to list out the sets on the table.\n";
  std::string idxstr;
  std::cin >> idxstr;
  if (idxstr[0] == 'a') {
    return PlayerAction{.action = ADD_CARD};
  } else if (idxstr[0] == 'v') {
    return PlayerAction{.action = HAS_SETS};
  } else if (idxstr[0] == 'l') {
    return PlayerAction{.action = LIST_SETS};
  } else {
    std::optional<std::vector<int>> idxs;
    idxs = game_->ValidateAndReturnSortedCardIndices(idxstr);
    if (!idxs.has_value()) {
      std::cout << idxstr << " is invalid. try again.\n";
      return GetAction();
    }
    return PlayerAction{.action = SET, .set_idxs = *idxs};
  }
}

Card ScanPlayer::LookupWithDelay(int idx, uint64_t delay_ms) {
  if (!absl::GetFlag(FLAGS_use_real_delays)) {
    delay_ms = 0;
  }
  absl::SleepFor(absl::Milliseconds(delay_ms));
  return *(game_->CardAtIndex(idx));
}

PlayerAction RandomScanPlayer::GetAction() {
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();

  std::vector<int> idxs;
  std::set<std::tuple<int, int, int>> seen_idxs;
  std::set<int> seen_cards;
  std::array<Card, 3> cards;
  int count = 0;
  while (true) {
    ++count;
    if (count % 10 == 0 && !game_->HasSet()) {
      return PlayerAction{.action = ADD_CARD};
    }
    do {
      idxs.clear();
      std::sample(valid_indicies.begin(), valid_indicies.end(),
                  std::back_inserter(idxs), 3, gen_);
    } while (seen_idxs.count({idxs[0], idxs[1], idxs[2]}));
    seen_idxs.insert({idxs[0], idxs[1], idxs[2]});
    for (int i = 0; i < idxs.size(); ++i) {
      if (seen_cards.count(idxs[i])) {
        cards[i] = LookupWithDelay(idxs[i], 5);
      } else {
        cards[i] = LookupWithDelay(idxs[i], 8 + i);
        seen_cards.insert(idxs[i]);
      }
    }
    if (IsASet(cards[0], cards[1], cards[2])) {
      return PlayerAction{.action = SET, .set_idxs = idxs};
    }
  }
}

PlayerAction OrderedScanPlayer::GetAction() {
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();
  for (int i = 0; i < valid_indicies.size() - 2; ++i) {
    Card card_i = LookupWithDelay(valid_indicies[i], 8);
    for (int j = i + 1; j < valid_indicies.size() - 1; ++j) {
      Card card_j = LookupWithDelay(valid_indicies[j], 9);
      for (int k = j + 1; k < valid_indicies.size(); ++k) {
        Card card_k = LookupWithDelay(valid_indicies[k], 10);
        if (IsASet(card_i, card_j, card_k)) {
          auto idxs = game_->ValidateAndReturnSortedCardIndices(std::vector{
              valid_indicies[i], valid_indicies[j], valid_indicies[k]});
          if (!idxs.has_value()) {
            std::cout << "idxs no value\n";
          }
          return PlayerAction{.action = SET, .set_idxs = *idxs};
        }
      }
    }
  }
  return PlayerAction{.action = ADD_CARD};
}

ttl_pointer<Card> ForgetfulScanPlayer::DelayedForgettableCard(
    int idx, int64_t delay_ms, int64_t timeout_ms) {
  if (!absl::GetFlag(FLAGS_use_real_delays)) {
    delay_ms = 0;
  }
  absl::SleepFor(absl::Milliseconds(delay_ms));
  return ttl_pointer<Card>::Create(*(game_->CardAtIndex(idx)), timeout_ms);
}

PlayerAction ForgetfulScanPlayer::GetAction() {
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();
  for (int i = 0; i < valid_indicies.size() - 2; ++i) {
    ttl_pointer<Card> card_i = DelayedForgettableCard(valid_indicies[i], 8, 40);
    for (int j = i + 1; j < valid_indicies.size() - 1; ++j) {
      ttl_pointer<Card> card_j =
          DelayedForgettableCard(valid_indicies[j], 9, 50);
      for (int k = j + 1; k < valid_indicies.size(); ++k) {
        Card card_k = LookupWithDelay(valid_indicies[k], 10);
        if (!card_i.has_value()) {
          absl::SleepFor(absl::Milliseconds(5));
          card_i.refresh();
        }
        if (!card_j.has_value()) {
          absl::SleepFor(absl::Milliseconds(5));
          card_j.refresh();
        }
        if (IsASet(*card_i, *card_j, card_k)) {
          auto idxs = game_->ValidateAndReturnSortedCardIndices(std::vector{
              valid_indicies[i], valid_indicies[j], valid_indicies[k]});
          if (!idxs.has_value()) {
            std::cout << "idxs no value\n";
          }
          return PlayerAction{.action = SET, .set_idxs = *idxs};
        }
      }
    }
  }
  return PlayerAction{.action = ADD_CARD};
}

Card MatchPlayer::CardToMakeASet(Card c1, Card c2) {
  Card match;

  std::set<Color> colors = {RED, GREEN, BLUE};
  if (c1.color == c2.color) {
    match.color = c1.color;
  } else {
    colors.erase(c1.color);
    colors.erase(c2.color);
    match.color = *colors.begin();
  }

  std::set<int> count = {1, 2, 3};
  if (c1.count == c2.count) {
    match.count = c1.count;
  } else {
    count.erase(c1.count);
    count.erase(c2.count);
    match.count = *count.begin();
  }

  std::set<Shape> shape = {CIRCLE, TRIANGLE, SQUARE};
  if (c1.shape == c2.shape) {
    match.shape = c1.shape;
  } else {
    shape.erase(c1.shape);
    shape.erase(c2.shape);
    match.shape = *shape.begin();
  }

  std::set<Fill> fill = {EMPTY, SOLID, STRIPE};
  if (c1.fill == c2.fill) {
    match.fill = c1.fill;
  } else {
    fill.erase(c1.fill);
    fill.erase(c2.fill);
    match.fill = *fill.begin();
  }

  return match;
}

PlayerAction MatchPlayer::GetAction() {
  std::set<std::pair<int, int>> indices_to_consider;
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();
  std::map<Card, int> card_to_index;
  for (int i = 0; i < valid_indicies.size() - 1; ++i) {
    for (int j = i + 1; j < valid_indicies.size(); ++j) {
      indices_to_consider.insert({valid_indicies[i], valid_indicies[j]});
    }
  }

  for (const std::pair<int, int>& indices : indices_to_consider) {
    int first_index = indices.first;
    int second_index = indices.second;

    Card c1 = LookupWithDelay(first_index, 10);
    Card c2 = LookupWithDelay(second_index, 10);

    card_to_index[c1] = first_index;
    card_to_index[c2] = second_index;

    Card card_to_find = CardToMakeASet(c1, c2);

    if (auto found_card = card_to_index.find(card_to_find);
        found_card != card_to_index.end()) {
      return {.action = SET,
              .set_idxs = {first_index, second_index, found_card->second}};
    }

    for (int i = 0; i < valid_indicies.size(); ++i) {
      if (valid_indicies[i] == first_index ||
          valid_indicies[i] == second_index) {
        continue;
      }

      Card c3 = LookupWithDelay(valid_indicies[i], 10);

      if (IsASet(c1, c2, c3)) {
        return {.action = SET,
                .set_idxs = {first_index, second_index, valid_indicies[i]}};
      }
    }
  }

  return {.action = ADD_CARD};
}

PlayerAction RememberTheLastTurnPlayer::GetAction() {
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();
  for (int idx : last_set_) {
    if (game_->CardAtIndex(idx).has_value()) {
      Card replaced_card = LookupWithDelay(idx, 5);
      if (cards_to_look_for_.count(replaced_card)) {
        PlayerAction to_ret = {
            .action = SET,
            .set_idxs = {idx, cards_to_look_for_[replaced_card].first,
                         cards_to_look_for_[replaced_card].second}};
        cards_to_look_for_.erase(replaced_card);
      }
    }
  }

  std::set<std::pair<int, int>> pairs_to_consider;
  std::map<Card, int> card_to_index;
  for (int i = 0; i < valid_indicies.size() - 1; ++i) {
    for (int j = i + 1; j < valid_indicies.size(); ++j) {
      pairs_to_consider.insert({valid_indicies[i], valid_indicies[j]});
    }
  }

  for (const std::pair<int, int>& indices : pairs_to_consider) {
    int first_index = indices.first;
    int second_index = indices.second;

    Card c1 = LookupWithDelay(first_index, 10);
    Card c2 = LookupWithDelay(second_index, 10);

    card_to_index[c1] = first_index;
    card_to_index[c2] = second_index;

    Card card_to_find = CardToMakeASet(c1, c2);

    if (auto found_card = card_to_index.find(card_to_find);
        found_card != card_to_index.end()) {
      cards_to_look_for_.erase(c1);
      cards_to_look_for_.erase(c2);
      cards_to_look_for_.erase(card_to_find);
      return {.action = SET,
              .set_idxs = {first_index, second_index, found_card->second}};
    }

    for (int i = 0; i < valid_indicies.size(); ++i) {
      if (valid_indicies[i] == first_index ||
          valid_indicies[i] == second_index) {
        continue;
      }

      Card c3 = LookupWithDelay(valid_indicies[i], 10);

      if (IsASet(c1, c2, c3)) {
        cards_to_look_for_.erase(c1);
        cards_to_look_for_.erase(c2);
        cards_to_look_for_.erase(c3);
        last_set_ = {first_index, second_index, valid_indicies[i]};
        return {.action = SET, .set_idxs = last_set_};
      }
    }
    cards_to_look_for_[card_to_find] = {first_index, second_index};
  }

  return {.action = ADD_CARD};
}

void RememberTheLastTurnPlayerForgetful::RememberSomeToLookFor(
    size_t n, std::vector<std::pair<Card, std::pair<int, int>>> all_trips) {
  std::set<int> chosen_idxs;
  n = std::min(n, all_trips.size());
  while (chosen_idxs.size() < n) {
    int idx = (size_t)rand() % all_trips.size();
    if (chosen_idxs.count(idx)) continue;

    cards_to_look_for_.insert(all_trips[idx]);
    chosen_idxs.insert(idx);
  }
}

PlayerAction RememberTheLastTurnPlayerForgetful::GetAction() {
  std::vector<int> valid_indicies = game_->IndiciesWithCardsOnTable();
  for (int idx : last_set_) {
    if (game_->CardAtIndex(idx).has_value()) {
      Card replaced_card = LookupWithDelay(idx, 5);
      if (cards_to_look_for_.count(replaced_card)) {
        PlayerAction to_ret = {
            .action = SET,
            .set_idxs = {idx, cards_to_look_for_[replaced_card].first,
                         cards_to_look_for_[replaced_card].second}};
        cards_to_look_for_.erase(replaced_card);
      }
    }
  }

  std::set<std::pair<int, int>> pairs_to_consider;
  std::map<Card, int> card_to_index;
  for (int i = 0; i < valid_indicies.size() - 1; ++i) {
    for (int j = i + 1; j < valid_indicies.size(); ++j) {
      pairs_to_consider.insert({valid_indicies[i], valid_indicies[j]});
    }
  }

  std::vector<std::pair<Card, std::pair<int, int>>> cards_to_look_for;
  for (const std::pair<int, int>& indices : pairs_to_consider) {
    int first_index = indices.first;
    int second_index = indices.second;

    Card c1 = LookupWithDelay(first_index, 10);
    Card c2 = LookupWithDelay(second_index, 10);

    card_to_index[c1] = first_index;
    card_to_index[c2] = second_index;

    Card card_to_find = CardToMakeASet(c1, c2);

    if (auto found_card = card_to_index.find(card_to_find);
        found_card != card_to_index.end()) {
      cards_to_look_for_.erase(c1);
      cards_to_look_for_.erase(c2);
      cards_to_look_for_.erase(card_to_find);
      RememberSomeToLookFor(3, cards_to_look_for);
      return {.action = SET,
              .set_idxs = {first_index, second_index, found_card->second}};
    }

    for (int i = 0; i < valid_indicies.size(); ++i) {
      if (valid_indicies[i] == first_index ||
          valid_indicies[i] == second_index) {
        continue;
      }

      Card c3 = LookupWithDelay(valid_indicies[i], 10);

      if (IsASet(c1, c2, c3)) {
        cards_to_look_for_.erase(c1);
        cards_to_look_for_.erase(c2);
        cards_to_look_for_.erase(c3);
        last_set_ = {first_index, second_index, valid_indicies[i]};
        RememberSomeToLookFor(3, cards_to_look_for);
        return {.action = SET, .set_idxs = last_set_};
      }
    }
    cards_to_look_for.push_back({card_to_find, {first_index, second_index}});
  }

  return {.action = ADD_CARD};
}
}  // namespace set