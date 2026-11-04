#include "VideoPoker.hpp"

#include <algorithm>
#include <vector>

// Card ranks from Card::getRank(): Ace = 0, Two = 1, ... Ten = 9,
// Jack = 10, Queen = 11, King = 12.
namespace {
const int Ace = 0;
const int Ten = 9;
const int Jack = 10;
const int Queen = 11;
const int King = 12;
} // namespace

VideoPoker::VideoPoker(int startingCredits)
    : startingCredits_(startingCredits), credits_(startingCredits) {
  if (credits_ <= 0)
    phase_ = Phase::GameOver;
}

// ---------------------------------------------------------------- actions

bool VideoPoker::deal(int bet) {
  if (phase_ != Phase::Betting)
    return false;
  if (bet < MinBet || bet > maxBet())
    return false;

  credits_ -= bet; // the bet comes off right away
  bet_ = bet;
  lastResult_ = HandRank::Nothing;
  lastPayout_ = 0;

  dealFreshHand();
  phase_ = Phase::Holding;
  return true;
}

bool VideoPoker::toggleHold(int index) {
  if (phase_ != Phase::Holding)
    return false;
  if (index < 0 || index >= HandSize)
    return false;
  held_[index] = !held_[index];
  return true;
}

bool VideoPoker::draw() {
  if (phase_ != Phase::Holding)
    return false;

  // Replace every card that is NOT held, left to right, from the same deck.
  for (int i = 0; i < HandSize; i++)
    if (!held_[i])
      hand_.replace(i, deck_.deal());

  lastResult_ = evaluate(hand_);
  lastPayout_ = bet_ * multiplier(lastResult_, bet_);
  credits_ += lastPayout_;

  phase_ = (credits_ <= 0) ? Phase::GameOver : Phase::Betting;
  return true;
}

bool VideoPoker::newGame() {
  if (phase_ != Phase::GameOver)
    return false;
  credits_ = startingCredits_;
  bet_ = 0;
  hand_.clear();
  held_.fill(false);
  lastResult_ = HandRank::Nothing;
  lastPayout_ = 0;
  phase_ = Phase::Betting;
  return true;
}

void VideoPoker::setNextDeck(const Deck &stacked) {
  nextDeck_ = stacked;
  hasNextDeck_ = true;
}

// ---------------------------------------------------------------- helpers

void VideoPoker::prepareDeck() {
  if (hasNextDeck_) { // tests: use the stacked deck exactly once
    deck_ = nextDeck_;
    hasNextDeck_ = false;
  } else {            // real play: fresh shuffled deck every hand
    deck_.reset();
    deck_.shuffle();
  }
}

void VideoPoker::dealFreshHand() {
  prepareDeck();
  hand_.clear();
  held_.fill(false);
  for (int i = 0; i < HandSize; i++)
    hand_.addCard(deck_.deal());
}

// ---------------------------------------------------------------- questions

Phase VideoPoker::phase() const { return phase_; }
int VideoPoker::credits() const { return credits_; }
int VideoPoker::bet() const { return bet_; }
int VideoPoker::maxBet() const { return std::min(MaxBet, credits_); }
const Hand &VideoPoker::hand() const { return hand_; }
bool VideoPoker::hasPlayedHand() const { return hand_.size() == HandSize; }
HandRank VideoPoker::lastResult() const { return lastResult_; }
int VideoPoker::lastPayout() const { return lastPayout_; }

bool VideoPoker::isHeld(int index) const {
  if (index < 0 || index >= HandSize)
    return false;
  return held_[index];
}

// ---------------------------------------------------------------- rules

HandRank VideoPoker::evaluate(const Hand &h) {
  if (h.size() != HandSize)
    return HandRank::Nothing;

  // 1. Count how many of each rank, and collect the ranks.
  int rankCount[13] = {0};
  std::vector<int> ranks;
  bool flush = true;
  for (int i = 0; i < HandSize; i++) {
    rankCount[h.at(i).getRank()]++;
    ranks.push_back(h.at(i).getRank());
    if (h.at(i).getSuit() != h.at(0).getSuit())
      flush = false;
  }
  std::sort(ranks.begin(), ranks.end());

  // 2. Straight: five different ranks in a row. Ace is 0, so A-2-3-4-5
  //    is already in a row; 10-J-Q-K-A is the one special case.
  bool allDifferent =
      std::adjacent_find(ranks.begin(), ranks.end()) == ranks.end();
  bool aceHigh = (ranks == std::vector<int>{Ace, Ten, Jack, Queen, King});
  bool straight = allDifferent && (ranks[4] - ranks[0] == 4 || aceHigh);

  if (straight && flush)
    return aceHigh ? HandRank::RoyalFlush : HandRank::StraightFlush;

  // 3. Look at the groups: how many pairs, trips, quads.
  int pairs = 0, trips = 0, quads = 0;
  bool highPair = false; // a pair of Jacks, Queens, Kings, or Aces
  for (int r = 0; r < 13; r++) {
    if (rankCount[r] == 4) quads++;
    if (rankCount[r] == 3) trips++;
    if (rankCount[r] == 2) {
      pairs++;
      if (r == Ace || r >= Jack)
        highPair = true;
    }
  }

  if (quads == 1)              return HandRank::FourOfAKind;
  if (trips == 1 && pairs == 1) return HandRank::FullHouse;
  if (flush)                   return HandRank::Flush;
  if (straight)                return HandRank::Straight;
  if (trips == 1)              return HandRank::ThreeOfAKind;
  if (pairs == 2)              return HandRank::TwoPair;
  if (pairs == 1 && highPair)  return HandRank::JacksOrBetter;
  return HandRank::Nothing;
}

int VideoPoker::multiplier(HandRank r, int bet) {
  switch (r) {
  case HandRank::RoyalFlush:    return bet == MaxBet ? 800 : 250;
  case HandRank::StraightFlush: return 50;
  case HandRank::FourOfAKind:   return 25;
  case HandRank::FullHouse:     return 9;
  case HandRank::Flush:         return 6;
  case HandRank::Straight:      return 4;
  case HandRank::ThreeOfAKind:  return 3;
  case HandRank::TwoPair:       return 2;
  case HandRank::JacksOrBetter: return 1;
  case HandRank::Nothing:       return 0;
  }
  return 0;
}

std::string VideoPoker::rankName(HandRank r) {
  switch (r) {
  case HandRank::RoyalFlush:    return "Royal Flush";
  case HandRank::StraightFlush: return "Straight Flush";
  case HandRank::FourOfAKind:   return "Four of a Kind";
  case HandRank::FullHouse:     return "Full House";
  case HandRank::Flush:         return "Flush";
  case HandRank::Straight:      return "Straight";
  case HandRank::ThreeOfAKind:  return "Three of a Kind";
  case HandRank::TwoPair:       return "Two Pair";
  case HandRank::JacksOrBetter: return "Jacks or Better";
  case HandRank::Nothing:       return "Nothing";
  }
  return "Nothing";
}
