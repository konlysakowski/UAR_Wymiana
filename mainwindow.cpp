#include "mainwindow.h"
#include "dialog.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsTextItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_timer(new QTimer(this)),  m_time(0), m_prevSetpoint(0.0), m_prevOutput(0.0),
     m_client(new NetworkClient(this)), m_server(new NetworkServer(this)) {

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


    connect(m_client, &NetworkClient::connected, this, [this](const QString& ip) {
        aktualizujStatusPolaczenia(true, ip);

        QTcpSocket* socket = m_client->socket();
        if (socket)
        {
            connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        }
    });

    connect(m_client, &NetworkClient::disconnected, this, [this](const QString& ip) {
        aktualizujStatusPolaczenia(false, ip);
    });

    connect(m_server, &NetworkServer::clientConnected, this, [this](const QString& ip) {
        aktualizujStatusPolaczenia(true, ip);

        QTcpSocket* socket = m_server->clientSocket();
        if (socket)
        {
            connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        }
    });

    connect(m_server, &NetworkServer::clientDisconnected, this, [this](const QString& ip) {
        aktualizujStatusPolaczenia(false, ip);
    });



    connect(ui->startButton, &QPushButton::clicked, this, [=]() {
        if(m_client && m_client->isConnected())
            sendCommand("start");
    });

    connect(ui->stopButton, &QPushButton::clicked, this, [=]() {
        if(m_client && m_client->isConnected())
            sendCommand("stop");
    });

    connect(ui->resetButton, &QPushButton::clicked, this, [=]() {
        if(m_client && m_client->isConnected())
            sendCommand("reset");
    });

    connect(ui->trybTaktowaniacomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_trybTaktowaniacomboBox_currentIndexChanged);

    initSimulation();

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
    if(m_client != nullptr)
        sendValue(0x10, kp);
    double ti = ui->tiLabel->value();
    if(m_client != nullptr)
        sendValue(0x11, ti);
    double td = ui->tdLabel->value();
    if(m_client != nullptr)
        sendValue(0x12, td);

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
        if(m_client != nullptr)
        {
            sendValue(0x20, 0);
            sendValue(0x21, amplitude);
            sendValue(0x24, activationTime);
        }
        break;
    case sinusoida:
        wartoscZadana->ustawsinusoide(amplitude, period);
        if(m_client != nullptr)
        {
            sendValue(0x20, 1);
            sendValue(0x21, amplitude);
            sendValue(0x23, period);
        }
        break;
    case prostokatny:
        wartoscZadana->ustawprostokatny(amplitude, period, dutyCycle);
        if(m_client != nullptr)
        {
            sendValue(0x20, 2);
            sendValue(0x21, amplitude);
            sendValue(0x23, period);
            sendValue(0x22, dutyCycle);
        }
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
    } else {
        m_symulacja = std::make_unique<Symulacja>(std::move(arx), std::move(pid), std::move(wartoscZadana));
    }

    setupPlots();

    if(resetclicked)
    {
        resetclicked = false;
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

    try {
        if (!m_symulacja) {
            throw std::logic_error("Symulacja nie została poprawnie zainicjalizowana.");
        }


        if(ui->RoleComboBox->currentText() == "Regulator") {
            if (ui->trybTaktowaniacomboBox->currentIndex() == 0) {
                trybTaktowania = TrybTaktowania::Jednostronne;
            } else {
                trybTaktowania = TrybTaktowania::Obustronne;
            }
        }



        if (trybTaktowania == TrybTaktowania::Obustronne) {

            sendValue(0x32, ui->interwalSpinBox->value());

            sendValue(0x33, m_symulacja->getNumerProbki());

            sendValue(0x30, 1);

            m_timer->start(ui->interwalSpinBox->value());

            qDebug() << "Symulacja uruchomiona (obustronne taktowanie)";
        }
        else {
            zoom(false);
           // sendValue(0x30, 0);
            m_timer->start(ui->interwalSpinBox->value());
            qDebug() << "Symulacja uruchomiona (jednostronne taktowanie)";
        }

    } catch (const std::exception) {
        //QMessageBox::critical(this, "Błąd", ex.what());
    }

}

