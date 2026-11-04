#include "TuiView.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <string>

using namespace ftxui; // fine in a .cpp file, never in a header

// =============================================================== drawing
// These functions only turn data into pictures. No rules in here.
namespace {

// One face-up card: red for diamonds/hearts, white for clubs/spades.
// Same look as the professor's demo in Resources/ftxui/main.cpp.
Element drawCard(const Card &card, bool selected, bool held) {
  Color suitColor = card.isRed() ? Color::Red : Color::White;
  Element body = vbox({
                     hbox({text(card.rankSymbol()), filler(),
                           text(card.suitSymbol())}),
                     filler(),
                     text(card.suitSymbol()) | bold | center,
                     filler(),
                     hbox({text(card.suitSymbol()), filler(),
                           text(card.rankSymbol())}),
                 }) |
                 color(suitColor) | size(WIDTH, EQUAL, 9) |
                 size(HEIGHT, EQUAL, 7);

  body = selected ? (body | borderDouble) : (body | border);
  Element label = held ? text("HOLD") | bold | color(Color::Yellow) | center
                       : text(" ");
  return vbox({body, label});
}

// A blank card back, shown before the first hand is dealt.
Element drawCardBack() {
  return vbox({vbox({filler()}) | size(WIDTH, EQUAL, 9) |
                   size(HEIGHT, EQUAL, 7) | border | dim,
               text(" ")});
}

// The five cards in a row (or five backs if nothing is dealt yet).
Element drawHand(const VideoPoker &game, int cursor) {
  Elements row;
  bool holding = game.phase() == Phase::Holding;
  for (int i = 0; i < VideoPoker::HandSize; i++) {
    if (i > 0)
      row.push_back(text(" "));
    if (!game.hasPlayedHand())
      row.push_back(drawCardBack());
    else
      row.push_back(drawCard(game.hand().at(i), holding && i == cursor,
                             holding && game.isHeld(i)));
  }
  return hbox(std::move(row)) | center;
}

// "BET  [1] 2 3 4 5" -- shows which bet is highlighted.
Element drawBetPicker(int betChoice) {
  Elements parts = {text("BET  ")};
  for (int b = VideoPoker::MinBet; b <= VideoPoker::MaxBet; b++) {
    Element num = text(" " + std::to_string(b) + " ");
    parts.push_back(b == betChoice ? num | inverted | bold : num | dim);
  }
  return hbox(std::move(parts)) | center;
}

// The pay table, with the row for the last result highlighted.
Element drawPayTable(HandRank highlight, int bet) {
  const HandRank ranks[] = {
      HandRank::RoyalFlush,   HandRank::StraightFlush, HandRank::FourOfAKind,
      HandRank::FullHouse,    HandRank::Flush,         HandRank::Straight,
      HandRank::ThreeOfAKind, HandRank::TwoPair,       HandRank::JacksOrBetter};
  int shownBet = bet > 0 ? bet : 1;

  Elements rows;
  for (HandRank r : ranks) {
    int pays = shownBet * VideoPoker::multiplier(r, shownBet);
    Element row = hbox({text(VideoPoker::rankName(r)), filler(),
                        text(std::to_string(pays))});
    rows.push_back(r == highlight ? row | inverted | bold : row);
  }
  return vbox({text("PAYS (bet " + std::to_string(shownBet) + ")") | bold |
                   center,
               separator(), vbox(std::move(rows))}) |
         size(WIDTH, EQUAL, 24) | border;
}

// The key help line changes with the phase, so the player always
// sees exactly which keys work right now.
std::string keysFor(Phase phase) {
  switch (phase) {
  case Phase::Betting:
    return "↑ ↓ or 1-5  Bet     ENTER  Deal     Q  Quit";
  case Phase::Holding:
    return "← →  Move     SPACE  Hold     ENTER  Draw     Q  Quit";
  case Phase::GameOver:
    return "N  New game     Q  Quit";
  }
  return "";
}

// Turns an FTXUI key event into a short name like "left" or "3".
// Anything we don't use becomes "".
std::string keyName(const Event &e) {
  if (e == Event::ArrowLeft)  return "left";
  if (e == Event::ArrowRight) return "right";
  if (e == Event::ArrowUp)    return "up";
  if (e == Event::ArrowDown)  return "down";
  if (e == Event::Return)     return "enter";
  if (e == Event::Character(' ')) return "space";
  if (e.is_character()) {
    std::string c = e.character();
    if (c == "Q") return "q";
    if (c == "N") return "n";
    return c;
  }
  return "";
}

} // namespace

