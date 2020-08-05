/* This module contains the Board class.
 *
 * Board class operates the game board.
 * It consists of a grid the nodes of which
 * are logical values described in struct
 * 'Tile'.
 *
 * Board moves the snake and checks if
 * the snake's move is valid or not.
 * Board also generated food and occasionally
 * gold, too.
 *
 * */


#ifndef BOARD_HH
#define BOARD_HH

#include "snake.hh"
#include <vector>
#include <random>


// A struct for game board node
struct Tile
{
    bool is_occupied;   // true if contains snake
    bool is_head;       // true if contains head
    bool has_food;      // true if contains food
    bool has_gold;      // true if contains gold
};

// A struct for generated gold food
struct Gold
{
    int time_remaining;   // a clock for the gold
    int x;  // x coordinate
    int y;  // y coordinate
};


class Board
{
public:

    /* Creates board with dimensions
     * columns x rows
     * */
    Board(int columns, int rows);

    ~Board();

    // Sets all Tiles unoccupied and headless
    void reset_board();

    // Assigns correct bool values to Tiles
    void update_board();

    /* Tries to move the snake
     *
     * Returns true if succeeds. Otherwise false.
     * */
    bool move_snake();

    // Checks if the snake's move is valid
    bool check();

    /* Checks if the board is full
     *
     * Returns true if full. Otherwise false.
     * */
    bool board_full();

    /* Generates food to random location on board
     *
     * Sometimes generates gold food
     * */
    void add_food();

    /* For toggling the gameplay setting
     *
     * If true, snake cannot cross borders.
     * Otherwise, snake emerges from the other side.
     * */
    void set_closed_borders(bool closed);

    /* Affects how often gold food appears.
     *
     * Gold is generated at the probability of
     * 1/value everytime a food is generated
     * */
    void set_gold_frequency(int value);

    // Determines for how long gold exists
    void set_gold_duration(int value);

    // Returns a pointer to board
    std::vector< std::vector< Tile > >* get_board();

    // Returns a pointer to snake
    Snake* get_snake();

    /* Assigns values for food and gold that
     * correspond to the consumption of snake
     * */
    void get_consumption(int& food, int& gold);

private:

    // Returns the positive remainder of a / b.
    int modulo_(int a, int b);

    int columns_;   // # of columns in board
    int rows_;      // # of rows in board

    std::vector< std::vector< Tile > > board_;
    Snake* snake_;

    bool closed_borders_ = false;

    // For keeping tally of consumption
    int food_eaten_;
    int gold_eaten_;

    /* For creating random integers for food
     *
     * Takes time(NULL) as seed for unique
     * gameplay
     * */
    std::default_random_engine
    generator_{(static_cast<unsigned int>(time(NULL)))};
    std::uniform_int_distribution<int> distr_x_;
    std::uniform_int_distribution<int> distr_y_;
    std::uniform_int_distribution<int> distr_gold_;


    // Attributes for managing gold
    int gold_frequency_;
    int gold_duration_;

    // Multiple coexisting golds is possible
    std::vector<Gold> gold_;
};

#endif // BOARD_HH