void MainWindow::stopSimulation() {
    if (trybTaktowania == TrybTaktowania::Obustronne) {
        sendValue(0x31, 0); // zatrzymaj po stronie ARX
        m_timer->stop();
    } else {
        m_timer->stop();
    }
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
    m_time = 0;
    m_symulacja->ustawNumerProbki(0);
}

void MainWindow::updateAllParams(bool restart) {
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

                if(restart)
                {
                    m_timer->start(ui->interwalSpinBox->value());
                }
            }



            else {
            m_symulacja = std::make_unique<Symulacja>(std::move(arx), std::move(pid), std::move(wartoscZadana));
            m_timer->start(ui->interwalSpinBox->value());
            }

        }
    } catch (const std::exception) {
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
        double dComponent = m_symulacja->getPID()->obliczD(setpoint, measured);
        double sterowanie;

        if (trybTaktowania == TrybTaktowania::Obustronne) {
            double u = m_symulacja->getPID()->oblicz(setpoint, measured);
            double y = measured;

            m_symulacja->inkrementujNumerProbki();
            sendValue(0x33, static_cast<double>(m_symulacja->getNumerProbki()));
            sendValue(0x01, y);
            sendValue(0x02, u);
        }

        if(m_client->isConnected() && ui->RoleComboBox->currentText() == "Model ARX")
            sendValue(0x01, measured);

        if(ui->liczenieCalkiLabel->isChecked() == true)
        {
             pComponent = m_symulacja->getPID()->obliczP_TiWSumie(setpoint, measured);
             iComponent = m_symulacja->getPID()->obliczI_TiWSumie(setpoint, measured);
             sterowanie = m_symulacja->getPID()->oblicz_TiWSumie(setpoint, measured);
        }
        else
        {
             pComponent = m_symulacja->getPID()->obliczP(setpoint, measured);
             iComponent = m_symulacja->getPID()->obliczI();
             sterowanie = m_symulacja->getPID()->oblicz(setpoint, measured);
        }

        if(m_client->isConnected() && ui->RoleComboBox->currentText() == "Regulator")
            sendValue(0x02, sterowanie);


        m_symulacja->setZadane(setpoint);
        double output = 0.0;

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


    } catch (const std::exception) {
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



void MainWindow::on_networkModeCheckBox_stateChanged(int state)
{

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Zmiana trybu sieciowego", "Czy na pewno chcesz zmienić tryb pracy sieciowej?", QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        ui->networkModeCheckBox->blockSignals(true);
        ui->networkModeCheckBox->setChecked(!ui->networkModeCheckBox->isChecked());
        ui->networkModeCheckBox->blockSignals(false);
        return;
    }

    bool sieciowy = state == Qt::Checked;
    QString rola = ui->RoleComboBox->currentText();
    QString ip = ui->ipLineEdit->text();

    if (sieciowy)
    {
        if (rola == "Regulator")
        {
            m_client->connectToServer(ip, 1234);
        }
        else if (rola == "Model ARX")
        {
            m_server->startListening(1234);
        }

    }
    else
    {
        if (rola == "Regulator")
        {
            m_client->disconnectFromHost();
        }
        else if (rola == "Model ARX")
        {
            m_server->stopListening();
        }

        aktualizujStatusPolaczenia(false, ip);
    }

    blokujGUIWDanymTrybie(sieciowy);
}

