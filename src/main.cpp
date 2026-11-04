// Video Poker (Jacks or Better)
//
//   ./build/game --text   -> ConsoleView (plain cout/cin)
//   ./build/game --test   -> rule tests, PASS/FAIL
//   ./build/game          -> TuiView (FTXUI) -- added in Milestone 2;
//                            until then it also runs the ConsoleView.

#include "ConsoleView.hpp"
#include "GameView.hpp"
#include "Tests.hpp"
#include "VideoPoker.hpp"

#include <memory>
#include <string>




int main(int argc, char **argv) {
  std::string mode = (argc > 1) ? argv[1] : "";
  if (mode == "--test")
    return runTests();

  VideoPoker game;



  
  // Polymorphism: main only knows it has "some GameView".
  std::unique_ptr<GameView> view;
  view = std::make_unique<ConsoleView>(game); // M2: TuiView when not --text

  view->run();
  return 0;
}
