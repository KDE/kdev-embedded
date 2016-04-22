#include "arduinowindow.h"

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include "toolkit.h"

using namespace KDevelop;

arduinoWindow::arduinoWindow(QWidget *parent) :
    QWizard(parent)
{
    setupUi(this);
    
    board = new Board;
    board->load();
}

arduinoWindow::~arduinoWindow()
{
    delete ui;
}