// =============================================================== the view

TuiView::TuiView(VideoPoker &game) : game(game) {}

// Words for the last result. Turning a HandRank into a sentence is
// presentation, so it belongs in the view. The NUMBERS come from the game.
std::string TuiView::resultMessage() const {
  HandRank r = game.lastResult();
  if (r == HandRank::Nothing)
    return "No win. Choose a bet and press ENTER to play again.";
  return VideoPoker::rankName(r) + "!  Pays " +
         std::to_string(game.lastPayout()) + ".";
}

bool TuiView::onBettingKey(const std::string &key) {
  if (key == "up" && betChoice_ < VideoPoker::MaxBet) betChoice_++;
  else if (key == "down" && betChoice_ > VideoPoker::MinBet) betChoice_--;
  else if (key.size() == 1 && key[0] >= '1' && key[0] <= '5')
    betChoice_ = key[0] - '0';
  else if (key == "enter") {
    if (game.deal(betChoice_)) { // the GAME checks the bet
      cursor_ = 0;
      message_ = "Hold the cards you want, then press ENTER to draw.";
    } else {
      message_ = "Not enough credits for a bet of " +
                 std::to_string(betChoice_) + ".";
    }
  } else
    return false;
  return true;
}

bool TuiView::onHoldingKey(const std::string &key) {
  const int last = VideoPoker::HandSize - 1;
  if (key == "left")       cursor_ = (cursor_ == 0) ? last : cursor_ - 1;
  else if (key == "right") cursor_ = (cursor_ == last) ? 0 : cursor_ + 1;
  else if (key == "space") game.toggleHold(cursor_);
  else if (key == "enter") {
    game.draw();
    message_ = (game.phase() == Phase::GameOver)
                   ? "Out of credits. GAME OVER."
                   : resultMessage();
  } else
    return false;
  return true;
}

bool TuiView::onGameOverKey(const std::string &key) {
  if (key != "n")
    return false;
  game.newGame();
  betChoice_ = 1;
  message_ = "New game! Choose your bet, then press ENTER to deal.";
  return true;
}

void TuiView::run() {
  auto screen = ScreenInteractive::Fullscreen();

  // RENDERER: only READS from the game. It never changes anything.
  auto renderer = Renderer([&] {
    Element betLine = game.phase() == Phase::Betting
                          ? drawBetPicker(betChoice_)
                          : text("BET  " + std::to_string(game.bet())) |
                                center;

    Element table = vbox({
        text("♠ ♥  JACKS OR BETTER  ♦ ♣") | bold | center,
        separator(),
        drawHand(game, cursor_),
        separator(),
        hbox({text(" CREDITS  " + std::to_string(game.credits())) | bold,
              filler(), betLine, filler(),
              text("LAST WIN  " + std::to_string(game.lastPayout()) + " ")}),
        separator(),
        text(message_) | bold | color(Color::Yellow) | center,
        separator(),
        text(keysFor(game.phase())) | dim | center,
    });

    HandRank highlight = (game.phase() == Phase::Holding)
                             ? HandRank::Nothing
                             : game.lastResult();
    // While choosing a bet, show what that bet would pay.
    int tableBet = (game.phase() == Phase::Betting) ? betChoice_ : game.bet();
    // Fixed size, centered in the terminal, so it looks the same on any
    // window size instead of stretching to fill it.
    return hbox({table | size(WIDTH, EQUAL, 70),
                 drawPayTable(highlight, tableBet)}) |
           border | center;
  });

  // EVENT HANDLER: one key -> one call into the game. No poker rules here.
  auto app = CatchEvent(renderer, [&](Event e) {
    std::string key = keyName(e);
    if (key == "q") {
      screen.Exit();
      return true;
    }
    switch (game.phase()) {
    case Phase::Betting:  return onBettingKey(key);
    case Phase::Holding:  return onHoldingKey(key);
    case Phase::GameOver: return onGameOverKey(key);
    }
    return false;
  });

  screen.Loop(app);
}
