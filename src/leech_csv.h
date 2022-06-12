#ifndef _LEECH_CSV_H
#define _LEECH_CSV_H

#include <stdbool.h>

bool LCH_TableReadCallbackCSV(const char *filename, char ****table);

bool LCH_TableWriteCallbackCSV(const char *filename, char ****table);

#endif // _LEECH_CSV_H
