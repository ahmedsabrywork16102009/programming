import random

#=======================
# Drawing rock, paper, scissors
#=======================
rockDrawing = """
    _______
---'   ____)
      (_____)
      (_____)
      (____)
---.__(___)
"""

paperDrawing = """
    _______
---'   ____)____
          ______)
          _______)
         _______)
---.__________)
"""

scissorsDrawing = """
    _______
---'   ____)____
          ______)
       __________)
      (____)
---.__(___)
"""

gameChoices = ["Rock","Paper","Scissor"]
print("Welcome to the Rock, Paper, Scissor game:\n")
isNeedHelp = input("Press Enter to continue or type (Help) for the rules help: ").lower()

if isNeedHelp == "help":
    print("\n\t\t******** RULES *********")
    print("\t\t1) You choose and the computer chooses")
    print("\t\t2) Rock smashes Scissors -> Rock wins")
    print("\t\t3) Scissors cut Paper -> Scissors win")
    print("\t\t4) Paper covers Rock -> Paper wins\n\n")

Playerchoice = input("Enter your choice (Rock, Paper, Scissor): ").capitalize()

computerChoice = random.choice(gameChoices)



playerWinTimes = 0
computerWinTimes = 0
drawTimes = 0

if Playerchoice in gameChoices:
    print("You choice: ")
    print(rockDrawing if Playerchoice == gameChoices[0] else paperDrawing if Playerchoice == gameChoices[1] else scissorsDrawing)

    print("Computer choice: ")
    print(rockDrawing if computerChoice == gameChoices[0] else paperDrawing if computerChoice == gameChoices[1] else scissorsDrawing)

    if(((Playerchoice == gameChoices[0]) and (computerChoice == gameChoices[2])) or ((Playerchoice == gameChoices[1]) and (computerChoice == gameChoices[0])) or ((Playerchoice == gameChoices[2]) and (computerChoice == gameChoices[1])) ):
        playerWinTimes += 1
        print(f"You win! {Playerchoice} beats {computerChoice}")
    elif Playerchoice == computerChoice:
        drawTimes += 1
        print("Draw!")
    else:
        computerWinTimes += 1
        print(f"You lose! {computerChoice} beats {Playerchoice}")
else:
    print("Invalid choice. Please run the program again and choose rock, paper, or scissors.")


print(f"\nPlayer wins: {playerWinTimes}")
print(f"Computer wins: {computerWinTimes}")
print(f"Draws: {drawTimes}")

score = computerWinTimes - playerWinTimes


print(f"Final score: {score}\n")
if score > 0:
    print("You are the best!")
elif score < 0:
    print("Computer is the best!")
else:
    print("Draw!")


print("\nThanks for playing!")