#include "Arduino.h"
#include "SamRunningAVG.h"

#define uint32_max ((uint32_t)-1)

SamRunningAVG::SamRunningAVG(double alpha)
{
  Alpha = alpha;
  last_time = uint32_max;
}

double SamRunningAVG::Update(uint32_t time, double value)
{
  uint32_t delta = 0;

  if (time < last_time) // Rollover of millis()
  {
    delta = uint32_max - last_time;
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

