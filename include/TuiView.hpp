#pragma once

#include "GameView.hpp"
#include "VideoPoker.hpp"

#include <string>

// A VIEW: shows the same VideoPoker game with FTXUI (colors, arrow keys).
//
// The only things this class remembers are about the SCREEN, not the game:
// which card the cursor is on, which bet is highlighted, and a message.
// Every rule question goes to the game.
class TuiView : public GameView {
private:
  VideoPoker &game; // uses the game, does not own it

  int cursor_ = 0;   // which card the arrow keys are on (0-4)
  int betChoice_ = 1; // bet highlighted on the betting screen (1-5)
  std::string message_ = "Choose your bet, then press ENTER to deal.";

  // Key handlers: each one turns a key into ONE call on the game.
  bool onBettingKey(const std::string &key);
  bool onHoldingKey(const std::string &key);
  bool onGameOverKey(const std::string &key);
  std::string resultMessage() const;

public:
  explicit TuiView(VideoPoker &game);
  void run() override;
};
