#include "board.hh"


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
            tile.has_food = false;
            tmp.push_back(tile);
        }
        board_.push_back(tmp);
    }

    int start_x, start_y;
    start_x = columns_ / 2;
    start_y = rows_ / 2;

    snake_ = new Snake(start_x, start_y);
    board_.at(start_y).at(start_x).is_occupied = true;

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
        }
    }
}


void Board::update_board()
{
    reset_board();

    Piece* tmp;
    tmp = snake_->get_head();


    // Setting all the game tiles with snake to occupied
    if (tmp->next == nullptr)
    {
        board_.at(tmp->y % rows_).at(tmp->x % columns_).is_occupied = true;
    }
    else
    {
        while (tmp->next != nullptr)
        {
            board_.at(tmp->y).at(tmp->x).is_occupied = true;
            tmp = tmp->next;
        }
        board_.at(tmp->y % rows_).at(tmp->x % columns_).is_occupied = true;
    }
}


void Board::move_snake()
{
    snake_->move();
    if (check())
    {
        update_board();
    }
}


bool Board::check()
{
    Piece* tmp;
    tmp = snake_->get_head();

    if (closed_borders)
    {
        if ((tmp->x < 0) or (tmp->x > columns_) or
                (tmp->y < 0) or (tmp->y > rows_))
        {
            return false;
        }
    }

    // Shifting the x and y periodically
    snake_->get_head()->x = modulo(tmp->x, columns_);
    snake_->get_head()->y = modulo(tmp->y, rows_);

    Tile current = board_.at(snake_->get_head()->y).at(snake_->get_head()->x);

    if (current.is_occupied)
    {
        return false;
    }

    if (current.has_food)
    {
        board_.at(snake_->get_head()->y).at(snake_->get_head()->x).has_food
                = false;
        snake_->append();
        add_food();
    }

    return true;
}


int Board::modulo(int a, int b)
{
    int r = a % b;

    while (r < 0)
    {
        r += b;
    }

    return r;
}


void Board::add_food()
{
    std::uniform_int_distribution<int> distribution_x(0, columns_-1);
    std::uniform_int_distribution<int> distribution_y(0, rows_-1);
    distribution_x(generator_);
    distribution_y(generator_);
    int x, y;

    x = distribution_x(generator_);
    y = distribution_y(generator_);
    while (board_.at(y).at(x).is_occupied)
    {
        x = distribution_x(generator_);
        y = distribution_y(generator_);
    }

    board_.at(y).at(x).has_food = true;
}

std::vector<std::vector<Tile> > Board::get_board()
{
    return board_;
}
