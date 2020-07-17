#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QRectF>
#include <QKeyEvent>
#include <QMessageBox>


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

    // The width of the graphicsView is BORDER_RIGHT added by 3,
    // since the borders take one pixel on each side
    // (1 on the left, and 1 on the right).
    // Similarly, the height of the graphicsView is BORDER_DOWN added by 3.
    ui->graphicsView->setGeometry(left_margin, top_margin,
                                  BORDER_RIGHT + 3,
                                  BORDER_DOWN + 3);

    ui->graphicsView->setScene(scene_);

    // The width of the scene_ is BORDER_RIGHT decreased by 1 and
    // the height of it is BORDER_DOWN decreased by 1, because
    // each snake piece is considered to be inside the sceneRect,
    // if its upper left corner is inside the sceneRect.
    scene_->setSceneRect(0, 0, BORDER_RIGHT - 1, BORDER_DOWN - 1);

    gameboard_ = new Board(COLUMNS, ROWS);

    snake_color_ = Qt::green;
    food_color_ = Qt::red;

    QColor color;
    color.setRgb(228, 3, 3);
    colors_.push_back(color);
    color.setRgb(255, 140, 0);
    colors_.push_back(color);
    color.setRgb(255, 237, 0);
    colors_.push_back(color);
    color.setRgb(0, 128, 38);
    colors_.push_back(color);
    color.setRgb(0, 77, 255);
    colors_.push_back(color);
    color.setRgb(117, 7, 135);
    colors_.push_back(color);

    clock_.setSingleShot(false);
    timer_.setSingleShot(false);

    connect(&clock_, &QTimer::timeout, this, &MainWindow::display_time);
    connect(&timer_, &QTimer::timeout, this, &MainWindow::play_game);

    ui->difficultyComboBox->addItem("BEGINNER");
    ui->difficultyComboBox->addItem("ADVANCED");
    ui->difficultyComboBox->addItem("PRO");

    ui->rainbowCheckBox->setVisible(false);

    initialize_game();
}


MainWindow::~MainWindow()
{
    gameboard_->~Board();
    gameboard_ = nullptr;
    delete ui;
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Move left
    if (event->key() == Qt::Key_A)
    {
        if (gameboard_->get_snake()->change_direction(-1, 0))
        {
            play_game();
        }
    }
    // Move up
    else if (event->key() == Qt::Key_W)
    {
        if (gameboard_->get_snake()->change_direction(0, -1))
        {
            play_game();
        }
    }
    // Move right
    else if (event->key() == Qt::Key_D)
    {
        if (gameboard_->get_snake()->change_direction(1, 0))
        {
            play_game();
        }
    }
    // Move down
    else if (event->key() == Qt::Key_S)
    {
        if (gameboard_->get_snake()->change_direction(0, 1))
        {
            play_game();
        }
    }
}

void MainWindow::initialize_game()
{
    if (gameboard_ == nullptr)
    {
        gameboard_ = new Board(COLUMNS, ROWS);
    }

    ui->startGameButton->setEnabled(true);
    ui->difficultyComboBox->setEnabled(true);
    ui->bordersCheckBox->setEnabled(true);

    ui->bordersCheckBox->setChecked(false);

    ui->lcdNumberSeconds->display(0);
    ui->lcdNumberMinutes->display(0);
    ui->lcdNumberScore->display(0);

    elapsed_time_ = 0;

    toggle_command_buttons(false);

    draw_board();
}


void MainWindow::play_game()
{
    if (gameboard_->move_snake())
    {
        update_score();
        draw_board();

        if (ui->lcdNumberScore->value() > 5 and
                ui->difficultyComboBox->currentText() == "PRO" and
                ui->bordersCheckBox->isChecked())
        {
            ui->rainbowCheckBox->setVisible(true);
        }

        return;
    }

    end_game();
}


void MainWindow::end_game()
{
    clock_.stop();
    timer_.stop();

    int messageBoxResult = 0;
    messageBoxResult = QMessageBox::question(0, "GAME OVER",
                                             "Do you want to try again?",
                                             QMessageBox::Yes, QMessageBox::No);

    if (messageBoxResult == QMessageBox::Yes)
    {
        gameboard_->~Board();
        gameboard_ = nullptr;
        initialize_game();
    }
    else
    {
        this->close();
    }
}


