#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

// Configure Arduino board and interface

#include <QDialog>

#include "board.h"

#include "ui_arduinowindow.h"

class arduinoWindow : public QDialog, Ui::arduinowindow
{
    Q_OBJECT

public:
    explicit arduinoWindow(QWidget *parent = 0);
    ~arduinoWindow();

private:
    void boardComboChanged(QString text);

    Board *board;
    Ui::arduinowindow *ui;
};

#endif // ARDUINOWINDOW_H
