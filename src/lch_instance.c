#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lch_instance.h"

struct LCH_Instance {
  char *instanceID;
  LCH_DebugMessenger *debugMessenger;
};

LCH_Instance *
LCH_InstanceCreate(const LCH_InstanceCreateInfo *const createInfo) {
  LCH_Instance *instance = malloc(sizeof(LCH_Instance));
  instance->instanceID = strdup(createInfo->instanceID);
  instance->debugMessenger = createInfo->debugMessenger;
}

void LCH_InstanceDestroy(LCH_Instance **instance) {
  if (*instance == NULL) {
    return;
  }
  free((*instance)->instanceID);
  free(*instance);
  *instance = NULL;
}

void LCH_InstanceLogMessage(const LCH_Instance *instance,
                            unsigned char severity, const char *format, ...) {
  assert(instance != NULL);
  va_list ap;
  va_start(ap, format);
  LCH_DebugMessengerLogMessage(instance->debugMessenger, severity, format, ap);
  va_end(ap);
}
