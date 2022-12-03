#include "csv.h"

#include <assert.h>

#include "buffer.h"
#include "debug_messenger.h"

LCH_List *LCH_ParseCSV(const char *const str) { assert(str != NULL); }

static bool ComposeField(LCH_Buffer *const buffer,
                         const LCH_List *const table) {
    assert(buffer != NULL);
    assert(table != NULL);
}

static bool ComposeRecord(LCH_Buffer *const buffer,
                          const LCH_List *const record) {
    assert(buffer != NULL);
    assert(record != NULL);
}

LCH_Buffer *LCH_ComposeCSV(const LCH_List *const table) {
    assert(table != NULL);

    LCH_Buffer *buffer = LCH_BufferCreate();
    if (buffer == NULL) {
        LCH_LOG_ERROR("Failed to compose CSV");
        return NULL;
    }

    const size_t length = LCH_ListLength(table);
    for (size_t i = 0; i < length; i++) {
        if (i > 0) {
            if (!LCH_BufferAppend(buffer, "\r\n")) {
                LCH_LOG_ERROR("Failed to compose CSV");
                LCH_BufferDestroy(buffer);
                return NULL;
            }
        }

        LCH_List *record = LCH_ListGet(table, i);
        if (!ComposeRecord(buffer, record)) {
            LCH_LOG_ERROR("Failed to compose CSV");
            LCH_BufferDestroy(buffer);
            return NULL;
        }
    }

    return buffer;
}
