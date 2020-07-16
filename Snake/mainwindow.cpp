#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QRectF>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Snake");

    // Graphics scene in which to draw rectangles.
    scene_ = new QGraphicsScene(this);

    // Setting the graphicsView object
    int left_margin = 35; // x coordinate
    int top_margin = 150; // y coordinate

    // The width of the graphicsView is BORDER_RIGHT added by 2,
    // since the borders take one pixel on each side
    // (1 on the left, and 1 on the right).
    // Similarly, the height of the graphicsView is BORDER_DOWN added by 2.
    ui->graphicsView->setGeometry(left_margin, top_margin,
                                  BORDER_RIGHT + 2, BORDER_DOWN + 2);
    ui->graphicsView->setScene(scene_);

    // The width of the scene_ is BORDER_RIGHT decreased by 1 and
    // the height of it is BORDER_DOWN decreased by 1, because
    // each snake piece is considered to be inside the sceneRect,
    // if its upper left corner is inside the sceneRect.
    scene_->setSceneRect(0, 0, BORDER_RIGHT - 1, BORDER_DOWN - 1);

    gameboard_ = new Board(COLUMNS, ROWS);

    snake_color_ = Qt::green;
    food_color_ = Qt::red;

    draw_board();
}

MainWindow::~MainWindow()
{
    delete gameboard_;
    delete ui;
}


void MainWindow::draw_board()
{
    scene_->clear();

    std::vector< std::vector< Tile > > board = gameboard_->get_board();

    for (int y = 0; y < ROWS; y++)
    {
        for (int x = 0; x < COLUMNS; x++)
        {
            if (board.at(y).at(x).is_occupied)
            {
                QPen pen(Qt::black);
                scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                SQUARE_SIDE, SQUARE_SIDE,
                                pen, snake_color_);
                continue;
            }

            if (board.at(y).at(x).has_food)
            {
                QPen pen(Qt::black);
                scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                SQUARE_SIDE, SQUARE_SIDE,
                                pen, food_color_);
                continue;
            }
        }
    }
}
