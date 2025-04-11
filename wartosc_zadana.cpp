#include "wartosc_zadana.h"
#include <cmath>
#include <stdexcept>
WartoscZadana::WartoscZadana() : m_typsygnalu(skok), m_amplituda(1.0), m_okres(1.0), m_cykl(0.5), m_aktywacja(0), m_czas(0) {}
using namespace std;
void WartoscZadana::ustawskok(double amplituda, int aktywacja) {
    m_typsygnalu = skok;
    m_amplituda = amplituda;
    m_aktywacja = aktywacja;
}

void WartoscZadana::ustawsinusoide(double amplituda, double okres) {
    if (amplituda < 0.0 || okres <= 0.0) {
        throw std::invalid_argument("Amplituda musi by� nieujemna, a okres wi�kszy od 0.");
    }
    m_typsygnalu = sinusoida;
    m_amplituda = amplituda;
    m_okres = okres;
}

void WartoscZadana::ustawprostokatny(double amplituda, double okres, double cykl) {
    if (amplituda < 0.0 || okres <= 0.0 || cykl < 0.0 || cykl > 1.0) {
        throw std::invalid_argument("Amplituda musi by� nieujemna, okres wi�kszy od 0, a cykl w przedziale [0, 1].");
    }
    m_typsygnalu = prostokatny;
    m_amplituda = amplituda;
    m_okres = okres;
    m_cykl = cykl;
}

double WartoscZadana::generuj() {

    double wartosc = 0.0;
    switch (m_typsygnalu) {
    case skok:
        if (m_czas >= m_aktywacja) wartosc = m_amplituda;
        break;
    case sinusoida:
        wartosc = m_amplituda * sin(2 * 3.14 * (m_czas % static_cast<int>(m_okres)) / m_okres);
        break;
    case prostokatny:
        if ((m_czas % static_cast<int>(m_okres)) < (m_cykl * m_okres)) {
            wartosc = m_amplituda;
        }
        break;
    }
    m_czas++;
    return wartosc;
}

void WartoscZadana::reset() {
    m_czas = 0;
}



void WartoscZadana::updateParams(TypSygnalu typsygnalu, double amplituda, double okres, double cykl, int aktywacja) {
    m_typsygnalu = typsygnalu;
    m_amplituda = amplituda;
    m_okres = okres;
    m_cykl = cykl;
    m_aktywacja = aktywacja;

}
