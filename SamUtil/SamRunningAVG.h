#ifndef SamRunningAVG_h
#define SamRunningAVG_h

#include "limits.h"

class SamRunningAVG
{
  public:
    SamRunningAVG(double alpha = 0);

    double Update(uint32_t time, double value);
  private:
    uint32_t last_time;
    double Alpha; // Set on initialization

  protected:
    virtual double UpdateElapsed(double old_weight, double value);
 
    double SumWeights = 0;
    double SumValues = 0;
};

#endif