#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "board.hh"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QTimer>
#include <QColor>
#include <QMediaPlayer>

const QString INFO_TEXT = "Welcome to Snake! \n"
                          "\n"
                          "Move the snake around and try to catch as many \n"
                          "apples as you can! At the same time, try not to \n"
                          "crash into yourself. But keep alert - sometimes \n"
                          "golden apples may appear... \n"
                          "\n"
                          "Start the game by pressing 'Start Game' button. \n"
                          "By choosing 'Borders', the snake is trapped inside \n"
                          "the game board and crashing into the walls results \n"
                          "in failure. End the game by pressing 'Quit Game' or \n"
                          "closing the window. \n"
                          "\n"
                          "There are three difficulty settings: BEGINNER, \n"
                          "ADVANCED and PRO. The difficulty affects the \n"
                          "speed of the snake. \n"
                          "\n"
                          "\n"
                          "CONTROLS: \n"
                          "\n"
                          "A - Turn left \n"
                          "W - Turn up \n"
                          "D - Turn right \n"
                          "S - Turn down \n"
                          "\n"
                          "\n"
                          "Have fun! \n"
                          "\n"
                          "Jasper Kosonen \n"
                          "Tampere 18.7.2020";

class Split;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent* event) override;
    std::vector<std::string> (Split::*memberFunctionPointer)(const std::string&,
                                                             const char,
                                                             bool);

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

    void save_score();

    static bool compare_players(std::vector<std::string> first,
                                std::vector<std::string> second);

private:

    std::string getTimeStr_();

    bool read_file_(std::string scoreboard_address,
                    std::vector<std::string> &file_copy_vec,
                    int &file_row, int &players,
                    QString &qaddress, QString &qstr);

    void format_dataline_(std::string &line);

    std::vector<std::string> split_(const std::string& s,
                                    const char delimiter = '\t',
                                    bool ignore_empty = true);

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

    const int FOOD_SCORE = 1;
    const int GOLD_SCORE_SCALE = 5;

    const int GOLD_FREQUENCY = 10;

    const int GOLD_DURATION_BEGINNER = 15;
    const int GOLD_DURATION_ADVANCED = 20;
    const int GOLD_DURATION_PRO = 25;

    Board* gameboard_;

    QBrush snake_color_;
    QBrush head_color;
    QBrush food_color_;
    QBrush gold_color_;

    QTimer clock_;
    QTimer timer_;
    int interval_;

    int elapsed_time_;

    std::vector< QColor > colors_;

    // QMediaPlayer* music_;
};

// To use the split function while sorting a vector
class Split
{
public:
    MainWindow myMainWindow;
    std::vector<std::string> split(const std::string& s,
                                        const char delimiter = ' ',
                                        bool ignore_empty = true)
    {
        std::vector<std::string> result;
        std::string tmp = s;

        while(tmp.find(delimiter) != std::string::npos) {
            std::string new_part = tmp.substr(0, tmp.find(delimiter));
            tmp = tmp.substr(tmp.find(delimiter)+1, tmp.size());
            if(not (ignore_empty and new_part.empty())) {
                result.push_back(new_part);
            }
        }
        if(not (ignore_empty and tmp.empty())) {
            result.push_back(tmp);
        }
        return result;
    }
};

#endif // MAINWINDOW_HH
