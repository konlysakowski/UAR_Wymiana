#include "pid.h"
#include "arx.h"
#include "wartosc_zadana.h"
class Symulacja {
    protected:
        std::unique_ptr<ARX> m_ARX;
        std::unique_ptr<PID> m_PID;
        std::unique_ptr<WartoscZadana> m_WartoscZadana;
        double m_zadane, m_zmierzone;
    
    public:
        Symulacja(std::unique_ptr<ARX> arx, std::unique_ptr<PID> pid, std::unique_ptr<WartoscZadana> wartoscZadana);
        void setARX(std::unique_ptr<ARX> arx);
        void setPID(std::unique_ptr<PID> pid);
        void setWartoscZadana(std::unique_ptr<WartoscZadana> wartoscZadana);
        void aktualizujParametryARX(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia);
    
        void setZadane(double zadane);
        double krok();
        double krok_TiWSumie();
        void reset();
        ARX* getARX() const { return m_ARX.get(); }  
        PID* getPID() const { return m_PID.get(); }  
        WartoscZadana* getWartoscZadana() const { return m_WartoscZadana.get(); }
        void updateARXParams(const std::vector<double>& vec_a, const std::vector<double>& vec_b, int delay, double zaklocenia) {
            if (m_ARX) {
                m_ARX->updateParamsWithoutReset(vec_a, vec_b, delay, zaklocenia);
            }
        }
    
    };
