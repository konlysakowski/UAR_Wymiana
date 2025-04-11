#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_buttonBox_accepted()
{
    wektorA = ui->dialogA->text();
    wektorB = ui->dialogB->text();
    delay = ui->dialogDelay->value();
    zaklocenia = ui->dialogZaklocenia->value();
}
void Dialog::setWektorA(QString text)
{
    ui->dialogA->setText(text);
}
void Dialog::setWektorB(QString text)
{
    ui->dialogB->setText(text);
}
void Dialog::setDelay(int delay)
{
    ui->dialogDelay->setValue(delay);
}
void Dialog::setZaklocenia(double zaklocenia)
{
    ui->dialogZaklocenia->setValue(zaklocenia);
}
