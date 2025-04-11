#include "arx.h"
ARX::ARX(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia)
    : m_ui(std::deque<double>(vec_b.size() + delay, 0.0)), m_yi(std::deque<double>(vec_a.size(), 0.0)) {
    setVektory(vec_a, vec_b);
    setDelay(delay);
    m_zaklocenia = zaklocenia;
    if(zaklocenia > 0)
    {
        gzaklocen = std::normal_distribution<double>(0.0, zaklocenia);
    }

}

double ARX::krok(double u) {
    m_ui.push_back(u);
    if (m_ui.size() > m_vec_b.size() + m_delay) m_ui.pop_front();

    double wyjscie = 0.0;
    for (size_t i = 0; i < m_vec_b.size(); i++) {
        wyjscie += m_vec_b[i] * m_ui[m_ui.size() - 1 - i - m_delay];
    }
    for (size_t i = 0; i < m_vec_a.size(); i++) {
        wyjscie -= m_vec_a[i] * m_yi[m_yi.size() - 1 - i];
    }
    if (m_zaklocenia > 0.0) wyjscie += gzaklocen(generator);

    m_yi.push_back(wyjscie);
    if (m_yi.size() > m_vec_a.size()) m_yi.pop_front();

    return wyjscie;
}

void ARX::setVektory(const std::vector<double>& vec_a, const std::vector<double>& vec_b) {
    if (vec_a.empty() || vec_b.empty()) {
        throw std::invalid_argument("Wektory A i B nie mog� by� puste.");
    }
    m_vec_a = vec_a;
    m_vec_b = vec_b;
    reset();
}

void ARX::setDelay(int delay) {
    if (delay < 1) {
        throw std::invalid_argument("Op�nienie musi by� wi�ksze od 0.");
    }
    m_delay = delay;
}

void ARX::setZaklocenia(double zaklocenia) {
    if (zaklocenia < 0.0) {
        throw std::invalid_argument("Zak��cenia musz� by� wi�ksze lub r�wne 0.");
    }
    m_zaklocenia = zaklocenia;
    if(zaklocenia > 0)
    {
            gzaklocen = std::normal_distribution<double>(0.0, zaklocenia);
    }

}

void ARX::reset() {
    std::fill(m_ui.begin(), m_ui.end(), 0.0);
    std::fill(m_yi.begin(), m_yi.end(), 0.0);
}

void ARX::updateParams(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia) {
    if (vec_a.empty() || vec_b.empty()) {
        throw std::invalid_argument("Wektory A i B nie mog� by� puste.");
    }
    m_vec_a = vec_a;
    m_vec_b = vec_b;

    if (delay < 1) {
        throw std::invalid_argument("Op�nienie musi by� wi�ksze od 0.");
    }
    m_delay = delay;

    if (zaklocenia < 0.0) {
        throw std::invalid_argument("Zak��cenia musz� by� nieujemne.");
    }
    m_zaklocenia = zaklocenia;

    if (zaklocenia > 0.0) {
        gzaklocen = std::normal_distribution<double>(0.0, zaklocenia);
    }
}
void ARX::updateParamsWithoutReset(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia) {
    if (vec_a.empty() || vec_b.empty()) {
        throw std::invalid_argument("Wektory A i B nie mog� by� puste.");
    }
    m_vec_a = vec_a;
    m_vec_b = vec_b;

    if (delay < 1) {
        throw std::invalid_argument("Op�nienie musi by� wi�ksze od 0.");
    }
    m_delay = delay;

    if (zaklocenia < 0.0) {
        throw std::invalid_argument("Zak��cenia musz� by� nieujemne.");
    }
    m_zaklocenia = zaklocenia;

    if (zaklocenia > 0.0) {
        gzaklocen = std::normal_distribution<double>(0.0, zaklocenia);
    }
}
