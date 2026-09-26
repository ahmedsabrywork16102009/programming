#include "../../../libraries/cpp_libraries/input.h"
#include "../../../libraries/cpp_libraries/output.h"
#include "../../../libraries/cpp_libraries/process.h"

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// =================================================================================
//  Enums & Structs
// =================================================================================

enum enChoice { Rock = 1, Paper = 2, Scissor = 3 };
enum enWinner { Player = 1, Computer = 2, Draw = 3 };

struct stRoundInfo {
  int roundNumber;
  enChoice playerChoice;
  enChoice computerChoice;
  enWinner RoundWinner;
  string winnerName;
};

struct stGameInfo {
  int numberOfRounds;
  vector<stRoundInfo> roundsInfo;
  int playerWinTimes = 0;
  int computerWinTimes = 0;
  int drawTimes = 0;
  enWinner gameWinner;
  string winnerName;
};

// =================================================================================
//  Game Logic
// =================================================================================

enWinner getWinner(enChoice playerChoice, enChoice computerChoice) {
  if (playerChoice == computerChoice) {
    return Draw;
  }
  switch (playerChoice) {
  case enChoice::Rock:
    return (computerChoice == enChoice::Paper) ? Computer : Player;
  case enChoice::Paper:
    return (computerChoice == enChoice::Scissor) ? Computer : Player;
  case enChoice::Scissor:
    return (computerChoice == enChoice::Rock) ? Computer : Player;
  }
  return Player;
}

string getWinnerName(enWinner winner) {
  string arr[3] = {"Player", "Computer", "Draw"};
  return arr[winner - 1];
}

string getChoiceName(enChoice choice) {
  string arr[3] = {"Rock", "Paper", "Scissor"};
  return arr[choice - 1];
}

enWinner getFinalGameWinner(int playerWinTimes, int computerWinTimes) {
  if (playerWinTimes > computerWinTimes) return Player;
  else if (computerWinTimes > playerWinTimes) return Computer;
  else return Draw;
}

// =================================================================================
//  Display
// =================================================================================

void printRoundResults(int roundNumber, enChoice playerChoice,
                       enChoice computerChoice, string winnerName) {
  cout << "----------------- Round [ " << roundNumber << " ] -----------------\n";
  cout << "Player choice: " << getChoiceName(playerChoice) << "\n";
  cout << "Computer choice: " << getChoiceName(computerChoice) << "\n\n";
  cout << "=============================================\n";
  cout << "======== Winner: " << setw(8) << winnerName << " ========\n";
  cout << "=============================================\n\n\n";
}

void printGameResults(const stGameInfo &gameInfo) {
  cout << "-------- Winner [ " << setw(8) << gameInfo.winnerName << " ] --------\n";
  cout << "Number of rounds   : " << gameInfo.numberOfRounds << "\n";
  cout << "Player win times   : " << gameInfo.playerWinTimes << "\n";
  cout << "Computer win times : " << gameInfo.computerWinTimes << "\n";
  cout << "Draw times         : " << gameInfo.drawTimes << "\n\n\n\n";
}

// =================================================================================
//  Game Flow
// =================================================================================

stGameInfo playGame() {
  stGameInfo gameInfo;
  gameInfo.numberOfRounds = input::readNumber<int>("How many rounds you want to play: ");
  gameInfo.roundsInfo.resize(gameInfo.numberOfRounds);

  for (int i = 0; i < gameInfo.numberOfRounds; i++) {
    gameInfo.roundsInfo[i].roundNumber = i + 1;
    gameInfo.roundsInfo[i].playerChoice = static_cast<enChoice>(input::readNumber<int>(
        "What is your choice \n[1] : Rock, \n[2] : Paper, \n[3] : Scissor "
        "\nYour Choice: ", 1, 3));
    gameInfo.roundsInfo[i].computerChoice = static_cast<enChoice>(process::randInt(1, 3));
    gameInfo.roundsInfo[i].RoundWinner = getWinner(gameInfo.roundsInfo[i].playerChoice,
                                                   gameInfo.roundsInfo[i].computerChoice);
    gameInfo.roundsInfo[i].winnerName = getWinnerName(gameInfo.roundsInfo[i].RoundWinner);

    if (gameInfo.roundsInfo[i].RoundWinner == enWinner::Player) {
      gameInfo.playerWinTimes++;
      system("color 0A");
    } else if (gameInfo.roundsInfo[i].RoundWinner == enWinner::Computer) {
      gameInfo.computerWinTimes++;
      system("color 0C");
    } else {
      gameInfo.drawTimes++;
      system("color 0E");
    }

    printRoundResults(gameInfo.roundsInfo[i].roundNumber,
                      gameInfo.roundsInfo[i].playerChoice,
                      gameInfo.roundsInfo[i].computerChoice,
                      gameInfo.roundsInfo[i].winnerName);
  }

  gameInfo.gameWinner = getFinalGameWinner(gameInfo.playerWinTimes, gameInfo.computerWinTimes);
  gameInfo.winnerName = getWinnerName(gameInfo.gameWinner);

  cin.get();
  return gameInfo;
}

void startGame() {
  bool playAgain = true;
  do {
    output::resetScreen();
    printGameResults(playGame());
    playAgain = input::readBool("Do you want to play again?\n");
  } while (playAgain);
}

// =================================================================================
//  Main
// =================================================================================

int main() {
  srand(time(NULL));
  startGame();
  return 0;
}