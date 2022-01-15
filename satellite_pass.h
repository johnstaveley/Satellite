#ifndef SatellitePass_h
#define SatellitePass_h
#include "RTClib.h"
class SatellitePass {
	public:
    SatellitePass(DateTime startDate, DateTime endDate);
    bool isInRange(DateTime targetDate);
  private:
    DateTime _startDate;
    DateTime _endDate;
};
#endif
