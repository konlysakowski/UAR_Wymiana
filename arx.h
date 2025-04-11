#include <deque>
#include <vector>

#include <random>
class ARX {
    protected:
        std::deque<double> m_yi;
        std::vector<double> m_vec_a, m_vec_b;
        int m_delay;
        double m_zaklocenia;
        std::deque<double> m_ui;
        std::default_random_engine generator;
        std::normal_distribution<double> gzaklocen;
    
    public:
        ARX(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia = 0.0);
        double krok(double u);
        void setVektory(const std::vector<double>& vec_a, const std::vector<double>& vec_b);
        void setDelay(int delay);
        void setZaklocenia(double zaklocenia);
        void reset();
        std::pair<std::vector<double>, std::vector<double>> getVektory() const { return {m_vec_a, m_vec_b}; }
        int getDelay() const { return m_delay; }
        double getZaklocenia() const { return m_zaklocenia; }
        void updateParams(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia);
        void updateParamsWithoutReset(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia);
    };
