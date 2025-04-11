class PID {
    protected:
        double m_kp, m_ti, m_td, m_suma, m_pop_blad;
    
    public:
        PID(double p, double i, double d);
        double obliczP(double cel, double zmierzone);
        double obliczI();
        double obliczD(double cel, double zmierzone);
        double oblicz(double cel, double zmierzone);
        double oblicz_TiWSumie(double cel, double zmierzone);
        void reset();
        void setParametry(double kp, double ti, double td);
        double getKp() const { return m_kp; }
        double getTi() const { return m_ti; }
        double getTd() const { return m_td; }
        void updateParamsWithoutReset(double kp, double ti, double td);
    
        void resetSuma();
    
        double obliczP_TiWSumie(double cel, double zmierzone);
        double obliczI_TiWSumie(double cel, double zmierzone);
    };