void MainWindow::blokujGUIWDanymTrybie(bool sieciowy)
{

    if(ui->RoleComboBox->currentText() == "Regulator")
    {
            ui->zmienARX->setDisabled(sieciowy);
            ui->vecaLabel->setDisabled(sieciowy);
            ui->vecbLabel->setDisabled(sieciowy);
            ui->delayLabel->setDisabled(sieciowy);
            ui->zakloceniaLabel->setDisabled(sieciowy);
    }
    else if(ui->RoleComboBox->currentText() == "Model ARX")
    {
            ui->kpLabel->setDisabled(sieciowy);
            ui->tiLabel->setDisabled(sieciowy);
            ui->tdLabel->setDisabled(sieciowy);
            ui->liczenieCalkiLabel->setDisabled(sieciowy);
            ui->resetI->setDisabled(sieciowy);
            ui->amplitudaLabel->setDisabled(sieciowy);
            ui->okresLabel->setDisabled(sieciowy);
            ui->sygnalcomboBox->setDisabled(sieciowy);
            ui->cyklLabel->setDisabled(sieciowy);
            ui->aktywacjaLabel->setDisabled(sieciowy);
            ui->interwalSpinBox->setDisabled(sieciowy);
            ui->startButton->setDisabled(sieciowy);
            ui->stopButton->setDisabled(sieciowy);
            ui->resetButton->setDisabled(sieciowy);
            ui->aktualizujButton->setDisabled(sieciowy);
            ui->trybTaktowaniacomboBox->setDisabled(sieciowy);

    }
}

void MainWindow::aktualizujStatusPolaczenia(bool connected, const QString& ip)
{
    if (connected) {
        if(ui->RoleComboBox->currentText() == "Model ARX")
        {
            ui->connectionStatusLabel->setText("Połączono");
            ui->connectionStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        }
        ui->connectionStatusLabel->setText("Połączono z: " + ip);
        ui->connectionStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    } else {
        ui->connectionStatusLabel->setText("Brak połączenia");
        ui->connectionStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
}


void MainWindow::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !socket->isOpen()) {
        qDebug() << "onReadyRead: brak socketu lub socket nie jest otwarty";
        return;
    }

    static QByteArray bufor;

    bufor = bufor + socket->readAll();

    while(bufor.size() >= 9)
    {
        quint8 type = static_cast<quint8>(bufor[0]);
        double value;
        memcpy(&value, bufor.constData() + 1, sizeof(double));
        bufor.remove(0, 9);

        qDebug() << "Odebrano: " << type << " " << value;


        switch (static_cast<int>(type)) {
        case 0x00: { //komenda start/stop/reset
            int command = static_cast<int>(value);
            if(command == 0)
                startSimulation();
            else if(command == 1)
                stopSimulation();
            else if(command == 2)
                resetSimulation();
            else
                qDebug() << "Błąd! Nie ma takiej komendy";

            break;
        }
        case 0x01:  //wartosc regulowana
            qDebug() << "Wartość regulowana (W):" << value;
            m_symulacja->setZmierzone(value);
            break;
        case 0x02: //sterowanie PID
            qDebug() << "Sterowanie (S):" << value;
            m_symulacja->setSterowanie(value);
            break;
        case 0x10: //kp
            qDebug() << "Kp:" << value;
            ui->kpLabel->setValue(value);
            break;
        case 0x11: //ti
            qDebug() << "Ti:" << value;
            ui->tiLabel->setValue(value);
            break;
        case 0x12: //td
            qDebug() << "Td:" << value;
            ui->tdLabel->setValue(value);
            break;
        case 0x20: //typ sygnalu
            qDebug() << "Typ sygnału:" << value;
            ui->sygnalcomboBox->setCurrentIndex(static_cast<int>(value));
            break;
        case 0x21: //amplituda
            qDebug() << "Amplituda:" << value;
            ui->amplitudaLabel->setValue(value);
            break;
        case 0x22: //cykl
            qDebug() << "Cykl:" << value;
            ui->cyklLabel->setValue(value);
            break;
        case 0x23: //okres
            qDebug() << "Okres:" << value;
            ui->okresLabel->setValue(value);
            break;
        case 0x24: //czas aktywacji
            qDebug() << "Czas aktywacji:" << value;
            ui->aktywacjaLabel->setValue(value);
            break;
        case 0x30: {// start obustronne
            if(trybTaktowania == TrybTaktowania::Obustronne)
            {
                m_timer->start();
            }
            break;
        }
        case 0x31: // stop obustronne
            m_timer->stop();
            break;
        case 0x32: // interwał obustronne
            m_timer->setInterval(static_cast<int>(value));
            break;
        case 0x33: {// numer próbki
            int zdalnyNumer = static_cast<int>(value);
            int lokalnyNumer = m_symulacja->getNumerProbki();

            aktualizujStatusSynchronizacji(lokalnyNumer, zdalnyNumer);

            if (zdalnyNumer != lokalnyNumer) {
                qWarning() << "Niesynchronizowane próbki!"
                           << "Lokalna:" << lokalnyNumer
                           << "Zdalna:" << zdalnyNumer;
            } else {
                qDebug() << "Próbki zsynchronizowane (nr" << lokalnyNumer << ")";
            }
            break;
        }
        case 0x34: //tryb taktowania
        {
            int tryb = static_cast<int>(value);
            trybTaktowania = (tryb == 0) ? TrybTaktowania::Jednostronne : TrybTaktowania::Obustronne;
            ui->trybTaktowaniacomboBox->setCurrentIndex(tryb);
            if(trybTaktowania == TrybTaktowania::Jednostronne)
                ui->syncStatusLabel->setText(" ");
            qDebug() << "Model ARX: otrzymano tryb:" << tryb;
            break;
        }
        default:
            qDebug() << "Nieznany typ wiadomości:" << type;
            break;
        }

        if(type >= 0x10 && type <= 0x24)
            updateAllParams(false);
    }

}

