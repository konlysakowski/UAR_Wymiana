#include "pid.h"
#include <stdexcept>
PID::PID(double p, double i, double d) : m_kp(p), m_ti(i), m_td(d), m_suma(0.0), m_pop_blad(0.0){}

double PID::obliczP(double cel, double zmierzone)
{
    double blad = cel - zmierzone;
    m_suma += blad;
    return m_kp * blad;
}

double PID::obliczI()
{
    return (m_ti == 0.0) ? 0.0 : (m_suma / m_ti);
}
double PID::obliczP_TiWSumie(double cel, double zmierzone)
{
    double blad = cel - zmierzone;
    //m_suma += (blad* (1/m_ti));
    return m_kp * blad;
}
double PID::obliczI_TiWSumie(double cel, double zmierzone)
{
    double blad = cel - zmierzone;
    m_suma += (blad / m_ti);
    return (m_ti == 0.0) ? 0.0 : (m_suma);
}
double PID::obliczD(double cel, double zmierzone)
{
    double blad = cel - zmierzone;
    double D = m_td * (blad - m_pop_blad);
    m_pop_blad = blad;
    return D;
}

double PID::oblicz(double cel, double zmierzone)
{
    return obliczP(cel, zmierzone) + obliczI() + obliczD(cel, zmierzone);
}
double PID::oblicz_TiWSumie(double cel, double zmierzone)
{
    return obliczP_TiWSumie(cel, zmierzone) + obliczI_TiWSumie(cel, zmierzone) + obliczD(cel, zmierzone);
}

void PID::reset() {
    m_suma = 0.0;
    m_pop_blad = 0.0;
}

void PID::setParametry(double kp, double ti, double td) {
    if (kp < 0.0 || ti < 0.0 || td < 0.0) {
        throw std::invalid_argument("Parametry PID musz� by� nieujemne.");
    }
    m_kp = kp;
    m_ti = ti;
    m_td = td;
}

void PID::resetSuma()
{
    m_suma = 0;
}
void PID::updateParamsWithoutReset(double kp, double ti, double td) {
    if (kp < 0.0 || ti < 0.0 || td < 0.0) {
        throw std::invalid_argument("Parametry PID musz� by� nieujemne.");
    }
    m_kp = kp;
    m_ti = ti;
    m_td = td;
}
