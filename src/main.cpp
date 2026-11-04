// Video Poker 
//
//   ./build/game          -> TuiView     (FTXUI: colors + arrow keys)
//   ./build/game --text   -> ConsoleView (plain cout/cin)
//   ./build/game --test   -> rule tests, PASS/FAIL

#include "ConsoleView.hpp"
#include "GameView.hpp"
#include "Tests.hpp"
#include "TuiView.hpp"
#include "VideoPoker.hpp"

#include <memory>
#include <string>

int main(int argc, char **argv) {
  std::string mode = (argc > 1) ? argv[1] : "";
  if (mode == "--test")
    return runTests();

  VideoPoker game;

  // Polymorphism: main() does never know which kind of view it have.
  // It only ever talks to it through a GameView pointer.
  std::unique_ptr<GameView> view;
  if (mode == "--text")
    view = std::make_unique<ConsoleView>(game);
  else
    view = std::make_unique<TuiView>(game);

  view->run();
  return 0;
}
