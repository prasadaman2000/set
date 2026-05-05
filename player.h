#ifndef PLAYER_H_
#define PLAYER_H_

#include <cstdint>
#include <map>
#include <random>

#include "deck.h"
#include "game.h"
#include "ttl_pointer.h"

namespace set {

class Player {
 public:
  Player(Game* game) : game_(game) {};
  virtual PlayerAction GetAction() { return PlayerAction{}; };
  virtual ~Player() {};

 protected:
  Game* game_;
};

// Allows a human to play the game via CLI.
class CLIPlayer : public Player {
 public:
  CLIPlayer(Game* g) : Player(g) {};
  PlayerAction GetAction() override;
};

class ScanPlayer : public Player {
 protected:
  ScanPlayer(Game* g) : Player(g) {}
  Card LookupWithDelay(int idx, uint64_t ms);
};

/*
  On each turn, look up random triples (i,j,k) of cards. Take 8 ms to read i, 9
  to read j, 10 to read k, each card from the triple. If you've seen a card at
  that index at that turn, take 5 ms to read that card, to simulate the "oh i
  remember that card" effect.
*/
class RandomScanPlayer : public ScanPlayer {
 public:
  RandomScanPlayer(Game* g) : ScanPlayer(g) {};
  PlayerAction GetAction() override;

 private:
  std::mt19937 gen_{std::random_device{}()};
};

/*
  loop through an ordered set of (i,j,k) triples. Take 8ms to read i, 9ms to
  read j, and 10ms to read k in order to simulate slowing down as more
  information is being processed.
*/
class OrderedScanPlayer : public ScanPlayer {
 public:
  OrderedScanPlayer(Game* g) : ScanPlayer(g) {};
  PlayerAction GetAction() override;
};

/*
  simlar to OrderedScanPlayer, except more realistic. Cards (i,j,k) are read
  with the same delays as above, but have a TTL associated with them. Once the
  TTL expires, incur a 5 ms penalty to refresh the card
*/
class ForgetfulScanPlayer : public ScanPlayer {
 public:
  ForgetfulScanPlayer(Game* g) : ScanPlayer(g) {};
  PlayerAction GetAction() override;

 private:
  ttl_pointer<Card> DelayedForgettableCard(int idx, int64_t delay_ms,
                                           int64_t timeout_ms);
};

/*
  for each pair of cards (i,j), pre-compute the card that you need. Find that
  card on the table. pretty simple tbh
*/
class MatchPlayer : public ScanPlayer {
 public:
  MatchPlayer(Game* g) : ScanPlayer(g) {};
  PlayerAction GetAction() override;

 protected:
  Card CardToMakeASet(Card c1, Card c2);
};

/*
  Does what MatchPlayer does, but also remembers what the set was in the last
  turn. It looks at the new cards that were added, and checks if those cards
  were needed to complete a set with something on the table.
*/
class RememberTheLastTurnPlayer : public MatchPlayer {
 public:
  RememberTheLastTurnPlayer(Game* g) : MatchPlayer(g) {};
  PlayerAction GetAction() override;

 private:
  std::map<Card, std::pair<int, int>> cards_to_look_for_;
  std::vector<int> last_set_;
};

/*
  Same as RememberTheLastTurnPlayer except it only remembers three of the cards
  that were needed.
*/
class RememberTheLastTurnPlayerForgetful : public MatchPlayer {
 public:
  RememberTheLastTurnPlayerForgetful(Game* g) : MatchPlayer(g) {};
  PlayerAction GetAction() override;

 private:
  void RememberSomeToLookFor(
      size_t n, std::vector<std::pair<Card, std::pair<int, int>>> all_trips);

  std::map<Card, std::pair<int, int>> cards_to_look_for_;
  std::vector<int> last_set_;
};

}  // namespace set

#endif  // PLAYER_H_
