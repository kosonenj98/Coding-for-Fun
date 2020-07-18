#ifndef BOARD_HH
#define BOARD_HH

#include "snake.hh"
#include <vector>
#include <random>


struct Tile
{
    bool is_occupied;
    bool is_head;
    bool has_food;
    bool has_gold;
};


struct Gold
{
    int duration;
    int time;
    int x;
    int y;
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

    bool board_full();

    void add_food();

    void set_closed_borders(bool closed);

    void set_gold_frequency(int value);

    void set_gold_duration(int value);

    std::vector< std::vector< Tile > > get_board();

    Snake* get_snake();

    void get_consumption(int& food, int& gold);

private:
    int modulo_(int a, int b);

    int columns_;
    int rows_;

    std::vector< std::vector< Tile > > board_;
    Snake* snake_;

    bool closed_borders_ = false;

    int food_eaten_;
    int gold_eaten_;

    std::default_random_engine generator_{(static_cast<unsigned int>(time(NULL)))};

    int gold_frequency_;
    int gold_duration_;
    std::vector<Gold> gold_;
};

#endif // BOARD_HH