void MainWindow::draw_board()
{
    scene_->clear();

    if (ui->bordersCheckBox->isChecked())
    {
        QPen border(Qt::black);
        scene_->addRect(BORDER_LEFT, BORDER_UP, BORDER_RIGHT, BORDER_DOWN,
                        border, Qt::white);
    }

    if (ui->rainbowCheckBox->isChecked())
    {
        int stripe_height = BORDER_DOWN / 6;
        for (int i = 0; i < 6; i++)
        {
            QPen border(colors_.at(i));
            scene_->addRect(BORDER_LEFT, BORDER_UP + i * stripe_height,
                            BORDER_RIGHT, BORDER_UP + (i + 1) * stripe_height,
                            border, colors_.at(i));
        }
    }

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


void MainWindow::display_time()
{
    elapsed_time_++;
    int minutes = elapsed_time_ / 60;
    int seconds = elapsed_time_ - minutes * 60;
    ui->lcdNumberMinutes->display(minutes);
    ui->lcdNumberSeconds->display(seconds);
}


void MainWindow::update_score()
{
    int scores = gameboard_->get_snake()->get_length();
    ui->lcdNumberScore->display(scores - 1);
}


void MainWindow::toggle_command_buttons(bool enabled)
{
    if (enabled)
    {
        ui->leftButton->setEnabled(true);
        ui->upButton->setEnabled(true);
        ui->rightButton->setEnabled(true);
        ui->downButton->setEnabled(true);
        return;
    }

    ui->leftButton->setDisabled(true);
    ui->upButton->setDisabled(true);
    ui->rightButton->setDisabled(true);
    ui->downButton->setDisabled(true);
}


void MainWindow::on_bordersCheckBox_clicked()
{
    if (ui->bordersCheckBox->isChecked())
    {
        gameboard_->set_closed_borders(true);
    }
    else
    {
        gameboard_->set_closed_borders(false);
    }

    draw_board();
}


void MainWindow::on_startGameButton_clicked()
{
    ui->startGameButton->setDisabled(true);
    ui->difficultyComboBox->setDisabled(true);
    ui->bordersCheckBox->setDisabled(true);

    clock_.start(1000);

    if (ui->difficultyComboBox->currentText() == "BEGINNER")
    {
        interval_ = BEGINNER_INTERVAL;
    }
    else if (ui->difficultyComboBox->currentText() == "ADVANCED")
    {
        interval_ = ADVANCED_INTERVAL;
    }
    else
    {
        interval_ = PRO_INTERVAL;
    }

    timer_.start(interval_);

    toggle_command_buttons(true);
}


void MainWindow::on_quitGameButton_clicked()
{
    if (clock_.isActive())
    {
        clock_.stop();
        timer_.stop();
    }

    int messageBoxResult = 0;
    messageBoxResult = QMessageBox::question(0, "QUIT", "Are you sure?",
                                             QMessageBox::Yes, QMessageBox::No);
    if (messageBoxResult == QMessageBox::Yes)
    {
        this->close();
    }

    if (!ui->startGameButton->isEnabled())
    {
        clock_.start(1000);
        timer_.start(interval_);
    }
}


void MainWindow::on_leftButton_clicked()
{
    if (gameboard_->get_snake()->change_direction(-1, 0))
    {
        play_game();
    }
}

void MainWindow::on_upButton_clicked()
{
    if (gameboard_->get_snake()->change_direction(0, -1))
    {
        play_game();
    }
}

void MainWindow::on_rightButton_clicked()
{
    if (gameboard_->get_snake()->change_direction(1, 0))
    {
        play_game();
    }
}

void MainWindow::on_downButton_clicked()
{
    if (gameboard_->get_snake()->change_direction(0, 1))
    {
        play_game();
    }
}

void MainWindow::on_infoButton_clicked()
{
    if (clock_.isActive())
    {
        clock_.stop();
        timer_.stop();
    }
    QMessageBox::information(0, "INFO", "Info about the gamlorem lipsum lapsum",
                                             QMessageBox::Ok);
    if (!ui->startGameButton->isEnabled())
    {
        clock_.start(1000);
        timer_.start(interval_);
    }
}

void MainWindow::on_rainbowCheckBox_clicked()
{
    draw_board();
}
