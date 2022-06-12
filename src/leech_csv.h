#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include <leech.h>
#include <stdbool.h>

bool LCH_TableReadCallbackCSV(const LCH_Instance *instance,
                              const char *filename, char ****table);

bool LCH_TableWriteCallbackCSV(const LCH_Instance *instance,
                               const char *filename, char ****table);

#endif // _LEECH_CSV_H
