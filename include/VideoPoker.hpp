#pragma once

// THE GAME (the "model"): Jacks or Better video poker.
// It stores state and enforces rules. It never prints and never reads
// input -- no <iostream>, no FTXUI. The views do all of that.

#include "Deck.hpp"
#include "Hand.hpp"

#include <array>
#include <string>

enum class Phase { Betting, Holding, GameOver };

enum class HandRank {
  Nothing,
  JacksOrBetter,
  TwoPair,
  ThreeOfAKind,
  Straight,
  Flush,
  FullHouse,
  FourOfAKind,
  StraightFlush,
  RoyalFlush
};

class VideoPoker {
public:
  static constexpr int HandSize = 5;
  static constexpr int MinBet = 1;
  static constexpr int MaxBet = 5;

private:
  // --- STATE ---
  Deck deck_;
  Hand hand_;
  std::array<bool, HandSize> held_{};
  int startingCredits_;
  int credits_;
  int bet_ = 0;
  Phase phase_ = Phase::Betting;
  HandRank lastResult_ = HandRank::Nothing;
  int lastPayout_ = 0;

  // Tests only: if set, the next deal() uses this deck instead of shuffling.
  Deck nextDeck_;
  bool hasNextDeck_ = false;

  void prepareDeck();
  void dealFreshHand();

public:
  explicit VideoPoker(int startingCredits = 100);

  // --- ACTIONS --- each returns false and changes nothing if not allowed.
  bool deal(int bet);         // Betting -> Holding
  bool toggleHold(int index); // only during Holding
  bool draw();                // Holding -> Betting (or GameOver); scores + pays
  bool newGame();             // GameOver -> Betting, credits reset

  // Tests only: the next deal() uses this exact deck (no shuffle).
  void setNextDeck(const Deck &stacked);

  // --- QUESTIONS for the views ---
  Phase phase() const;
  int credits() const;
  int bet() const;            // bet for the current / most recent hand
  int maxBet() const;         // largest bet allowed right now (1..5, never above credits)
  const Hand &hand() const;
  bool isHeld(int index) const;
  bool hasPlayedHand() const; // true once at least one hand has been dealt
  HandRank lastResult() const;
  int lastPayout() const;

  // --- PURE RULES (static: no game object needed, easy to test) ---
  static HandRank evaluate(const Hand &h);
  static int multiplier(HandRank r, int bet); // pays per credit bet
  static std::string rankName(HandRank r);    // "Full House", ...
};
