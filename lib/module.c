#include "module.h"

#include <stdlib.h>

#if HAVE_DLFCN_H
#include <dlfcn.h>
#elif defined(_WIN32)
#include <errhandlingapi.h>
#include <libloaderapi.h>
#endif

#include "logger.h"

void *LCH_ModuleLoad(const char *const path) {
  LCH_LOG_DEBUG("Loading dynamic shared library '%s'", path);
#if HAVE_DLFCN_H
  void *handle = dlopen(path, RTLD_NOW);
  if (handle == NULL) {
    LCH_LOG_ERROR("Failed to load dynamic shared library '%s': %s", path,
                  dlerror());
    return NULL;
  }
  return handle;
#elif defined(_WIN32)
  void *handle = LoadLibraryA(path);
  if (handle == NULL) {
    LCH_LOG_ERROR("Failed to load dynamic shared library '%s': Error code %lu",
                  path, GetLastError());
  }
  return handle;
#else
  LCH_LOG_ERROR("Failed to load dynamic shared library '%s'", path);
  return NULL;
#endif
}

void *LCH_ModuleGetSymbol(void *const handle, const char *const symbol) {
  LCH_LOG_DEBUG("Obtaining address of symbol '%s'", symbol);
#if HAVE_DLFCN_H
  void *address = dlsym(handle, symbol);
  if (address == NULL) {
    LCH_LOG_ERROR("Failed to obtain address of symbol '%s': %s", symbol,
                  dlerror());
    return NULL;
  }
  return address;
#elif defined(_WIN32)
  void *address = GetProcAddress(handle, symbol);
  if (address == NULL) {
    LCH_LOG_ERROR("Failed to obtain address of symbol '%s': Error code %lu",
                  symbol, GetLastError());
  }
  return address;
#else
  LCH_LOG_ERROR("Failed to obtain address of symbol '%s'", symbol);
  return NULL;
#endif
}

void LCH_ModuleDestroy(void *const handle) {
  if (handle == NULL) {
    return;
  }
#if HAVE_DLFCN_H
  if (dlclose(handle) != 0) {
    LCH_LOG_WARNING("Failed to release handle to dynamic shared library: %s",
                    dlerror());
  }
#elif defined(_WIN32)
  if (FreeLibrary(handle) == 0) {
    LCH_LOG_WARNING(
        "Failed to release handle to dynamic shared library: Error code %lu",
        GetLastError());
  }
#endif
}
