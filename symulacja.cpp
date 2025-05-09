#include "symulacja.h"

Symulacja::Symulacja(std::unique_ptr<ARX> arx, std::unique_ptr<PID> pid, std::unique_ptr<WartoscZadana> wartoscZadana, QObject* parent)
    :QObject(parent), m_ARX(std::move(arx)), m_PID(std::move(pid)), m_WartoscZadana(std::move(wartoscZadana)), m_zadane(0.0), m_zmierzone(0.0) {}


void Symulacja::setARX(std::unique_ptr<ARX> arx) {
    m_ARX = std::move(arx);
}

void Symulacja::setPID(std::unique_ptr<PID> pid) {
    m_PID = std::move(pid);
}

void Symulacja::setWartoscZadana(std::unique_ptr<WartoscZadana> wartoscZadana) {
    m_WartoscZadana = std::move(wartoscZadana);
}

void Symulacja::setZadane(double zadane) {
    m_zadane = zadane;
}

double Symulacja::krok() {

    if(!m_trybSieciowy)
    {
        qDebug() << "Symulacja::krok() - tryb" << static_cast<int>(m_tryb);
        m_zadane = m_WartoscZadana->generuj();
        double u = m_PID->oblicz(m_zadane, m_zmierzone);
        m_zmierzone = m_ARX->krok(u);
        return m_zmierzone;
    }
    else
    {
        qDebug() << "Symulacja::krok() wywołany. m_tryb =" << static_cast<int>(m_tryb);

        if(m_tryb == TrybSymulacji::Regulator) {
            qDebug() << "Symulacja::krok() - tryb" << static_cast<int>(m_tryb);
            m_zadane = m_WartoscZadana->generuj();
            qDebug() << "Poszedł za m_zadane";
            double sygnalSterujacy = m_PID->oblicz(m_zadane, m_zmierzone);
            qDebug() << "Sygnał sterujący obliczony";
            if (m_Client && m_Client->isConnected()) {
                m_Client->sendValue(sygnalSterujacy);
                qDebug() << "Wartość wysłana";
                double odpowiedz;
                if (m_Client->receiveData(odpowiedz)) {
                    qDebug() << "Odebrano wartość regulowaną:" << odpowiedz;
                    m_zmierzone = odpowiedz;
                    emit statusKomunikacji(true);
                } else {
                     qDebug() << "Nie odebrano danych.";
                    emit statusKomunikacji(false);
                }
            } else {
                emit statusKomunikacji(false);
            }

            return m_zmierzone;
        }
    }


}

double Symulacja::krok_TiWSumie() {
    if (!m_WartoscZadana || !m_PID || !m_ARX) {
        throw std::logic_error("Symulacja nie zosta�a poprawnie zainicjalizowana.");
    }

    m_zadane = m_WartoscZadana->generuj();
    double sygnal = m_PID->oblicz_TiWSumie(m_zadane, m_zmierzone);
    m_zmierzone = m_ARX->krok(sygnal);
    return m_zmierzone;
}

void Symulacja::reset() {
    if (m_ARX) m_ARX->reset();
    if (m_PID) m_PID->reset();
    if (m_WartoscZadana) m_WartoscZadana->reset();
    m_zmierzone = 0.0;
}

void Symulacja::aktualizujParametryARX(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia) {
    if (m_ARX) {
        m_ARX->setVektory(vec_a, vec_b);
        m_ARX->setDelay(delay);
        m_ARX->setZaklocenia(zaklocenia);
    }
}

double Symulacja::przetworzSterowanie(float u)
{
    return m_ARX->krok(static_cast<double>(u));
}

