#ifndef SamRunningAVG_h
#define SamRunningAVG_h

#include "limits.h"

class SamRunningAVG
{
  public:
    SamRunningAVG(double alpha = 0);

    double Update(unsigned long time, double value);
  private:
    unsigned long last_time = ULONG_MAX;    
    double Alpha; // Set on initialization

  protected:
    virtual double UpdateElapsed(double old_weight, double value);
 
    double SumWeights = 0;
    double SumValues = 0;
};

#endif