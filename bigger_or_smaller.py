# A simple user interface for a drinking game "Bigger or Smaller"
# Jasper Kosonen 29.6.2020

# RULES:

# A drinking game where on each round a random number will be generated.
# On their turn, each player has to guess whether the next number will
# be greater or smaller than the previous one. If the player guesses wrong,
# they need to take a sip. The player in turn changes after every guess.

import random
import time

UPPER_BOUND = 100  # A constant for the number range


# Checks if the player's guess was correct.
# Takes an erroneous input into consideration.
# crrnt = the current number
# nxt = the next number
# guess = the player's guess
def check(crrnt, nxt, guess):
    # Trivial part
    if guess == "+":
        return nxt > crrnt
    elif guess == "-":
        return nxt < crrnt
    elif guess == "q" or guess == "Q":
        return "q"

    # Recursive part
    else:
        guess = input("Try again. [+/-] ")
        return check(crrnt, nxt, guess)


def play_game(players):
    rounds = 1
    in_turn = 0  # Indicates who is in turn
    crrnt = random.randint(0, UPPER_BOUND)  # The current number
    nxt = random.randint(0, UPPER_BOUND)  # The next number
    # The next number must not be the same as the previous one.
    while nxt == crrnt:
        nxt = random.randint(0, UPPER_BOUND)

    # The game goes on until stopped.
    while True:
        print("Round {:d}:".format(rounds))
        print()

        player_in_turn = players[in_turn % len(players)]

        for i in range(len(players)):
            prompt = input(
                "The current number is {:d}. Will the next one be bigger or "
                "smaller? [+/-] ".format(crrnt))
            print()

            check_value = check(crrnt, nxt, prompt)
            if check_value == "q":
                return
            elif check_value:
                print("The next number is {:d}.".format(nxt))
                print("Nice guess, {:s}!"
                      .format(player_in_turn))
                print()
            else:
                print("The next number is {:d}.".format(nxt))
                print("Too bad. Now take a sip, {:s}!"
                      .format(player_in_turn))
                print()

            # Updating the game for the next guess
            crrnt = nxt
            nxt = random.randint(0, UPPER_BOUND)
            while nxt == crrnt:
                nxt = random.randint(0, UPPER_BOUND)
            in_turn += 1
            player_in_turn = players[in_turn % len(players)]
            input("It is now your turn, {:s}. Continue with ENTER."
                  .format(player_in_turn))
            print()

        rounds += 1


def main():
    print("Welcome to play 'Bigger or smaller'!")
    print("Guess whether the next number will be bigger (+) or not (-).")
    print("The numbers will be between 0 and {:d}.".format(UPPER_BOUND))
    print()
    prompt = input("Press ENTER to start. Press q to quit. ")
    print()

    if prompt == "q" or prompt == "Q":
        print("Good bye!")
        return

    # Asking for player info
    players = []
    prompt = input("How many players are playing? ")
    print()

    while True:
        try:
            prompt = int(prompt)
            if prompt > 0:
                for i in range(prompt):
                    name = input("Give player {:d}'s name: ".format(i+1))
                    players.append(name)
                print()
                break

            prompt = input(
                "Give the number of players as a positive whole number. ")
            print()

        except ValueError:
            prompt = input(
                "Give the number of players as a positive whole number. ")
            print()

    print("Welcome", end="")
    for i in range(len(players)-1):
        print(", " + players[i], end="")
    print(" and {:s}!".format(players[-1]))
    print()

    print("Let's go! {:s} starts.".format(players[0]))
    print()

    # The actual game starts here.
    play_game(players)

    print("Good bye!")


main()
