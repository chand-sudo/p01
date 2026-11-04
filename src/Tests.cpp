#include "Tests.hpp"

#include "Deck.hpp"
#include "Hand.hpp"
#include "VideoPoker.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Card numbers: suit * 13 + rank.
// Suits: 0 Clubs, 1 Diamonds, 2 Hearts, 3 Spades.
// Ranks: 0 Ace, 1 Two, ... 9 Ten, 10 Jack, 11 Queen, 12 King.
enum Suit { C = 0, D = 1, H = 2, S = 3 };
const int A = 0, J = 10, Q = 11, K = 12;

Card card(int rank, Suit suit) { return Card(suit * 13 + rank); }

// card(rank-1...) is awkward to read, so ranks are written as the number
// on the card: n(2, H) is the 2 of Hearts, n(10, S) is the 10 of Spades.
Card n(int faceValue, Suit suit) { return card(faceValue - 1, suit); }

Hand makeHand(const std::vector<Card> &cards) {
  Hand h;
  for (const Card &c : cards)
    h.addCard(c);
  return h;
}

int failures = 0;

void check(bool ok, const std::string &name) {
  std::cout << (ok ? "PASS  " : "FAIL  ") << name << '\n';
  if (!ok)
    failures++;
}

void checkRank(const std::vector<Card> &cards, HandRank expected,
               const std::string &name) {
  check(VideoPoker::evaluate(makeHand(cards)) == expected, name);
}

// ------------------------------------------------------------- every rank
void testEveryRank() {
  std::cout << "-- Every hand rank --\n";
  checkRank({n(2, C), n(5, D), n(7, H), n(9, S), card(J, C)},
            HandRank::Nothing, "Nothing: 2 5 7 9 J mixed");
  checkRank({card(J, C), card(J, D), n(4, H), n(7, S), n(9, C)},
            HandRank::JacksOrBetter, "Jacks or Better: J J 4 7 9");
  checkRank({n(8, C), n(8, D), n(4, H), n(4, S), card(K, C)},
            HandRank::TwoPair, "Two Pair: 8 8 4 4 K");
  checkRank({n(7, C), n(7, D), n(7, H), n(2, S), card(K, C)},
            HandRank::ThreeOfAKind, "Three of a Kind: 7 7 7 2 K");
  checkRank({n(5, C), n(6, D), n(7, H), n(8, S), n(9, C)},
            HandRank::Straight, "Straight: 5 6 7 8 9");
  checkRank({n(2, H), n(5, H), n(7, H), n(9, H), card(K, H)},
            HandRank::Flush, "Flush: 2 5 7 9 K hearts");
  checkRank({n(3, C), n(3, D), n(3, H), n(6, S), n(6, C)},
            HandRank::FullHouse, "Full House: 3 3 3 6 6");
  checkRank({n(9, C), n(9, D), n(9, H), n(9, S), n(2, C)},
            HandRank::FourOfAKind, "Four of a Kind: 9 9 9 9 2");
  checkRank({n(5, S), n(6, S), n(7, S), n(8, S), n(9, S)},
            HandRank::StraightFlush, "Straight Flush: 5-9 spades");
  checkRank({n(10, D), card(J, D), card(Q, D), card(K, D), card(A, D)},
            HandRank::RoyalFlush, "Royal Flush: 10 J Q K A diamonds");
}

// ------------------------------------------------------------- edge cases
void testEdgeCases() {
  std::cout << "-- Pairs and straights --\n";
  checkRank({n(10, C), n(10, D), n(3, H), n(6, S), n(8, C)},
            HandRank::Nothing, "pair of 10s -> Nothing");
  checkRank({card(J, H), card(J, S), n(3, H), n(6, S), n(8, C)},
            HandRank::JacksOrBetter, "pair of Jacks -> Jacks or Better");
  checkRank({card(A, C), card(A, D), n(3, H), n(6, S), n(8, C)},
            HandRank::JacksOrBetter, "pair of Aces -> Jacks or Better");
  checkRank({card(A, C), n(2, D), n(3, H), n(4, S), n(5, C)},
            HandRank::Straight, "A-2-3-4-5 -> Straight");
  checkRank({n(10, C), card(J, D), card(Q, H), card(K, S), card(A, C)},
            HandRank::Straight, "10-J-Q-K-A mixed suits -> Straight");
  checkRank({card(Q, C), card(K, D), card(A, H), n(2, S), n(3, C)},
            HandRank::Nothing, "Q-K-A-2-3 -> not a straight");
  checkRank({card(A, H), n(2, H), n(3, H), n(4, H), n(5, H)},
            HandRank::StraightFlush, "A-2-3-4-5 hearts -> Straight Flush");
}

