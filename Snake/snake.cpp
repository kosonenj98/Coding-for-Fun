#include "snake.hh"

Snake::Snake(int x, int y)
{
    head_ = new Piece;
    tail_ = head_;
    head_->x = x;
    head_->y = y;
    head_->next = nullptr;
    head_->previous = nullptr;
    dx_ = -1; // Initially snake moving left
    dy_ = 0;
}

Snake::~Snake()
{
    while (tail_->previous != nullptr)
    {
        tail_ = tail_->previous;
        delete tail_->next;
    }
    tail_->next = nullptr;
    tail_ = nullptr;
    delete head_;
}

void Snake::append()
{
    // The new piece is added to the end.
    tail_->next = new Piece;
    tail_->next->x = tail_->x;
    tail_->next->y = tail_->y;
    tail_->next->next = nullptr;
    tail_->next->previous = tail_;
    tail_ = tail_->next;
}

void Snake::move()
{
    if (head_->next == nullptr)
        {
            head_->x += dx_;
            head_->y += dy_;
            return;
        }

    Piece* tmp = tail_;

    while (tmp->previous != nullptr)
    {
        if (!(tmp->x == tmp->previous->x &&
            tmp->y == tmp->previous->y))
        {
            tmp->x = tmp->previous->x;
            tmp->y = tmp->previous->y;
        }

        tmp = tmp->previous;
    }

    tmp->x += dx_;
    tmp->y += dy_;
}

void Snake::change_direction(int dx, int dy)
{
    if (!(dx + dx_ == 0 or dy + dy_ == 0))
    {
        dx_ = dx;
        dy_ = dy;
    }
}

Piece *Snake::get_head()
{
    return head_;
}
