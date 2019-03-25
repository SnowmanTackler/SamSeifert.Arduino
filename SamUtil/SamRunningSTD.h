#ifndef SamRunningSTD_h
#define SamRunningSTD_h

#include "Arduino.h"
#include "SamRunningAVG.h"

class SamRunningSTD : public SamRunningAVG
{
    public:
        SamRunningSTD(double alpha = 0);
        double AVG();
        double UpdateElapsed(double old_weight, double value) override;

    private:

    protected:
        double SumValues2 = 0;
};

#endif