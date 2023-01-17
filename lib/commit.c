#include "commit.h"

#include <assert.h>

#include "leech.h"
#include "buffer.h"
#include "csv.h"

LCH_Commit *LCH_CommitCreate(void) {
    LCH_Dict *const commit = LCH_DictCreate();
    return LCH_DictCreate();
}

void LCH_CommitDestroy(LCH_Commit *commit) {
    LCH_DictDestroy(commit);
}

bool LCH_CommitAddDelta(LCH_Commit *const commit, const char *const table_id, const char *const block_id) {
    assert(commit != NULL);
    assert(table_id != NULL);
    assert(block_id != NULL);
    assert(!LCH_DictHasKey(commit, table_id));

    if (!LCH_DictSet(commit, table_id, block_id, free)) {
        LCH_LOG_ERROR("Failed to add delta with for table '%s' and with block id '%s' to commit.", table_id, block_id);
        return false;
    }

    return true;
}

char *LCH_CommitMarshal(const LCH_Commit *const commit) {
    LCH_DictIter *iter = LCH_DictIterCreate(commit);
    if (iter == NULL) {
        LCH_LOG_ERROR("Failed to create iterator for marshaling commit.");
        return NULL;
    }

    LCH_Buffer *buffer = LCH_BufferCreate();
    if (buffer == NULL) {
        LCH_LOG_ERROR("Failed to create buffer for marshaling commit.");
        free(iter);
        return NULL;
    }

    while (LCH_DictIterNext(iter)) {
        const char *const key = LCH_DictIterGetKey(iter);
        assert(key != NULL);

        const char *const value = (char *) LCH_DictIterGetValue(iter);
        assert(value != NULL);

        char *const composed_key = LCH_CSVComposeField(key);
        if (composed_key == NULL) {
            LCH_LOG_ERROR("Failed to compose key for marshaling commit.");
            LCH_BufferDestroy(buffer);
            free(iter);
        }

        char *const composed_value = LCH_CSVComposeField(value);
    }

    free(iter);
}
