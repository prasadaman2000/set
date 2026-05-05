
#ifndef DECK_H_
#define DECK_H_

#include <memory>
#include <optional>
#include <vector>

namespace set {

enum Shape { SQUARE, CIRCLE, TRIANGLE };

enum Color { RED, GREEN, BLUE };

enum Fill { EMPTY, SOLID, STRIPE };

struct Card {
  Shape shape;
  Color color;
  Fill fill;
  int count;

  bool operator==(const Card& a) const {
    return a.shape == shape && a.color == color && a.fill == fill &&
           a.count == count;
  }

  bool operator!=(const Card& a) const { return !(*this == a); }

  bool operator<(const Card& a) const {
    return a.shape < shape || a.color < color || a.fill < fill ||
           a.count < count;
  }
};

inline bool IsASet(Card c1, Card c2, Card c3) {
  if (!(c1.color == c2.color && c2.color == c3.color) &&
      !(c1.color != c2.color && c2.color != c3.color && c1.color != c3.color)) {
    return false;
  }

  if (!(c1.shape == c2.shape && c2.shape == c3.shape) &&
      !(c1.shape != c2.shape && c2.shape != c3.shape && c1.shape != c3.shape)) {
    return false;
  }

  if (!(c1.count == c2.count && c2.count == c3.count) &&
      !(c1.count != c2.count && c2.count != c3.count && c1.count != c3.count)) {
    return false;
  }

  if (!(c1.fill == c2.fill && c2.fill == c3.fill) &&
      !(c1.fill != c2.fill && c2.fill != c3.fill && c1.fill != c3.fill)) {
    return false;
  }

  return true;
}

class Deck {
 public:
  Deck() = default;
  static std::unique_ptr<Deck> Create();
  void Shuffle();
  void Reset();
  std::optional<Card> NextCard();
  bool HasMoreCards();

 private:
  std::vector<Card> deck_;
  int curr_card_idx_;
};

}  // namespace set

#endif  // DECK_H_