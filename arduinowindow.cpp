#include "arduinowindow.h"

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include "toolkit.h"

using namespace KDevelop;

arduinoWindow::arduinoWindow(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    
    //board = new Board;
    Board::instance().update();
}

arduinoWindow::~arduinoWindow()
{
    delete ui;
}
