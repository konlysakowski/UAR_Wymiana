#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QString getWektorA() const {return wektorA;}
    QString getWektorB() const {return wektorB;}
    int getDelay() const {return delay;}
    double getZaklocenia() const {return zaklocenia;}
    void setWektorA(QString text);
    void setWektorB(QString text);
    void setDelay(int delay);
    void setZaklocenia(double zaklocenia);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Dialog *ui;
    QString wektorA;
    QString wektorB;
    int delay;
    double zaklocenia;
};

#endif // DIALOG_H
