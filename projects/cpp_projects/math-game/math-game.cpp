#include "../../../libraries/enumeration.h"
#include "../../../libraries/input.h"
#include "../../../libraries/process.h"

#include <iostream>
#include <string>
#include <string_view>
#include <cstdlib>

using namespace std;
using namespace AS;

// =================================================================================
//  Math Game Enums
// =================================================================================

enum enQuestionsLevel { Easy = 1, Medium = 2, Hard = 3, MixL = 4 };

// =================================================================================
//  Math Game Logic
// =================================================================================

int calcSum(int a, int b)      { return a + b; }
int calcSubtract(int a, int b) { return a - b; }
int calcMultiply(int a, int b) { return a * b; }
int calcDivide(int a, int b)   { return (b == 0) ? 0 : a / b; }

int simpleCalculator(int number1, int number2, enOperationType operation) {
  switch (operation) {
    case enOperationType::sum:      return calcSum(number1, number2);
    case enOperationType::subtract: return calcSubtract(number1, number2);
    case enOperationType::multiply: return calcMultiply(number1, number2);
    case enOperationType::divide:   return calcDivide(number1, number2);
    case enOperationType::mixOp:    return simpleCalculator(number1, number2, (enOperationType)getRandomNumber(1, 4));
    default: return 0;
  }
}

string getOperationSymbol(enOperationType operationType) {
  switch (operationType) {
    case enOperationType::sum:      return "+";
    case enOperationType::subtract: return "-";
    case enOperationType::multiply: return "*";
    case enOperationType::divide:   return "/";
    default: return "";
  }
}

void resetConsole() {
  system("cls");
  system("color 0F");
}

// =================================================================================
//  Quiz Structs
// =================================================================================

struct stQuestion {
  int number1 = 0;
  int number2 = 0;
  enQuestionsLevel questionLevel;
  enOperationType operationType;
  int correctAnswer = 0;
  int userAnswer = 0;
  bool isCorrect = false;
};

struct stQuizz {
  stQuestion questionsList[100];
  short numberOfQuestions = 0;
  enQuestionsLevel questionsLevel;
  enOperationType operationType;
  int numberOfCorrectAnswers = 0;
  int numberOfWrongAnswers = 0;
  bool isPass = false;
};

// =================================================================================
//  Quiz Setup
// =================================================================================

bool doYouWantToPlay() {
  return readBool("Do you want to play the math game? 1. Yes, 0. No: ");
}

int readNumberOfQuestions() {
  return readNumber<int>("How many questions do you want to answer? (1-100): ", 1, 100);
}

enQuestionsLevel readQuestionsLevel() {
  return (enQuestionsLevel)readNumber<int>("Choose the level of questions: 1. Easy, 2. Medium, 3. Hard, 4. Mix: ", 1, 4);
}

enOperationType readOperationType() {
  return (enOperationType)readNumber<int>("Choose the operation type: 1. Add, 2. Subtract, 3. Multiply, 4. Divide, 5. Mix: ", 1, 5);
}

void generateNumbersWithLevel(int &number1, int &number2, enQuestionsLevel questionLevel) {
  switch (questionLevel) {
    case Easy:   number1 = getRandomNumber(1, 10);  number2 = getRandomNumber(1, 10);  break;
    case Medium: number1 = getRandomNumber(10, 50); number2 = getRandomNumber(10, 50); break;
    case Hard:   number1 = getRandomNumber(50, 100); number2 = getRandomNumber(50, 100); break;
    case MixL:   generateNumbersWithLevel(number1, number2, (enQuestionsLevel)getRandomNumber(1, 3)); break;
  }
}

stQuestion generateQuestion(stQuizz &quizz) {
  stQuestion question;
  question.questionLevel = (quizz.questionsLevel == MixL) ? (enQuestionsLevel)getRandomNumber(1, 3) : quizz.questionsLevel;
  question.operationType = (quizz.operationType == enOperationType::mixOp) ? (enOperationType)getRandomNumber(1, 4) : quizz.operationType;
  generateNumbersWithLevel(question.number1, question.number2, question.questionLevel);
  question.correctAnswer = simpleCalculator(question.number1, question.number2, question.operationType);
  return question;
}

void generateQuestions(stQuizz &quizz) {
  for (int i = 0; i < quizz.numberOfQuestions; i++)
    quizz.questionsList[i] = generateQuestion(quizz);
}

// =================================================================================
//  Quiz Display & Grading
// =================================================================================

void printQuestion(stQuizz quizz, int questionNumber) {
  cout << "\nQuestion " << questionNumber + 1 << "/" << quizz.numberOfQuestions << ":\n";
  cout << quizz.questionsList[questionNumber].number1 << " "
       << getOperationSymbol(quizz.questionsList[questionNumber].operationType) << " "
       << quizz.questionsList[questionNumber].number2 << " = ";
}

void changeScreenColor(bool isCorrect) {
  system(isCorrect ? "color 2F" : "color 4F");
}

void askAndCorrectQuestionListAnswers(stQuizz &quizz) {
  for (int i = 0; i < quizz.numberOfQuestions; i++) {
    resetConsole();
    printQuestion(quizz, i);
    quizz.questionsList[i].userAnswer = readNumber<int>("\nYour answer: ");
    if (quizz.questionsList[i].userAnswer == quizz.questionsList[i].correctAnswer) {
      quizz.questionsList[i].isCorrect = true;
      quizz.numberOfCorrectAnswers++;
    } else {
      quizz.questionsList[i].isCorrect = false;
      quizz.numberOfWrongAnswers++;
    }
    changeScreenColor(quizz.questionsList[i].isCorrect);
  }
  quizz.isPass = quizz.numberOfCorrectAnswers >= quizz.numberOfWrongAnswers;
}

string getFinalGameText(bool isPass) { return isPass ? "Pass :-)" : "Fail :-("; }

string getQuestionLevelText(enQuestionsLevel level) {
  string arr[] = { "Easy", "Medium", "Hard", "Mix" };
  return arr[level - 1];
}

void printQuizzResult(stQuizz quizz) {
  cout << "\n==========================================\n\n";
  cout << "Final Result: " << getFinalGameText(quizz.isPass) << "\n";
  cout << "==========================================\n";
  cout << "Total Questions: " << quizz.numberOfQuestions << "\n";
  cout << "Questions Level: " << getQuestionLevelText(quizz.questionsLevel) << "\n";
  cout << "Operation Type: " << getOperationSymbol(quizz.operationType) << "\n";
  cout << "Correct Answers: " << quizz.numberOfCorrectAnswers << "\n";
  cout << "Wrong Answers: " << quizz.numberOfWrongAnswers << "\n";
  cout << "==========================================\n\n";
}

void startQuizz() {
  stQuizz quizz;
  quizz.numberOfQuestions = readNumberOfQuestions();
  quizz.questionsLevel = readQuestionsLevel();
  quizz.operationType = readOperationType();
  generateQuestions(quizz);
  askAndCorrectQuestionListAnswers(quizz);
  printQuizzResult(quizz);
}

void startGame() {
  bool playAgain = true;
  do {
    resetConsole();
    startQuizz();
    playAgain = doYouWantToPlay();
    resetConsole();
  } while (playAgain);
}

// =================================================================================
//  Main
// =================================================================================

int main() {
  srand((unsigned int)(time(NULL)));
  startGame();
  return 0;
}