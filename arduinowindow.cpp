#include "arduinowindow.h"

#include <QDebug>

#include <interfaces/isession.h>
#include <interfaces/icore.h>

#include "toolkit.h"

using namespace KDevelop;

// TODO create a abstracttablemodel to update img and text
arduinoWindow::arduinoWindow(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    // Just select the options
    boardCombo->setEditable(false);
    interfaceCombo->setEditable(false);
    baudCombo->setEditable(false);

    //board = new Board;
    Board::instance().update();
    QStringList boardsId = Board::instance().boardList;
    QStringList boardsName;

    foreach(const QString &boardId, boardsId)
        boardsName << Board::instance().boards[boardId].name;

    boardCombo->addItems(boardsName);
    boardComboChanged(boardCombo->currentText());
    connect(boardCombo, &QComboBox::currentTextChanged, this,  &arduinoWindow::boardComboChanged);
}

void arduinoWindow::boardComboChanged(QString text)
{
    QString id = Board::instance().getIdFromName(text);
    QStringList baud = Board::instance().boards[id].upSpeed;
    baudCombo->clear();
    baudCombo->addItems(baud);
}

arduinoWindow::~arduinoWindow()
{
    delete ui;
}
