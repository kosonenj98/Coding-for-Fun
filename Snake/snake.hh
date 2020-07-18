#ifndef SNAKE_HH
#define SNAKE_HH


struct Piece
{
    int x;
    int y;
    Piece* next;
    Piece* previous;
};


class Snake
{
public:
    Snake(int x, int y);

    ~Snake();

    void move();

    bool change_direction(int dx, int dy);

    void append();

    Piece* get_head();

    Piece* get_tail();

private:
    Piece* head_;
    Piece* tail_;
    int dx_;
    int dy_;
};

#endif // SNAKE_HH
