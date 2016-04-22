#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

// Configure Arduino board and interface

#include <QWizard>

#include "board.h"

#include "ui_arduinowindow.h"

class arduinoWindow : public QWizard, Ui::arduinowindow
{
    Q_OBJECT

public:
    explicit arduinoWindow(QWidget *parent = 0);
    ~arduinoWindow();

private:
    Board *board;
    Ui::arduinowindow *ui;
};

#endif // ARDUINOWINDOW_H
