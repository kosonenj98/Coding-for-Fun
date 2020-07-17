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

    void append();

    void move();

    bool change_direction(int dx, int dy);

    Piece* get_head();

    int get_length();

private:
    Piece* head_;
    Piece* tail_;
    int dx_;
    int dy_;
    int length_;
};

#endif // SNAKE_HH
