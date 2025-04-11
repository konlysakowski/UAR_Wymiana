 #include "mainwindow.h"
#include "dialog.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsTextItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_timer(new QTimer(this)),  m_time(0), m_prevSetpoint(0.0), m_prevOutput(0.0) {

    ui->setupUi(this);

    sterowaniePlot = ui->sterowanie;
    uchybPlot = ui->uchyb;
    zadanaPlot = ui->zadana;
    ui->sygnalcomboBox->addItem("Skok");
    ui->sygnalcomboBox->addItem("Sinusoida");
    ui->sygnalcomboBox->addItem("Prostokątny");

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopSimulation);
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    //connect(ui->aktualizujButton, &QPushButton::clicked, this, &MainWindow::updateAllParams);


    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateSimulation);

    this->ui->kpLabel->setValue(0.5);
    this->ui->tiLabel->setValue(10);
    this->ui->tdLabel->setValue(0.1);
    this->ui->vecaLabel->setText("-0.4");
    this->ui->vecbLabel->setText("0.6");
    this->ui->zakloceniaLabel->setText("0.01");
    this->ui->delayLabel->setText("1");
    this->ui->amplitudaLabel->setValue(1);
    this->ui->okresLabel->setValue(40);
    this->ui->cyklLabel->setValue(0.5);
    this->ui->aktywacjaLabel->setValue(1);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        updateAllParams();
    }
}




std::unique_ptr<ARX> MainWindow::updateARXParams()
{
    QStringList a_values = ui->vecaLabel->text().split(",");
    QStringList b_values = ui->vecbLabel->text().split(",");
    std::vector<double> vec_a, vec_b;

    for (const auto& val : a_values) {
        bool ok;
        double number = val.toDouble(&ok);
        if (ok) {
            vec_a.push_back(number);
        } else {
            //QMessageBox::warning(this, "Błąd danych", "Nieprawidłowa wartość w polu vecA: " + val);
            return nullptr;
        }
    }
    for (const auto& val : b_values) {
        bool ok;
        double number = val.toDouble(&ok);
        if (ok) {
            vec_b.push_back(number);
        } else {
            //QMessageBox::warning(this, "Błąd danych", "Nieprawidłowa wartość w polu vecB: " + val);
            return nullptr;
        }
    }

    bool delayOk;
    int delay = ui->delayLabel->text().toInt(&delayOk);
    if (!delayOk || delay<0) {
        //QMessageBox::warning(this, "Błąd danych", "Nieprawidłowa wartość opóźnienia.");
        return nullptr;
    }


    bool zakloceniaOk;
    double zaklocenia = ui->zakloceniaLabel->text().toDouble(&zakloceniaOk);
    if (!zakloceniaOk || zaklocenia<0) {
        //QMessageBox::warning(this, "Błąd danych", "Nieprawidłowa wartość zakłócenia.");
        return nullptr;
    }

    auto arx = std::make_unique<ARX>(vec_a, vec_b, delay, zaklocenia);
    return arx;
}


std::unique_ptr<PID> MainWindow::updatePIDParams()
{

    double kp = ui->kpLabel->value();
    double ti = ui->tiLabel->value();
    double td = ui->tdLabel->value();

    return std::make_unique<PID>(kp, ti, td);
}


std::unique_ptr<WartoscZadana> MainWindow::updateSignalParams()
{
    auto wartoscZadana = std::make_unique<WartoscZadana>();

    double amplitude = ui->amplitudaLabel->value();
    double period = ui->okresLabel->value();
    double dutyCycle = ui->cyklLabel->value();
    int activationTime = ui->aktywacjaLabel->value();


    TypSygnalu type = static_cast<TypSygnalu>(ui->sygnalcomboBox->currentIndex());

    switch (type) {
    case skok:
        wartoscZadana->ustawskok(amplitude, activationTime);
        break;
    case sinusoida:
        wartoscZadana->ustawsinusoide(amplitude, period);
        break;
    case prostokatny:
        wartoscZadana->ustawprostokatny(amplitude, period, dutyCycle);
        break;
    default:
        //QMessageBox::warning(this, "Błąd danych", "Nieznany typ sygnału.");
        return nullptr;
    }

    return wartoscZadana;
}


