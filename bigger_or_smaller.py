# A simple user interface for a drinking game "Bigger or Smaller"
# Jasper Kosonen 29.6.2020

# RULES:

# A drinking game where on each round a random number will be generated.
# On their turn, each player has to guess whether the next number will
# be greater or smaller than the previous one. If the player guesses wrong,
# they need to take a sip. If the player guesses correctly, their sip gets
# added to a "queue" that the next player who guesses wrong has to drink.
# The player in turn changes after every guess.

import random
import numpy as np
import matplotlib.pyplot as plt

UPPER_BOUND = 100  # A constant for the number range
INDENT = "  "  # An indent to make the UI more visual


def print_instructions():
    """
    Explains the rules of the game.
    :return: none
    """
    print(INDENT + "INSTRUCTIONS")
    print()
    print(2 * INDENT + "This is a drinking game for 1-n players.")
    print(2 * INDENT + "The game generates random numbers between 0 and {:d}."
          .format(UPPER_BOUND))
    print(2 * INDENT + "On each round, every player gets to guess whether the")
    print(2 * INDENT + "next generated number will be bigger (+) than the "
                       "previous")
    print(2 * INDENT + "one or not (-).")
    print()
    print(2 * INDENT + "If the player guesses incorrectly, they have to take ")
    print(2 * INDENT + "a sip of their drink. Otherwise, their sip gets added")
    print(2 * INDENT + "to a queue of sips. The next person who fails has to")
    print(2 * INDENT + "drink all the sips from the queue including theirs.")
    print()
    print(INDENT + "Have fun!")
    print()


def draw_sips(players):
    """
    Draws each player's sips in a cumulative graph.
    :param players: the list of players with their names and sips
    :return: none
    """
    biggest_sip_total = 0  # Sets the maximum for the y axis.

    plt.figure()

    # Calculating the cumulative sips on each round for each player
    for i in range(len(players) - 1):
        cumulative_sips = []
        for j in range(len(players[i][-1])):
            sum = 0
            k = 0
            while k <= j:
                sum += players[i][-1][k]
                k += 1
            cumulative_sips.append(sum)

        if cumulative_sips and cumulative_sips[-1] > biggest_sip_total:
            biggest_sip_total = cumulative_sips[-1]

        # Drawing the marker and line with the same color
        ax = plt.gca()
        color = next(ax._get_lines.prop_cycler)['color']
        plt.plot(cumulative_sips, label=players[i][0], color=color)
        plt.plot(cumulative_sips, 'o', color=color)

    plt.xlabel('Round number')
    plt.ylabel('Total sips')
    plt.title('Results')
    plt.xticks(np.arange(len(players[0][-1])),
               np.arange(1, len(players[0][-1]) + 1, 1))
    plt.yticks(np.arange(0, biggest_sip_total + 1, 1))
    plt.legend()
    plt.show()


def check(crrnt, nxt, guess):
    """
    Checks if the player's guess was correct.
    Takes an erroneous input into consideration.
    :param crrnt: the current number
    :param nxt: the next number
    :param guess: the player's guess
    :returns True if the guess is correct. False if the guess is incorrect.
    :returns "q" if the player wants to quit the game.
    """
    # Trivial part
    if guess == "+":
        print()
        return nxt > crrnt
    elif guess == "-":
        print()
        return nxt < crrnt
    elif guess == "q" or guess == "Q":
        print()
        return "q"

    # Recursive part
    else:
        guess = input(2 * INDENT + "Try again. [+/-] ")
        return check(crrnt, nxt, guess)


def play_game(players):
    """
    The actual game engine. Takes care of game play and player data.
    The data is collected on every round in the form of sips.
    :param players: the list of all the players and their sips
    :return: the updated version of the player list with the longest streak
             at the very end
    """
    rounds = 1
    in_turn = 0  # Indicates who is in turn
    streak = 0  # The ongoing streak of correct guesses.
    longest_streak = 0
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

        i = 0
        while i < len(players):
            prompt = input(INDENT + "The current number is {:d}. Is the next "
                                    "one be bigger or smaller? [+/-] "
                           .format(crrnt))

            check_value = check(crrnt, nxt, prompt)
            if check_value == "q":
                if streak > longest_streak:
                    longest_streak = streak
                players.append(longest_streak)
                return players
            elif check_value:
                streak += 1
                player_in_turn[1].append(0)
                print(INDENT + "The next number is {:d}.".format(nxt))
                print(INDENT + "Nice guess, {:s}!"
                      .format(player_in_turn[0]))
                print(2 * INDENT + "Current streak: {:d}".format(streak))
                print()
            else:
                # A wrong guess breaks the streak.
                # The player has to drink as many sips as the streak was.
                if streak > longest_streak:
                    longest_streak = streak
                print(INDENT + "The next number is {:d}.".format(nxt))
                if streak < 2:
                    player_in_turn[1].append(1)
                    print(INDENT + "Too bad. Now take a sip, {:s}!"
                          .format(player_in_turn[0]))
                else:
                    player_in_turn[1].append(streak)
                    print(INDENT + "Too bad. Now take {:d} sips, {:s}!"
                          .format(streak, player_in_turn[0]))

                streak = 0
                print()

            # Updating the game for the next guess
            crrnt = nxt
            nxt = random.randint(0, UPPER_BOUND)
            while nxt == crrnt:
                nxt = random.randint(0, UPPER_BOUND)
            in_turn += 1
            player_in_turn = players[in_turn % len(players)]
            input(INDENT + "It's now your turn, {:s}. Continue with ENTER. "
                  .format(player_in_turn[0]))
            print()
            i += 1

        rounds += 1


def main():
    print()
    print("Welcome to play 'Bigger or smaller'!")
    print()
    prompt = input("Press ENTER to start. Press q to quit. Press i for "
                   "instructions.")
    print()

    if prompt == "q" or prompt == "Q":
        print("Good bye!")
        return
    elif prompt == "i" or prompt == "I":
        print_instructions()

    # Asking for player info
    players = []
    prompt = input("How many players are playing? ")

    if prompt == "q" or prompt == "Q":
        print()
        print("Good bye!")
        return

    while True:
        try:
            prompt = int(prompt)
            if prompt > 0:
                print()
                for i in range(prompt):
                    name = input(INDENT + "Give player {:d}'s name: "
                                 .format(i+1))
                    while not name.strip():
                        name = input(2 * INDENT + "You must give a name: ")

                    if name == "q" or name == "Q":
                        print()
                        print("Good bye!")
                        return

                    sips = []  # Player's personal sips on each round
                    tmp_list = [name.strip(), sips]
                    players.append(tmp_list)
                print()
                break

            prompt = input(INDENT + "Give the number of players as a positive "
                                    "whole number. ")

        except ValueError:
            prompt = input(INDENT + "Give the number of players as a positive "
                                    "whole number. ")

    print("Welcome", end="")
    for i in range(len(players) - 1):
        print(", " + players[i][0], end="")
    print(" and {:s}!".format(players[-1][0]))
    print()

    print("Let's go! {:s} starts.".format(players[0][0]))
    print()

    # The actual game starts here.
    players = play_game(players)

    # Possibility to view results before ending the game
    prompt = input("Thank you for playing! Press 'R' for results or quit with "
                   "ENTER. ")
    print()

    if prompt == "R" or prompt == "r":
        if players[0][-1]:
            print("The results open up in a new window.")
            print("Longest streak: {:d}".format(players[-1]))
            print()

            # Drawing the player data in a visual form
            draw_sips(players)

        else:
            print("No results to be shown...")
            print()

    print("Remember to drink water!")
    print("Good bye!")


main()
