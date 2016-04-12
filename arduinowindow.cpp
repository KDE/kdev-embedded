#include "arduinowindow.h"
#include "ui_arduinowindow.h"

arduinowindow::arduinowindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::arduinowindow)
{
    ui->setupUi(this);
}

arduinowindow::~arduinowindow()
{
    delete ui;
}