// ------------------------------------------------------------- payouts
void testPayouts() {
  std::cout << "-- Payouts --\n";
  check(5 * VideoPoker::multiplier(HandRank::RoyalFlush, 5) == 4000,
        "royal flush at bet 5 pays 4000");
  bool all250 = true;
  for (int bet = 1; bet <= 4; bet++)
    if (VideoPoker::multiplier(HandRank::RoyalFlush, bet) != 250)
      all250 = false;
  check(all250, "royal flush pays 250 per credit at bets 1-4");
  check(VideoPoker::multiplier(HandRank::FullHouse, 3) == 9,
        "full house pays 9 per credit");
  check(VideoPoker::multiplier(HandRank::JacksOrBetter, 5) == 1,
        "jacks or better pays 1 (bet back)");
  check(VideoPoker::multiplier(HandRank::Nothing, 5) == 0,
        "nothing pays 0");

  // Whole game: bet 5, get dealt a royal flush, hold everything, draw.
  VideoPoker g(100);
  g.setNextDeck(Deck({n(10, S), card(J, S), card(Q, S), card(K, S),
                      card(A, S), n(2, C), n(3, C), n(4, C), n(5, C),
                      n(6, C)}));
  g.deal(5);
  for (int i = 0; i < 5; i++)
    g.toggleHold(i);
  g.draw();
  check(g.lastResult() == HandRank::RoyalFlush && g.lastPayout() == 4000 &&
            g.credits() == 95 + 4000,
        "game: royal flush at bet 5 -> credits 100 - 5 + 4000");

  // Whole game: bet 2, a flush pays 2 * 6 = 12.
  VideoPoker f(100);
  f.setNextDeck(Deck({n(2, H), n(5, H), n(7, H), n(9, H), card(K, H)}));
  f.deal(2);
  for (int i = 0; i < 5; i++)
    f.toggleHold(i);
  f.draw();
  check(f.lastPayout() == 12 && f.credits() == 98 + 12,
        "game: flush at bet 2 pays 12");
}



// ------------------------------------------------------------- hold / draw
void testHoldAndDraw() {
  std::cout << "-- Hold and draw --\n";
  // First 5 cards are dealt; the next 5 are the replacements, in order.
  VideoPoker g(100);
  g.setNextDeck(Deck({card(K, H), n(4, D), card(K, S), n(9, C), n(2, H),
                      card(K, C), n(7, S), n(3, D), n(8, D), n(8, H)}));
  g.deal(5);
  check(g.credits() == 95, "bet is subtracted right away");
  g.toggleHold(0); // KH
  g.toggleHold(2); // KS
  check(g.isHeld(0) && g.isHeld(2) && !g.isHeld(1), "holds are recorded");
  g.toggleHold(4);
  g.toggleHold(4); // on, then off again
  check(!g.isHeld(4), "toggling twice un-holds a card");
  g.draw();

  const Hand &h = g.hand();
  check(h.at(0).getValue() == card(K, H).getValue() &&
            h.at(2).getValue() == card(K, S).getValue(),
        "held cards survive draw()");
  check(h.at(1).getValue() == card(K, C).getValue() &&
            h.at(3).getValue() == n(7, S).getValue() &&
            h.at(4).getValue() == n(3, D).getValue(),
        "unheld cards are replaced, in order");
  check(g.lastResult() == HandRank::ThreeOfAKind && g.lastPayout() == 15 &&
            g.credits() == 110,
        "K K K after draw: three of a kind pays 15, credits 110");
}




// ------------------------------------------------------------- phases
void testPhases() {
  std::cout << "-- Phase rules --\n";
  VideoPoker g(100);
  check(!g.draw(), "draw() refused before dealing");
  check(!g.toggleHold(0), "toggleHold() refused before dealing");
  check(!g.deal(0) && !g.deal(6) && !g.deal(-1), "bets of 0, 6, -1 refused");
  check(g.credits() == 100 && g.phase() == Phase::Betting,
        "refused calls change nothing");
  check(g.deal(1) && g.phase() == Phase::Holding, "deal(1) -> Holding");
  check(!g.deal(1), "deal() refused while holding");
  check(!g.toggleHold(5) && !g.toggleHold(-1), "bad hold index refused");
  check(g.draw() && g.phase() != Phase::Holding, "draw() ends the hand");
  check(!g.draw(), "draw() twice is refused");



  VideoPoker poor(3);
  check(!poor.deal(4) && poor.credits() == 3, "can't bet more than credits");




  // Lose everything: bet the last 2 credits on a losing hand.
  VideoPoker broke(2);
  broke.setNextDeck(Deck({n(2, C), n(5, D), n(7, H), n(9, S), card(J, C)}));
  broke.deal(2);
  for (int i = 0; i < 5; i++)
    broke.toggleHold(i);
  broke.draw();
  check(broke.credits() == 0 && broke.phase() == Phase::GameOver,
        "0 credits -> GameOver");
  check(!broke.deal(1), "deal() refused after game over");
  check(broke.newGame() && broke.credits() == 2 &&
            broke.phase() == Phase::Betting,
        "newGame() restores starting credits");
}




// ------------------------------------------------------------- Hand
void testHand() {
  std::cout << "-- Hand --\n";
  Hand h;
  h.addCard(card(A, S));
  h.addCard(n(7, D));
  check(h.size() == 2, "addCard grows the hand");
  bool threw = false;
  try {
    h.at(2);
  } catch (const std::out_of_range &) {
    threw = true;
  }
  check(threw, "at() throws out_of_range on a bad index");
  h.replace(1, card(K, C));
  check(h.at(1).getValue() == card(K, C).getValue(), "replace swaps a card");
  h.clear();
  check(h.size() == 0, "clear empties the hand");
}




} // namespace

int runTests() {
  failures = 0;
  testEveryRank();
  testEdgeCases();
  testPayouts();
  testHoldAndDraw();
  testPhases();
  testHand();
  std::cout << '\n'
            << (failures == 0 ? "ALL TESTS PASSED"
                              : std::to_string(failures) + " TEST(S) FAILED")
            << '\n';
  return failures == 0 ? 0 : 1;
}
