#include "leech.h"
#include "lch_utils.h"

void LCH_TestFunc(const LCH_Instance *instance) {
  LCH_LOG_DEBUG(instance, "This is a debug message");
  LCH_LOG_VERBOSE(instance, "This is a verbose message");
  LCH_LOG_INFO(instance, "This is a info message");
  LCH_LOG_WARNING(instance, "This is a warning message");
  LCH_LOG_ERROR(instance, "This is an error message");
}
