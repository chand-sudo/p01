#pragma once

#include "GameView.hpp"
#include "VideoPoker.hpp"

#include <string>

// A VIEW: shows the game with cout and reads commands with getline.
// ASCII only (no suit symbols) so it works in any Windows console.
// It never decides a rule; it just asks the game and tells the game.
class ConsoleView : public GameView {
private:
  VideoPoker &game; // uses the game, does not own it

  void showHand() const;
  void showResult() const;
  bool bettingTurn();  // returns false when the player quits
  bool holdingTurn();  // returns false when the player quits
  bool gameOverTurn(); // returns false when the player quits

public:
  explicit ConsoleView(VideoPoker &game);
  void run() override;
};
