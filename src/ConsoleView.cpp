#include "ConsoleView.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>




namespace {

// Turns "3" into 3. Rejects "", "abc", "3x", "99999999999". Never throws.
bool parseInt(const std::string &text, int &out) {
  std::istringstream in(text);
  int value;
  if (!(in >> value))
    return false;
  std::string leftover;
  if (in >> leftover) // something after the number, like "3x" or "3 4"
    return false;
  out = value;
  return true;
}




std::string upper(std::string s) {
  for (char &ch : s)
    ch = (char)std::toupper((unsigned char)ch);
  return s;
}

} // namespace

ConsoleView::ConsoleView(VideoPoker &game) : game(game) {}




void ConsoleView::run() {
  std::cout << "=== Jacks or Better Video Poker ===\n";
  bool keepPlaying = true;
  while (keepPlaying) {
    switch (game.phase()) {
    case Phase::Betting:  keepPlaying = bettingTurn();  break;
    case Phase::Holding:  keepPlaying = holdingTurn();  break;
    case Phase::GameOver: keepPlaying = gameOverTurn(); break;
    }
  }
  std::cout << "Thanks for playing. Final credits: " << game.credits() << '\n';
}





// "  [1] KH*  [2] KS*  [3] 4D   [4] 9C   [5] 2H"
void ConsoleView::showHand() const {
  const Hand &h = game.hand();
  std::cout << ' ';
  for (int i = 0; i < h.size(); i++) {
    std::string card = h.at(i).toString() + (game.isHeld(i) ? "*" : "");
    card.resize(4, ' '); // keeps the columns lined up ("10H*" is longest)
    std::cout << " [" << (i + 1) << "] " << card;
  }
  std::cout << '\n';
}




// "THREE OF A KIND! Pays 15.   Credits: 110"  
void ConsoleView::showResult() const {
  HandRank r = game.lastResult();
  if (r == HandRank::Nothing)
    std::cout << "No win.";
  else
    std::cout << upper(VideoPoker::rankName(r)) << "! Pays "
              << game.lastPayout() << '.';
  std::cout << "   Credits: " << game.credits() << "\n\n";
}





bool ConsoleView::bettingTurn() {
  std::cout << "Credits: " << game.credits() << "   Bet (" << VideoPoker::MinBet
            << '-' << game.maxBet() << ", q to quit): ";
  std::string line;
  if (!std::getline(std::cin, line) || line == "q" || line == "Q")
    return false;

  int bet;
  if (!parseInt(line, bet)) {
    std::cout << "  Please type a number.\n";
    return true;
  }
  if (!game.deal(bet)) { // the GAME decides whether the bet is legal
    if (bet > game.credits())
      std::cout << "  Not enough credits.\n";
    else
      std::cout << "  Bet must be " << VideoPoker::MinBet << " to "
                << game.maxBet() << ".\n";
    return true;
  }
  showHand();
  return true;
}





bool ConsoleView::holdingTurn() {
  std::cout << "Toggle holds (e.g. \"1 2\"), or press Enter to draw (q to quit): ";
  std::string line;
  if (!std::getline(std::cin, line) || line == "q" || line == "Q")
    return false;

  bool blank = std::all_of(line.begin(), line.end(),
                           [](unsigned char ch) { return std::isspace(ch); });
  if (blank) { // Enter: draw and show the result
    game.draw();
    showHand();
    showResult();
    return true;
  }




  // Each number toggles one card. Bad tokens are reported, not fatal.
  std::istringstream words(line);
  std::string word;
  bool anyBad = false;
  while (words >> word) {
    int pos;
    if (!parseInt(word, pos) || !game.toggleHold(pos - 1))
      anyBad = true;
  }
  if (anyBad)
    std::cout << "  Use card numbers 1-" << VideoPoker::HandSize
              << ", separated by spaces.\n";
  showHand();
  return true;
}




bool ConsoleView::gameOverTurn() {
  std::cout << "GAME OVER - you're out of credits. Play again? (y/n): ";
  std::string line;
  if (!std::getline(std::cin, line) || (line != "y" && line != "Y"))
    return false;
  game.newGame();
  std::cout << '\n';
  return true;
}
