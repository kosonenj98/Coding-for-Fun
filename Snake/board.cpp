#include "board.hh"
#include <QDebug>
#include <QString>

Board::Board(int columns, int rows)
{
    columns_ = columns;
    rows_ = rows;

    // Initializing the board
    for (int y = 0; y < rows_; y++)
    {
        std::vector <Tile> tmp;
        for (int x = 0; x < columns_; x++)
        {
            Tile tile;
            tile.is_occupied = false;
            tile.is_head = false;
            tile.has_food = false;
            tile.has_gold = false;
            tmp.push_back(tile);
        }
        board_.push_back(tmp);
    }

    food_eaten_ = 0;
    gold_eaten_ = 0;

    gold_frequency_ = 10;
    gold_duration_ = 20;

    // Distributions for generating random numbers
    distr_x_ = std::uniform_int_distribution<int>(0, columns_ - 1);
    distr_y_ = std::uniform_int_distribution<int>(0, rows_ - 1);
    distr_gold_ = std::uniform_int_distribution<int>(0, gold_frequency_ - 1);

    // The snake starts at the center of the board.
    int start_x, start_y;
    start_x = columns_ / 2 - 1;
    start_y = rows_ / 2 - 1;

    // Creating the snake, generating food
    // and updating the board
    snake_ = new Snake(start_x, start_y);
    board_.at(start_y).at(start_x).is_occupied = true;
    board_.at(start_y).at(start_x).is_head = true;

    add_food();
    update_board();
}

Board::~Board()
{
    delete snake_;
}


void Board::reset_board()
{
    for (int y = 0; y < rows_; y++)
    {
        for (int x = 0; x < columns_; x++)
        {
            board_.at(y).at(x).is_occupied = false;
            board_.at(y).at(x).is_head = false;
        }
    }
}


void Board::update_board()
{
    reset_board();

    Piece* tmp;
    tmp = snake_->get_tail();


    // Setting all the game tiles with snake to occupied
    if (tmp->previous == nullptr)
    {
        board_.at(tmp->y % rows_).at(tmp->x % columns_)
                .is_occupied = true;
        board_.at(tmp->y % rows_).at(tmp->x % columns_)
                .is_head = true;
    }
    else
    {
        while (tmp->previous != nullptr)
        {
            board_.at(tmp->y).at(tmp->x).is_occupied = true;
            tmp = tmp->previous;
        }
        board_.at(tmp->y % rows_).at(tmp->x % columns_)
                .is_occupied = true;
        board_.at(tmp->y % rows_).at(tmp->x % columns_)
                .is_head = true;
    }
}


bool Board::move_snake()
{
    if (!gold_.empty())
    {
        // Gold exists on the board

        std::vector<Gold>::iterator iter;
        iter = gold_.begin();

        while (iter != gold_.end())
        {
            // The gold exists only for certain
            // amount of moves.
            iter->time_remaining--;

            if (iter->time_remaining == 0)
            {
                board_.at(iter->y).at(iter->x)
                        .has_food = false;

                board_.at(iter->y).at(iter->x)
                        .has_gold = false;

                gold_.erase(gold_.begin());

                iter--;
            }
            iter++;
        }
    }

    // The tail moves out of the way.
    board_.at(snake_->get_tail()->y)
            .at(snake_->get_tail()->x)
            .is_occupied = false;

    snake_->move();

    // Checking if the move is valid
    if (!check())
    {
        return false;
    }

    return true;
}


bool Board::check()
{
    Piece* tmp_head;
    tmp_head = snake_->get_head();

    // With closed borders, the snake cannot cross them.
    if (closed_borders_)
    {
        if (tmp_head->x < 0 or tmp_head->x > columns_ - 1 or
                tmp_head->y < 0 or tmp_head->y > rows_ - 1)
        {
            return false;
        }
    }

    // Shifting the x and y periodically
    snake_->get_head()->x = modulo_(tmp_head->x, columns_);
    snake_->get_head()->y = modulo_(tmp_head->y, rows_);

    // The tile where head is currently
    Tile current = board_.at(snake_->get_head()->y)
                         .at(snake_->get_head()->x);

    // Cannot move, game ends
    if (current.is_occupied)
    {
        return false;
    }

    update_board();

    // Snake eats the food on the tile
    if (current.has_food)
    {
        food_eaten_++;

        board_.at(snake_->get_head()->y)
                .at(snake_->get_head()->x)
                .has_food = false;

        if (current.has_gold)
        {
            gold_eaten_++;

            board_.at(snake_->get_head()->y)
                    .at(snake_->get_head()->x)
                    .has_gold = false;
        }
        else
        {
            add_food();
        }

        // The snake grows after eating.
        snake_->append();
    }

    return true;
}


bool Board::board_full()
{
    for (int y = 0; y < rows_; y++)
    {
        for (int x = 0; x < columns_; x++)
        {
            if (!(board_.at(y).at(x).is_occupied or
                    board_.at(y).at(x).has_food))
            {
                return false;
            }
        }
    }
    return true;
}


void Board::add_food()
{
    // Cannot add food to full board
    if (board_full())
    {
        return;
    }

    // First generated number is not so random.
    distr_x_(generator_);
    distr_y_(generator_);
    distr_gold_(generator_);
    int x, y;

    // Generating random coordinates for food
    x = distr_x_(generator_);
    y = distr_y_(generator_);
    while (board_.at(y).at(x).is_occupied or
           board_.at(y).at(x).has_food)
    {
        x = distr_x_(generator_);
        y = distr_y_(generator_);
    }

    board_.at(y).at(x).has_food = true;

    // Possibly generating the gold food
    if (distr_gold_(generator_) % gold_frequency_ == 0 and !board_full())
    {
        Gold tmp;
        tmp.time_remaining = gold_duration_;

        x = distr_x_(generator_);
        y = distr_y_(generator_);
        while (board_.at(y).at(x).is_occupied or
               board_.at(y).at(x).has_food)
        {
            x = distr_x_(generator_);
            y = distr_y_(generator_);
        }

        board_.at(y).at(x).has_food = true;
        board_.at(y).at(x).has_gold = true;

        tmp.x = x;
        tmp.y = y;

        gold_.push_back(tmp);
    }
}


void Board::set_closed_borders(bool closed)
{
    closed_borders_ = closed;
}


void Board::set_gold_frequency(int value)
{
    gold_frequency_ = value;
}


void Board::set_gold_duration(int value)
{
    gold_duration_ = value;
}


std::vector<std::vector<Tile> > *Board::get_board()
{
    return &board_;
}


Snake *Board::get_snake()
{
    return snake_;
}


void Board::get_consumption(int &food, int &gold)
{
    food = food_eaten_;
    gold = gold_eaten_;
}


int Board::modulo_(int a, int b)
{
    int r = a % b;

    while (r < 0)
    {
        r += b;
    }

    return r;
}
