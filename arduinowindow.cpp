#include "arduinowindow.h"
#include "ui_arduinowindow.h"

arduinowindow::arduinowindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::arduinowindow)
{
    ui->setupUi(this);
}

arduinowindow::~arduinowindow()
{
    delete ui;
}
