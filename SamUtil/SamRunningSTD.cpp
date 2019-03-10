#include "Arduino.h"
#include "SamRunningAVG.h"
#include "SamRunningSTD.h"

SamRunningSTD::SamRunningSTD(double alpha) : SamRunningAVG(alpha)
{
}

double SamRunningSTD::AVG()
{
  return SumValues / SumWeights;    
}

double SamRunningSTD::UpdateElapsed(double old_weight, double value)
{
    // Upate base clase.
    double mean = SamRunningAVG::UpdateElapsed(old_weight, value);

    SumValues2 *= old_weight; // weighted mean
    SumValues2 += value * value;

    double variance = SumValues2 / SumWeights - mean * mean;
    
    if (variance < 0) // Double precision sometimes gives us negative variance
        variance = 0;

    return sqrt(variance);
}