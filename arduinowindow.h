#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

// Configure Arduino board and interface

#include <QDialog>

namespace Ui {
class arduinowindow;
}

class arduinowindow : public QDialog
{
    Q_OBJECT

public:
    explicit arduinowindow(QWidget *parent = 0);
    ~arduinowindow();

private:
    Ui::arduinowindow *ui;
};

#endif // ARDUINOWINDOW_H
