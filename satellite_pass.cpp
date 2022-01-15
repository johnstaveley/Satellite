#include "satellite_pass.h"
#include "RTClib.h"
SatellitePass::SatellitePass(DateTime startDate, DateTime endDate) {
    _startDate = startDate;
    _endDate = endDate;
}

bool SatellitePass::isInRange(DateTime targetDate) {
  return targetDate >= _startDate && targetDate <= _endDate;
}
