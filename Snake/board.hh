#ifndef BOARD_HH
#define BOARD_HH

#include <iostream>
#include <vector>
#include <random>
#include "snake.hh"

struct Tile
{
    bool is_occupied;
    bool has_food;
};


class Board
{
public:
    Board(int columns, int rows);

    ~Board();

    void reset_board();

    void update_board();

    bool move_snake();

    bool check();

    int modulo(int a, int b);

    void add_food();

    std::vector< std::vector< Tile > > get_board();

    Snake* get_snake();

    void set_closed_borders(bool closed);

private:
    int columns_;
    int rows_;
    std::vector< std::vector< Tile > > board_;
    Snake* snake_;
    bool closed_borders_ = false;
    std::default_random_engine generator_{(static_cast<unsigned int>(time(NULL)))};
};

#endif // BOARD_HH