void MainWindow::initSimulation() {

    auto arx = this->updateARXParams();

    auto pid = this->updatePIDParams();

    auto wartoscZadana = this->updateSignalParams();

    if (arx==nullptr||pid==nullptr||wartoscZadana==nullptr)
    {
        m_symulacja = nullptr;
    } else
    {
    m_symulacja = std::make_unique<Symulacja>(std::move(arx), std::move(pid), std::move(wartoscZadana));
    m_time = 0;
    m_prevSetpoint = 0.0;
    m_prevOutput = 0.0;
    if(resetclicked)
    {

        setupPlots();
        resetclicked = false;
    }
    }
}
void MainWindow::setupPlots()
{
    sterowaniePlot->addGraph();
    sterowaniePlot->addGraph();
    sterowaniePlot->addGraph();
    sterowaniePlot->addGraph();
    uchybPlot->addGraph();
    zadanaPlot->addGraph();
    zadanaPlot->addGraph();
    if(!wasreseted)
    {
        sterowaniePlot->xAxis->setLabel("t");
        sterowaniePlot->yAxis->setLabel("y");

        uchybPlot->xAxis->setLabel("t");
        uchybPlot->yAxis->setLabel("y");

        zadanaPlot->xAxis->setLabel("t");
        zadanaPlot->yAxis->setLabel("y");

    zadanaPlot->plotLayout()->insertRow(0);
    zadanaPlot->plotLayout()->addElement(0, 0, new QCPTextElement(zadanaPlot, "Wartość zadana", QFont("Arial", 12, QFont::Bold)));
    sterowaniePlot->plotLayout()->insertRow(0);
    sterowaniePlot->plotLayout()->addElement(0, 0, new QCPTextElement(sterowaniePlot, "PID", QFont("Arial", 12, QFont::Bold)));
    uchybPlot->plotLayout()->insertRow(0);
    uchybPlot->plotLayout()->addElement(0, 0, new QCPTextElement(uchybPlot, "Uchyb", QFont("Arial", 12, QFont::Bold)));
    }
    sterowaniePlot->graph(0)->setPen(QPen(Qt::red));
    sterowaniePlot->graph(1)->setPen(QPen(Qt::blue));
    sterowaniePlot->graph(2)->setPen(QPen(Qt::green));
    sterowaniePlot->graph(3)->setPen(QPen(QColorConstants::DarkMagenta));


    uchybPlot->graph(0)->setPen(QPen(Qt::green));

    zadanaPlot->graph(0)->setPen(QPen(Qt::red));
    zadanaPlot->graph(1)->setPen(QPen(Qt::blue));

    sterowaniePlot->legend->setVisible(true);
    uchybPlot->legend->setVisible(true);
    zadanaPlot->legend->setVisible(true);
    uchybPlot->graph(0)->setName("Uchyb");

    zadanaPlot->graph(0)->setName("Zadana");
    zadanaPlot->graph(1)->setName("Regulowana");

    sterowaniePlot->graph(0)->setName("Składowa P");
    sterowaniePlot->graph(1)->setName("Składowa I");
    sterowaniePlot->graph(2)->setName("Składowa D");
    sterowaniePlot->graph(3)->setName("S.Sterujący");
    zadanaPlot->xAxis->setRange(m_x-1, m_x);
    zadanaPlot->yAxis->setRange(m_yZ-6, m_yZ);
    sterowaniePlot->xAxis->setRange(m_x-1, m_x);
    sterowaniePlot->yAxis->setRange(m_yZ-6, m_yZ);
    uchybPlot->xAxis->setRange(m_x-1, m_x);
    uchybPlot->yAxis->setRange(m_yZ-6, m_yZ);
    zoom(true);

    sterowaniePlot->replot();
    uchybPlot->replot();
    zadanaPlot->replot();
}

void MainWindow::startSimulation() {
    if(m_time > 0)
    {
        m_timer->start(this->ui->interwalSpinBox->value());
        return;
    }
initSimulation();
    try {
        if (!m_symulacja) {
            throw std::logic_error("Symulacja nie została poprawnie zainicjalizowana.");
        }

        zoom(false);
        m_timer->start(this->ui->interwalSpinBox->value());
    } catch (const std::exception& ex) {
        //QMessageBox::critical(this, "Błąd", ex.what());
    }

}

void MainWindow::stopSimulation() {
    m_timer->stop();
    zoom(true);


}

void MainWindow::resetSimulation() {
    resetclicked = true;
    ui->zadaneLabel->setText("0.00");
    ui->wyjscieLabel->setText("0.00");
    sterowaniePlot->clearGraphs();
    zadanaPlot->clearGraphs();
    uchybPlot->clearGraphs();
    wasreseted = true;
    sterowaniePlot->replot();
    zadanaPlot->replot();
    uchybPlot->replot();
    initSimulation();
}

