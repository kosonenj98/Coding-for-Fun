/* This module contains the MainWindow class.
 *
 * MainWindow class operates the GUI and gameplay.
 * The GUI has a GraphicsScene that acts as the
 * game board. Timers send signals that make up
 * the game
 *
 * MainWindow also initiates both the Board and
 * Snake classes and manages them throughout the
 * game.
 *
 * Player score saving is implemented in this class.
 *
 * */


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


// Pixelated digits for countdown
const std::vector<std::vector<std::vector<int>>>
DIGITS =

{
  {{0, 1, 0},   // 1
   {0, 1, 0},
   {0, 1, 0},
   {0, 1, 0},
   {0, 1, 0}},

  {{1, 1, 1},   // 2
   {0, 0, 1},
   {1, 1, 1},
   {1, 0, 0},
   {1, 1, 1}},

  {{1, 1, 1},   // 3
   {0, 0, 1},
   {1, 1, 1},
   {0, 0, 1},
   {1, 1, 1}},
};

// Info text for user when '?' Button is clicked
const QString INFO_TEXT =
          "Welcome to Snake! \n"
          "\n"
          "Move the snake around and try to catch as many apples as \n"
          "you can! At the same time, try not to crash into yourself. \n"
          "But keep alert - sometimes golden apples may appear... \n"
          "\n"
          "Start the game by pressing 'Start Game' button. By choosing \n"
          "'Borders', the snake is trapped inside the game board and \n"
          "crashing into the walls results in failure. End the game by \n"
          "pressing 'Quit Game' or closing the window.\n"
          "\n"
          "There are three difficulty settings: BEGINNER, ADVANCED \n"
          "and PRO. The difficulty affects the speed of the snake. \n"
          "\n"
          "At the end, you can choose to save your score if you think \n"
          "you did well. Let's see who claims the first place... \n"
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


// Declaring this class for MainWindow
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

    /* The snake can also be moved using keyboard.
     *
     * Before the direction is changed, its validity is checked
     * via Snake class.
     * */
    void keyPressEvent(QKeyEvent* event) override;

    /* Function pointer to the split function
     *
     * This is used when player results are compared
     * while sorting a vector.
     * */
    std::vector<std::string> (Split::*memberFunctionPointer)(const std::string&,
                                                             const char,
                                                             bool);

private slots:

    // Sets necessary parameters for the game
    void initialize_game();


    // Slots for gameplay

    // Performs the countdown and starts the game
    void countdown();

    /* Calls various functions to perform
     * every necessary task for a game round
     *
     * Moves snake, checks if valid, updates score and
     * draws the board.
     *
     * QTimer timer_ is connected to this.
     * */
    void play_game();

    // Draws the board according to Board class
    void draw_board();

    /* Manages the dialog with player after GAME OVER
     *
     * Saving score and trying again
     * */
    void end_game();

    // Saves the player information to local .txt file
    void save_score();


    // Useful slots for GUI

    // For displaying the game time on GUI
    void display_time();

    // Calculating the player score and displaying it in GUI
    void update_score();

    // Sets control Buttons either enabled or disabled
    void toggle_control_buttons(bool enabled);

    // Draws the countdown digits in GUI
    void draw_digit(int dig);


    // Slots for Buttons and checkBoxes

    // Modifies the Board class attribute 'closed_borders_'
    void on_bordersCheckBox_clicked();

    // Initiates countdown and game settings
    void on_startGameButton_clicked();

    // Opens a dialog with user to end the game
    void on_quitGameButton_clicked();

    void on_leftButton_clicked();

    void on_upButton_clicked();

    void on_rightButton_clicked();

    void on_downButton_clicked();

    // Opens a pop up info window
    void on_infoButton_clicked();

    // Easter Egg
    void on_rainbowCheckBox_clicked();


    // Other

    /* Compares two vectors of player data
     *
     * Players are ranked according to scores.
     * If players have the same score, ranking is based on
     * gameplay time.
     *
     * first = vector containing player data
     * second = vector containing player data
     *
     * data = {rank, username, score, gameplay time, save date}
     * */
    static bool compare_players(std::vector<std::string> first,
                                std::vector<std::string> second);

