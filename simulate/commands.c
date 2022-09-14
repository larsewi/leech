#include <string.h>

#include "commands.h"

typedef struct Command {
  char *key;
  bool (*func)(LCH_Instance *, const LCH_List *);
} Command;

static bool Bootstrap(LCH_Instance *const instance, const LCH_List *const args);
static bool Commit(LCH_Instance *const instance, const LCH_List *const args);
static bool Fetch(LCH_Instance *const instance, const LCH_List *const args);

static const Command commands[] = {
    { "bootstrap", Bootstrap },
    { "connect", Commit },
    { "fetch", Fetch },
};

bool ParseCommand(LCH_Instance *const instance, const char *const str) {
    LCH_List *args = LCH_SplitString(str, " \t\n");
    if (args == NULL) {
        LCH_ListDestroy(args);
        return false;
    }

    if (LCH_ListLength(args) < 1) {
        LCH_ListDestroy(args);
        return true;
    }

    char *command = LCH_ListGet(args, 0);
    if (command == NULL) {
        LCH_ListDestroy(args);
        return false;
    }

    int len = LCH_LENGTH(commands);
    for (int i = 0; i < len; i++) {
        if (strcmp(command, commands[i].key) == 0) {
            const bool ret = commands[i].func(instance, args);
            LCH_ListDestroy(args);
            return ret;
        }
    }

    LCH_LOG_ERROR("Bad command '%s'", command);
    return true;
}

static bool Bootstrap(LCH_Instance *const instance, const LCH_List *const args) {
    LCH_LOG_DEBUG("Bootstrap command called!");
    return true;
}

static bool Commit(LCH_Instance *const instance, const LCH_List *const args) {
    LCH_LOG_DEBUG("Commit command called!");
    return true;
}

static bool Fetch(LCH_Instance *const instance, const LCH_List *const args) {
    LCH_LOG_DEBUG("Fetch command called!");
    return true;
}
