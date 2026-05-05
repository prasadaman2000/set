#include "deck.h"

#include <algorithm>
#include <random>

namespace set {

void Deck::Reset() {
  deck_.clear();
  curr_card_idx_ = 0;
  for (int shape = 0; shape < 3; ++shape) {
    for (int color = 0; color < 3; ++color) {
      for (int fill = 0; fill < 3; ++fill) {
        for (int count = 1; count <= 3; ++count) {
          deck_.push_back(Card{
              .shape = Shape(shape),
              .color = Color(color),
              .fill = Fill(fill),
              .count = count,
          });
        }
      }
    }
  }
}

std::unique_ptr<Deck> Deck::Create() {
  auto deck = std::make_unique<Deck>();
  deck->Reset();
  return deck;
}

void Deck::Shuffle() {
  std::random_device rd;
  std::mt19937 g(rd());

  std::shuffle(deck_.begin(), deck_.end(), g);
}

bool Deck::HasMoreCards() { return curr_card_idx_ < deck_.size(); }

std::optional<Card> Deck::NextCard() {
  if (curr_card_idx_ >= deck_.size()) {
    return std::nullopt;
  }

  return deck_[curr_card_idx_++];
}

}  // namespace set