void MainWindow::sendValue(float type, double value)
{
    QByteArray msg;

    char buf[sizeof(double)];
    memcpy(buf, &value, sizeof(double));

    msg.append(type);
    msg.append(buf, sizeof(double));

    m_client->socket()->write(msg);
    m_client->socket()->flush();

}

void MainWindow::sendCommand(QString cmd)
{
    if(cmd == "start")
        sendValue(0x00, 0);
    else if(cmd == "stop")
        sendValue(0x00, 1);
    else if(cmd == "reset")
        sendValue(0x00, 2);
}

double MainWindow::receiveValue()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    if (!socket || !socket->bytesAvailable())
        return 0;


    return socket->readAll().toDouble();

}

void MainWindow::aktualizujStatusSynchronizacji(int lokalny, int zdalny)
{
    int roznica = std::abs(lokalny - zdalny);

    QString tekst = QString("Synchronizacja: %1 (lokalna: %2, zdalna: %3)")
                        .arg(roznica == 0 ? "OK" : roznica <= 5 ? "OPÓŹNIENIE" : "BŁĄD")
                        .arg(lokalny)
                        .arg(zdalny);

    QString kolor = roznica == 0 ? "green" :
                        roznica <= 5 ? "orange" :
                        "red";

    ui->syncStatusLabel->setText(tekst);
    ui->syncStatusLabel->setStyleSheet(QString(
                                           "QLabel { font-weight: bold; color: %1; }").arg(kolor));

    if(trybTaktowania == TrybTaktowania::Jednostronne)
        ui->syncStatusLabel->setText(" ");
}

void MainWindow::on_trybTaktowaniacomboBox_currentIndexChanged(int index)
{
    if (ui->RoleComboBox->currentText() != "Regulator")
        return;

    trybTaktowania = (index == 0) ? TrybTaktowania::Jednostronne
                                  : TrybTaktowania::Obustronne;

    sendValue(0x34, index);

    if(index == 0){
        ui->syncStatusLabel->setText(" ");
    }
}

