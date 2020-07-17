#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include "board.hh"
#include <QTimer>
#include <QColor>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent* event) override;

private slots:

    void initialize_game();


    // Slots for gameplay

    void play_game();

    void end_game();

    void draw_board();


    // Useful slots

    void display_time();

    void update_score();

    void toggle_command_buttons(bool enabled);


    // Slots for buttons

    void on_bordersCheckBox_clicked();

    void on_startGameButton_clicked();

    void on_quitGameButton_clicked();

    void on_leftButton_clicked();

    void on_upButton_clicked();

    void on_rightButton_clicked();

    void on_downButton_clicked();

    void on_infoButton_clicked();

    void on_rainbowCheckBox_clicked();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene_;

    // Constants describing scene coordinates
    const int BORDER_UP = 0;
    const int BORDER_DOWN = 240;
    const int BORDER_LEFT = 0;
    const int BORDER_RIGHT = 300;

    // Size of a snake piece
    const int SQUARE_SIDE = 10;

    // Number of horizontal cells
    const int COLUMNS = BORDER_RIGHT / SQUARE_SIDE;

    // Number of vertical cells
    const int ROWS = BORDER_DOWN / SQUARE_SIDE;

    const int BEGINNER_INTERVAL = 300;
    const int ADVANCED_INTERVAL = 200;
    const int PRO_INTERVAL = 100;

    Board* gameboard_;

    QBrush snake_color_;
    QBrush food_color_;

    QTimer clock_;
    QTimer timer_;
    int interval_;

    int elapsed_time_;

    std::vector< QColor > colors_;
};

#endif // MAINWINDOW_HH
