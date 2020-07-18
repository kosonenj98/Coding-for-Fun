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

    int start_x, start_y;
    start_x = columns_ / 2;
    start_y = rows_ / 2;

    snake_ = new Snake(start_x, start_y);
    board_.at(start_y).at(start_x).is_occupied = true;
    board_.at(start_y).at(start_x).is_head = true;

    food_eaten_ = 0;
    gold_eaten_ = 0;

    gold_frequency_ = 10;
    gold_duration_ = 20;

    add_food();
    update_board();
}

Board::~Board()
{
    snake_->~Snake();
    snake_ = nullptr;
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
        std::vector<Gold>::iterator iter;
        iter = gold_.begin();

        while (iter != gold_.end())
        {
            iter->time++;
            if (iter->time > iter->duration)
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

    if (check())
    {
        update_board();
        return true;
    }

    return false;
}


bool Board::check()
{
    Piece* tmp_head;
    tmp_head = snake_->get_head();

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

    Tile current = board_.at(snake_->get_head()->y)
                         .at(snake_->get_head()->x);

    if (current.is_occupied)
    {
        return false;
    }

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
    if (board_full())
    {
        return;
    }

    std::uniform_int_distribution<int> distr_x(0, columns_ - 1);
    std::uniform_int_distribution<int> distr_y(0, rows_ - 1);
    std::uniform_int_distribution<int> distr_golden(0, gold_frequency_ - 1);
    distr_x(generator_);
    distr_y(generator_);
    distr_golden(generator_);
    int x, y;

    x = distr_x(generator_);
    y = distr_y(generator_);
    while (board_.at(y).at(x).is_occupied or
           board_.at(y).at(x).has_food)
    {
        x = distr_x(generator_);
        y = distr_y(generator_);
    }

    board_.at(y).at(x).has_food = true;

    // Possibly generating the golden food
    if (distr_golden(generator_) % gold_frequency_ == 0 and !board_full())
    {
        Gold tmp;
        tmp.duration = gold_duration_;
        tmp.time = 0;

        x = distr_x(generator_);
        y = distr_y(generator_);
        while (board_.at(y).at(x).is_occupied or
               board_.at(y).at(x).has_food)
        {
            x = distr_x(generator_);
            y = distr_y(generator_);
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

std::vector<std::vector<Tile> > Board::get_board()
{
    return board_;
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
