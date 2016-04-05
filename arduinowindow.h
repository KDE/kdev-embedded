#ifndef ARDUINOWINDOW_H
#define ARDUINOWINDOW_H

#include <QMainWindow>

namespace Ui {
class arduinowindow;
}

class arduinowindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit arduinowindow(QWidget *parent = 0);
    ~arduinowindow();

private:
    Ui::arduinowindow *ui;
};

#endif // ARDUINOWINDOW_H
