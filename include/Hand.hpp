#pragma once

// A Hand HAS cards (composition). It is not a vector and not a Deck,
// so it does not inherit from either one. It just keeps a private
// vector and exposes the few operations a hand actually needs.

#include "Card.hpp"

#include <vector>

class Hand {
private:
  std::vector<Card> cards_;

public:
  void addCard(const Card &c);
  void clear();
  int size() const;

  // Both throw std::out_of_range if i is not a valid position.
  const Card &at(int i) const;
  void replace(int i, const Card &c); // swap one card for another (the draw)
};
