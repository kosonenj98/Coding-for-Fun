/* This module contains the Snake class.
 *
 * Snake class operates the actual snake.
 * The snake is constructed via a two way
 * linked list. The head and the tail are
 * kept track of. This way, every piece
 * needs to only know where it self is
 * and where its neighbouring bits are.
 *
 * */


#ifndef SNAKE_HH
#define SNAKE_HH


// A struct for a snake piece
struct Piece
{
    int x;  // x coordinate
    int y;  // y coordinate
    Piece* next;    // address to next piece
    Piece* previous;    // address to previous piece
};


/* Class Snake operates the snake.
 *
 * The head and tail of the snake are being
 * kept track of. The snake has a direction
 * encoded in dx_ and dy_.
 *
 * Positive dx_ corresponds to right and
 * positive dy_ to down.
 *
 * The snake moves from tail to head such that
 * every piece gets shifted to the place where
 * the previous place was. Only head moves
 * according to dx_ and dy_.
 *
 * */
class Snake
{
public:

    // The snake will be initialized at point (x,y).
    Snake(int x, int y);

    ~Snake();

    // Moves the snake
    void move();

    /* Changes the snake's direction by altering
     * dx_ and dy_ to dx and dy.
     *
     * Returns true if succeeds. Otherwise false.
     * */
    bool change_direction(int dx, int dy);

    // Adds a new snake piece at the end.
    void append();

    // Returns a pointer to head.
    Piece* get_head();

    // Returns a pointer to tail.
    Piece* get_tail();

private:
    Piece* head_;
    Piece* tail_;
    int dx_;
    int dy_;
};

#endif // SNAKE_HH
