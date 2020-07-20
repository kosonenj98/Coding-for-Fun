#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>


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
    gameboard_->set_gold_frequency(GOLD_FREQUENCY);

    snake_color_ = Qt::green;
    head_color = Qt::gray;
    food_color_ = Qt::red;
    gold_color_ = Qt::yellow;

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

    /*
    music_ = new QMediaPlayer(this);
    music_->setMedia(QUrl("qrc:/Sounds/Badinerie.mp3"));
    music_->play();
    */

    initialize_game();
}


MainWindow::~MainWindow()
{
    // delete music_;
    gameboard_->~Board();
    gameboard_ = nullptr;
    delete ui;
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // The game has not started.
    if (!clock_.isActive())
    {
        return;
    }

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

        if (ui->lcdNumberScore->value() > 68 and
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
                                             "Save score?",
                                             QMessageBox::Yes,
                                             QMessageBox::No);

    if (messageBoxResult == QMessageBox::Yes)
    {
        save_score();
    }

    messageBoxResult = QMessageBox::question(0, "GAME OVER",
                                             "Try again?",
                                             QMessageBox::Yes,
                                             QMessageBox::No);

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
    QPen border(Qt::black);

    if (ui->bordersCheckBox->isChecked())
    {
        scene_->addRect(BORDER_LEFT, BORDER_UP, BORDER_RIGHT, BORDER_DOWN,
                        border, Qt::white);
    }

    if (ui->rainbowCheckBox->isChecked())
    {
        int stripe_height = BORDER_DOWN / 6;
        for (int i = 0; i < 6; i++)
        {
            border.setBrush(colors_.at(i));
            scene_->addRect(BORDER_LEFT, BORDER_UP +
                            i * stripe_height,
                            BORDER_RIGHT, BORDER_UP +
                            (i + 1) * stripe_height,
                            border, colors_.at(i));
        }
        border.setBrush(Qt::black);
    }

    if (!clock_.isActive())
    {
        return;
    }

    std::vector< std::vector< Tile > > board = gameboard_->get_board();

    for (int y = 0; y < ROWS; y++)
    {
        for (int x = 0; x < COLUMNS; x++)
        {
            if (board.at(y).at(x).is_occupied)
            {
                if (board.at(y).at(x).is_head)
                {
                    scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                    SQUARE_SIDE, SQUARE_SIDE,
                                    border, head_color);
                    continue;
                }

                scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                SQUARE_SIDE, SQUARE_SIDE,
                                border, snake_color_);
                continue;
            }

            if (board.at(y).at(x).has_food)
            {
                if (board.at(y).at(x).has_gold)
                {
                    scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                    SQUARE_SIDE, SQUARE_SIDE,
                                    border, gold_color_);
                    continue;
                }

                scene_->addRect(x * SQUARE_SIDE, y * SQUARE_SIDE,
                                SQUARE_SIDE, SQUARE_SIDE,
                                border, food_color_);
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
    int food = 0;
    int gold = 0;
    gameboard_->get_consumption(food, gold);

    int scores = food * FOOD_SCORE +
                 gold * (GOLD_SCORE_SCALE - 1) * FOOD_SCORE;

    ui->lcdNumberScore->display(scores);
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

    draw_board();

    if (ui->difficultyComboBox->currentText() == "BEGINNER")
    {
        interval_ = BEGINNER_INTERVAL;
        gameboard_->set_gold_duration(GOLD_DURATION_BEGINNER);
    }
    else if (ui->difficultyComboBox->currentText() == "ADVANCED")
    {
        interval_ = ADVANCED_INTERVAL;
        gameboard_->set_gold_duration(GOLD_DURATION_ADVANCED);
    }
    else
    {
        interval_ = PRO_INTERVAL;
        gameboard_->set_gold_duration(GOLD_DURATION_PRO);
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
                                             QMessageBox::Yes,
                                             QMessageBox::No);

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

    QMessageBox::information(0, "INFO", INFO_TEXT, QMessageBox::Ok);

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

void MainWindow::save_score()
{
    std::string scoreboard_address;
    if (ui->difficultyComboBox->currentText() == "BEGINNER")
    {
        scoreboard_address = "scoreboard_beginner.txt";
    }
    else if (ui->difficultyComboBox->currentText() == "ADVANCED")
    {
        scoreboard_address = "scoreboard_advanced.txt";
    }
    else
    {
        scoreboard_address = "scoreboard_pro.txt";
    }

    // Reading the file
    std::vector<std::string> file_copy_vec; // Copy of old scoreboard
    int file_row, players;
    QString qaddress = QString::fromStdString(scoreboard_address);
    QString qstr;   // For printing into QWidgets

    // Cannot read file
    if (!read_file_(scoreboard_address, file_copy_vec,
                    file_row, players, qaddress, qstr))
    {
        return;
    }

    // Current player info
    std::string username;
    int score;
    int minutes, seconds;
    std::string date = getTimeStr_();

    // Asking the current player's username
    bool ok;
    QString text = QInputDialog::getText(0, "SAVING SCORE",
                                         "Username: (Max 10 symbols)",
                                         QLineEdit::Normal, "", &ok);

    if (ok and !text.isEmpty())
    {
        username = text.toStdString();

        // The username cannot exceed over 10 symbols for aligment
        // purposes.
        if (username.size() > 10)
        {
            username.erase(username.begin() + 10, username.end());
        }

        std::vector<std::string> name_bits = split_(username, ' ');

        // Name only contains space.
        if (name_bits.size() == 0)
        {
            username = "Anonymous";
        }
        else
        {
            username = "";
            for (int i = 0;
                 i < static_cast<int>(name_bits.size()) - 1; i++)
            {
                username += name_bits.at(i) + ' ';
            }
            username += name_bits.back();
        }

    }
    // No specific username given
    else if (ok and text.isEmpty())
    {
        username = "Anonymous";
    }
    // Player pressed "Cancel".
    else
    {
        return;
    }

    score = static_cast<int>(ui->lcdNumberScore->value());
    minutes = elapsed_time_ / 60;
    seconds = elapsed_time_ - minutes * 60;

    std::string line;

    line = "1.\t" +
            username + '\t' +
            std::to_string(score) + " pts" + '\t' +
            std::to_string(minutes) + " min " +
            std::to_string(seconds) + " sec" + '\t' +
            date;

    format_dataline_(line);

    // There are no previous saves.
    if (players == 0)
    {
        file_copy_vec.at(file_row) = line;
    }

    // Determine the order of the saves
    else
    {
        // A vec containing the previous players' info
        std::vector<std::vector<std::string>> player_info_list;

        std::string player_line;

        for (int i = 0; i < players; i++)
        {
            player_line = file_copy_vec.at(file_row - players + i);
            player_info_list.push_back(split_(player_line));
        }

        // Adding the current player result
        player_info_list.push_back(split_(line));
        players++;

        // Sorting the players according to their scores
        std::sort(player_info_list.begin(),
                  player_info_list.end(),
                  compare_players);

        // Determining the players' ranks
        for (int i = 0; i < players; i++)
        {
            if (i > 0)
            {
                // Players with same score have the same rank.
                if (player_info_list.at(i).at(2) ==
                        player_info_list.at(i - 1).at(2))
                {
                    player_info_list.at(i).at(0) = " ";
                    continue;
                }
            }
            std::string rank = std::to_string(i + 1);
            player_info_list.at(i).at(0) = rank + ".";
        }

        // Removing the previous saves from the copied version
        file_copy_vec.erase(file_copy_vec.begin() + file_row - (players - 1),
                            file_copy_vec.begin() + file_row);

        // Inserting the ordered saves
        for (int i = 0; i < players; i++)
        {
            player_line = player_info_list.at(i).at(0) + '\t' +
                    player_info_list.at(i).at(1) + '\t' +
                    player_info_list.at(i).at(2) + '\t' +
                    player_info_list.at(i).at(3) + '\t' +
                    player_info_list.at(i).at(4);

            format_dataline_(player_line);

            file_copy_vec.insert(file_copy_vec.begin() + file_row -
                                 (players - 1) + i, player_line);
        }
    }

    // Writing the saves to file
    std::ofstream writefile_obj(scoreboard_address);

    for (std::string s : file_copy_vec)
    {
        writefile_obj << s + '\n';
    }
    writefile_obj.close();

    qstr = "Saved successfully to \n"
                   "'" + qaddress + "'.";
    QMessageBox::information(0, "SAVE OK", qstr, QMessageBox::Ok);

}

std::vector<std::string> MainWindow::split_(const std::string& s,
                                            const char delimiter,
                                            bool ignore_empty)
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

void MainWindow::format_dataline_(std::string &line)
{
    // Default settings
    std::string name_separator = "\t\t";
    std::string pts_aligner = " ";
    std::string min_aligner = " ";
    std::string sec_aligner = " ";

    std::vector<std::string> pieces = split_(line);

    std::vector<std::string> time_vec = split_(pieces.at(3), ' ');

    int minutes = std::stoi(time_vec.at(0));
    int seconds = std::stoi(time_vec.at(2));

    // Setting the separators for aligment purposes
    if (pieces.at(1).size() > 7)
    {
        name_separator = '\t';
    }

    int score_as_int = std::stoi(pieces.at(2));

    if (score_as_int > 99)
    {
        pts_aligner = "";
    }
    else if (score_as_int < 10)
    {
        pts_aligner = "  ";
    }

    std::string score_as_string = std::to_string(score_as_int);
    pieces.at(2) = pts_aligner + score_as_string + " pts";

    if (minutes > 9)
    {
        min_aligner = "";
    }
    else if (seconds < 10)
    {
        sec_aligner = "  ";
    }

    std::string minutes_as_string, seconds_as_string;
    minutes_as_string = std::to_string(minutes);
    seconds_as_string = std::to_string(seconds);
    pieces.at(3) = min_aligner + minutes_as_string + " min" +
            sec_aligner + seconds_as_string + " sec";

    // Final dataline
    line = pieces.at(0) + '\t' +
            pieces.at(1) + name_separator +
            pieces.at(2) + "\t\t" +
            pieces.at(3) + '\t' +
            pieces.at(4);

}

std::string MainWindow::getTimeStr_()
{
    auto t = std::chrono::system_clock::now();
    std::time_t now = std::chrono::system_clock::to_time_t(t);

    std::string s(24, ' ');
    std::strftime(&s.at(0), s.size(), "%a %Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    s.erase(s.begin() + 23, s.end());

    return s;
}

bool MainWindow::read_file_(std::string scoreboard_address,
                            std::vector<std::string>& file_copy_vec,
                            int& file_row, int& players,
                            QString& qaddress, QString& qstr)
{
    std::ifstream copyfile_obj(scoreboard_address);

    std::string line;
    std::string start_point = "WITHOUT BORDERS";    // Default game mode
    file_row = 0;
    players = 0;
    std::vector<std::string> pieces;


    if (ui->bordersCheckBox->isChecked())
    {
        start_point = "WITH BORDERS";
    }

    // The scoreboard file cannot be opened.
    if (!copyfile_obj)
    {
        qstr = "Could not find file \n"
               "'" + qaddress + "'.";
        QMessageBox::critical(0, "FILE ERROR", qstr, QMessageBox::Ok);
        return false;
    }

    // Copying the old scoreboard into vec
    while (getline(copyfile_obj, line))
    {
        file_copy_vec.push_back(line);
    }
    copyfile_obj.close();

    // Opening the file again for collecting the previous saves
    std::ifstream comparefile_obj(scoreboard_address);

    bool counting_players= false;    // Comparing results if true

    while (getline(comparefile_obj, line))
    {

        // Start counting previous saves
        if (counting_players)
        {
            pieces = split_(line);

            // Reached end of the save section
            if (pieces.size() == 0)
            {
                break;
            }
            // Invalid formatting in save file
            else if (pieces.size() != 5)
            {
                qstr = "Please check the format in \n"
                               "'" + qaddress + "'.";
                QMessageBox::critical(0, "FORMAT ERROR",
                                      qstr, QMessageBox::Ok);
                return false;
            }

            players++;
        }

        if (line == start_point)
        {
            counting_players = true;
        }

        file_row++;
    }
    comparefile_obj.close();

    // Cannot find correct place to save
    if (file_row > static_cast<int>(file_copy_vec.size()))
    {
        qstr = "Please check the format in \n"
                       "'" + qaddress + "'.";
        QMessageBox::critical(0, "FORMAT ERROR",
                              qstr, QMessageBox::Ok);
        return false;
    }

    return true;
}

bool MainWindow::compare_players(std::vector<std::string> first,
                                 std::vector<std::string> second)
{
    // If players have the same score, they will be printed
    // in increasing gameplay time order.
    if (std::stoi(first.at(2)) == std::stoi(second.at(2)))
    {
        Split split_f;
        split_f.myMainWindow.memberFunctionPointer =
                &Split::split;

        std::vector<std::string> time_vec_first =
                ((split_f).*(split_f.
                myMainWindow.
                memberFunctionPointer))
                (first.at(3), ' ', true);

        std::vector<std::string> time_vec_second =
                ((split_f).*(split_f.
                myMainWindow.
                memberFunctionPointer))
                (second.at(3), ' ', true);

        int time_first = std::stoi(time_vec_first.at(0)) * 60 +
                std::stoi(time_vec_first.at(2));

        int time_second = std::stoi(time_vec_second.at(0)) * 60 +
                std::stoi(time_vec_second.at(2));


        return (time_first < time_second);
    }

    return (std::stoi(first.at(2)) > std::stoi(second.at(2)));
}