void MainWindow::updateAllParams() {
    if(m_time == 0)
    {
        return;
    }
    try {

        auto arx = this->updateARXParams();

        auto pid = this->updatePIDParams();

        auto wartoscZadana = this->updateSignalParams();



        if (!(arx==nullptr||pid==nullptr||wartoscZadana==nullptr))
        {
            if (m_symulacja) {
                m_symulacja->getARX()->updateParamsWithoutReset(arx->getVektory().first, arx->getVektory().second, arx->getDelay(), arx->getZaklocenia());
                m_symulacja->getPID()->updateParamsWithoutReset(ui->kpLabel->value(), ui->tiLabel->value(), ui->tdLabel->value());
                TypSygnalu type = static_cast<TypSygnalu>(ui->sygnalcomboBox->currentIndex());
                m_symulacja->getWartoscZadana()->updateParams(type, ui->amplitudaLabel->value(), ui->okresLabel->value(), ui->cyklLabel->value(), ui->aktywacjaLabel->value());

                m_timer->start(ui->interwalSpinBox->value());
            }



            else {
            m_symulacja = std::make_unique<Symulacja>(std::move(arx), std::move(pid), std::move(wartoscZadana));
            m_timer->start(ui->interwalSpinBox->value());
            }

        }
    } catch (const std::exception& ex) {
        //QMessageBox::critical(this, "Błąd aktualizacji", ex.what());
    }
}



void MainWindow::updateSimulation() {
    try {
        double setpoint = m_symulacja->getWartoscZadana()->generuj();
        double measured = m_prevOutput;
        double error = setpoint - measured;
        double pComponent;
        double iComponent;
        if(ui->liczenieCalkiLabel->isChecked() == true)
        {
             pComponent = m_symulacja->getPID()->obliczP_TiWSumie(setpoint, measured);
             iComponent = m_symulacja->getPID()->obliczI_TiWSumie(setpoint, measured);
        }
        else
        {
             pComponent = m_symulacja->getPID()->obliczP(setpoint, measured);
             iComponent = m_symulacja->getPID()->obliczI();
        }

        double dComponent = m_symulacja->getPID()->obliczD(setpoint, measured);

        m_symulacja->setZadane(setpoint);
        double output;
        if(ui->liczenieCalkiLabel->isChecked() == true)
        {
            output = m_symulacja->krok_TiWSumie();
        }
        else
        {
            output = m_symulacja->krok();
        }


        ui->zadaneLabel->setText(QString::number(setpoint, 'f', 2));
        ui->wyjscieLabel->setText(QString::number(output, 'f', 2));



        if (m_time > 0) {
            m_x = m_time;
            zadanaPlot->graph(0)->addData(m_time-1,setpoint);
            zadanaPlot->graph(1)->addData(m_time-1,measured);
            sterowaniePlot->graph(0)->addData(m_time-1,pComponent);
            sterowaniePlot->graph(1)->addData(m_time-1,iComponent);
            sterowaniePlot->graph(2)->addData(m_time-1,dComponent);
            if(ui->liczenieCalkiLabel->isChecked() == true)
            {
                sterowaniePlot->graph(3)->addData(m_time-1,m_symulacja->getPID()->oblicz_TiWSumie(setpoint,measured));
            }
            else
            {
                sterowaniePlot->graph(3)->addData(m_time-1,m_symulacja->getPID()->oblicz(setpoint,measured));
            }

            uchybPlot->graph(0)->addData(m_time-1, error);
            if(m_x > 100)
            {
                zadanaPlot->xAxis->setRange((m_x-100), m_x);
                sterowaniePlot->xAxis->setRange((m_x-100), m_x);
                uchybPlot->xAxis->setRange((m_x-100), m_x);
            }
            else
            {
                zadanaPlot->xAxis->setRange(0, m_x);
                sterowaniePlot->xAxis->setRange(0, m_x);
                uchybPlot->xAxis->setRange(0, m_x);
            }
            sterowaniePlot->yAxis->rescale();
            zadanaPlot->yAxis->rescale();
            uchybPlot->yAxis->rescale();
            zadanaPlot->replot();
            sterowaniePlot->replot();
            uchybPlot->replot();
        }
        m_time++;
        m_prevOutput = output;
        m_prevSetpoint = setpoint;
    } catch (const std::exception& ex) {
        //QMessageBox::critical(this, "Błąd symulacji", ex.what());
        stopSimulation();
    }
}

void MainWindow::zoom(bool stan)
{
    sterowaniePlot->setInteraction(QCP::iRangeZoom, stan);
    sterowaniePlot->setInteraction(QCP::iRangeDrag, stan);
    zadanaPlot->setInteraction(QCP::iRangeZoom, stan);
    zadanaPlot->setInteraction(QCP::iRangeDrag, stan);
    uchybPlot->setInteraction(QCP::iRangeZoom, stan);
    uchybPlot->setInteraction(QCP::iRangeDrag, stan);
}



void MainWindow::on_zmienARX_clicked()
{
   Dialog dialog(this);
   if (dialog.exec() == QDialog::Accepted)
    {
        ui->vecaLabel->setText(dialog.getWektorA());
        ui->vecbLabel->setText(dialog.getWektorB());
        ui->delayLabel->setText(QString::number(dialog.getDelay()));
        ui->zakloceniaLabel->setText(QString::number(dialog.getZaklocenia()));
        updateAllParams();
    }
}


void MainWindow::on_resetI_clicked()
{
    if(m_time != 0)
    {
        m_symulacja->getPID()->resetSuma();
    }

}


