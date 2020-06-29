# A simple user interface for a drinking game "Bigger or Smaller"
# Jasper Kosonen 29.6.2020

# RULES:

# A drinking game where on each round a random number will be generated.
# On their turn, each player has to guess whether the next number will
# be greater or smaller than the previous one. If the player guesses wrong,
# they need to take a sip. The player in turn changes after every guess.


UPPER_BOUND = 100  # A constant for the number range


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


main()

