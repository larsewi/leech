#include "table.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct LCH_Table {
    char *readLocator;
    LCH_List *(*readCallback)(const char *);
    char *writeLocator;
    bool (*writeCallback)(const char *, const LCH_List *);
} LCH_Table;

LCH_Table *LCH_TableCreate(LCH_TableCreateInfo *createInfo) {
    assert(createInfo != NULL);
    assert(createInfo->readLocator != NULL);
    assert(createInfo->readCallback != NULL);
    assert(createInfo->writeLocator != NULL);
    assert(createInfo->writeCallback != NULL);

    LCH_Table *table = (LCH_Table *)malloc(sizeof(LCH_Table));
    if (table == NULL) {
        return NULL;
    }

    table->readLocator = createInfo->readLocator;
    table->readCallback = createInfo->readCallback;
    table->writeLocator = createInfo->writeLocator;
    table->writeCallback = createInfo->writeCallback;

    return table;
}

void LCH_TableDestroy(LCH_Table *table) {
    if (table == NULL) {
        return;
    }
    free(table->readLocator);
    free(table->writeLocator);
    free(table);
}
