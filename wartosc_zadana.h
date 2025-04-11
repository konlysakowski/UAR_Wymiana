#pragma once
#include "typ_sygnalu.h"
class WartoscZadana {
    protected:
        TypSygnalu m_typsygnalu;
        double m_amplituda, m_okres, m_cykl;
        int m_aktywacja, m_czas;
    
    public:
        WartoscZadana();
        void ustawskok(double amplituda, int aktywacja);
        void ustawsinusoide(double amplituda, double okres);
        void ustawprostokatny(double amplituda, double okres, double cykl);
        double generuj();
        void reset();
        void updateParams(TypSygnalu typsygnalu, double amplituda, double okres, double cykl, int aktywacja);
    
    };
