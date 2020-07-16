#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "board.hh"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void draw_board();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene_;

    // Constants describing scene coordinates
    const int BORDER_UP = 0;
    const int BORDER_DOWN = 230;
    const int BORDER_LEFT = 0;
    const int BORDER_RIGHT = 300;

    // Size of a snake piece
    const int SQUARE_SIDE = 10;

    // Number of horizontal cells
    const int COLUMNS = BORDER_RIGHT / SQUARE_SIDE;

    // Number of vertical cells
    const int ROWS = BORDER_DOWN / SQUARE_SIDE;

    Board* gameboard_;

    QBrush snake_color_;

    QBrush food_color_;

};

#endif // MAINWINDOW_HH
