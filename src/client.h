#ifndef _CLIENT_H
#define _CLIENT_H

#include <leech.h>
#include <stdbool.h>

bool Client(LCH_Instance *instance, const char *address, const char *port);

#endif // _CLIENT_H