private:

    /* Returns the current date as string in the format
     * Day Year-Month-Day Hours-Minutes-Seconds
     * */
    std::string get_time_str_();

    /* Reads the save file named 'scoreboard_address'
     *
     * Copies the old version to file_copy_vec line by line
     * Afterwards, starts comparing the current result to saved
     *
     * Informs the user if the reading is not successful
     * */
    bool read_file_(std::string scoreboard_address,
                    std::vector<std::string> &file_copy_vec,
                    int &file_row, int &players,
                    QString &qaddress, QString &qstr);

    /* Helps to align player datalines neatly
     *
     * Datalines are initially in the format
     * "RANK\tUSERNAME\tSCORE\tGAMEPLAY_TIME\tSAVE_DATE"
     * */
    void format_dataline_(std::string &line);

    /* Separates elements from stringlines using a delimeter
     *
     * Delimeter is '\t' by default.
     *
     * If ignore_empty, all empty elements are ignored.
     * */
    std::vector<std::string> split_(const std::string& s,
                                    const char delimiter = '\t',
                                    bool ignore_empty = true);

    Ui::MainWindow *ui;

    // The field where game is drawn in GUI
    QGraphicsScene* scene_;

    // Constants describing scene coordinates
    const int BORDER_UP = 0;
    const int BORDER_DOWN = 250;
    const int BORDER_LEFT = 0;
    const int BORDER_RIGHT = 290;

    // Size of a snake piece
    const int SQUARE_SIDE = 10;

    // Number of horizontal cells
    const int COLUMNS = BORDER_RIGHT / SQUARE_SIDE;

    // Number of vertical cells
    const int ROWS = BORDER_DOWN / SQUARE_SIDE;

    const int COUNTDOWN = 3;    // # of seconds in countdown

    /* Snake speeds
     *
     * Value in milliseconds, the interval for QTimer timer_
     * */
    const int BEGINNER_INTERVAL = 300;
    const int ADVANCED_INTERVAL = 200;
    const int PRO_INTERVAL = 100;

    // Constants for scoring
    const int FOOD_SCORE = 1;
    const int GOLD_SCORE_SCALE = 5; // FOOD_SCORE gets multipied

    /* Value for how likely gold food is generated
     *
     * The possibility is 1 / GOLD_FREQUENCY.
     * */
    const int GOLD_FREQUENCY = 10;

    // Constants for how long the gold exists
    const int GOLD_DURATION_BEGINNER = (COLUMNS + ROWS) / 4;
    const int GOLD_DURATION_ADVANCED = GOLD_DURATION_BEGINNER + 5;
    const int GOLD_DURATION_PRO = GOLD_DURATION_BEGINNER + 10;

    // A pointer to the game board
    Board* gameboard_;

    // Colors for GUI
    QBrush snake_color_;
    QBrush head_color;
    QBrush food_color_;
    QBrush gold_color_;

    // Timer variables
    QTimer countdown_;
    QTimer clock_;
    QTimer timer_;
    int interval_;

    // For managing time in GUI
    int countdown_time;
    int elapsed_time_;

    // Easter Egg
    std::vector< QColor > colors_;

    // Music for GUI if possible
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

        while (tmp.find(delimiter) != std::string::npos)
        {
            std::string new_part = tmp.substr(0, tmp.find(delimiter));
            tmp = tmp.substr(tmp.find(delimiter)+1, tmp.size());

            if (!(ignore_empty and new_part.empty()))
            {
                result.push_back(new_part);
            }
        }

        if (!(ignore_empty and tmp.empty()))
        {
            result.push_back(tmp);
        }

        return result;
    }
};

#endif // MAINWINDOW_HH
