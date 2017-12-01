#include "Arduino.h"
#include "SamRunningAVG.h"

SamRunningAVG::SamRunningAVG(double alpha)
{
  Alpha = alpha;
}

double SamRunningAVG::Update(unsigned long time, double value)
{
  unsigned long delta = 0;

  if (time < last_time) // Rollover of millis()
  {
    delta = ULONG_MAX - last_time;
    delta += 1;
    delta += time;
  }
  else
  {
    delta = time - last_time;
  }

  last_time = time;

  double ti = delta / 1000.0;  
  return UpdateElapsed(exp(-ti * Alpha), value);
}

double SamRunningAVG::UpdateElapsed(double old_weight, double value)
{  
  SumValues *= old_weight; // weighted mean
  SumValues += value;
  
  SumWeights *= old_weight;
  SumWeights += 1;

  return SumValues / SumWeights;
}

