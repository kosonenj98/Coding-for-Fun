# A simple user interface for a drinking game "Bigger or Smaller"
# Jasper Kosonen 29.6.2020

# RULES:

# A drinking game where on each round a random number will be generated.
# On their turn, each player has to guess whether the next number will
# be greater or smaller than the previous one. If the player guesses wrong,
# they need to take a sip. The player in turn changes after every guess.

import random

UPPER_BOUND = 100  # A constant for the number range


# Checks if the player's guess was correct.
# Takes an erroneous input into consideration.
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
            
    print("Let's go!")
    print()

    rounds = 1
    crrnt = random.randint(0, UPPER_BOUND)  # The current number
    nxt = random.randint(0, UPPER_BOUND)  # The next number
    # The next number must not be the same as the previous one.
    while nxt == crrnt:
        nxt = random.randint(0, UPPER_BOUND)

    print("Round {:d}:".format(rounds))
    print()
    prompt = input(
        "The first number is {:d}. Will the next one be bigger or smaller? "
        "[+/-] ".format(crrnt))
    print()

    # The game goes on until stopped.
    while prompt != "q" and prompt != "Q":
        check_value = check(crrnt, nxt, prompt)
        if check_value == "q":
            print("Good bye!")
            return
        elif check_value:
            print("The next number is {:d}.".format(nxt))
            input("Nice! ")
            print()
        else:
            print("The next number is {:d}.".format(nxt))
            input("Too bad. Now take a sip! ")
            print()

        # Updating the game for the next round
        rounds += 1
        crrnt = nxt
        nxt = random.randint(0, UPPER_BOUND)
        while nxt == crrnt:
            nxt = random.randint(0, UPPER_BOUND)

        print("Round {:d}:".format(rounds))
        print()
        prompt = input(
            "The current number is {:d}. Will the next be bigger or smaller? "
            "[+/-] ".format(crrnt))

    print("Good bye!")


main()
