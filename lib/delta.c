#include "delta.h"

#include <errno.h>
#include <string.h>
#include <assert.h>

#include "leech.h"
#include "buffer.h"
#include "csv.h"

struct LCH_Delta {
    LCH_Dict *insertions;
    LCH_Dict *deletions;
    LCH_Dict *modifications;
};

LCH_Delta *LCH_DeltaCreate(const LCH_Dict *const new_state, const LCH_Dict *const old_state) {
    assert(new_state != NULL);
    assert(old_state != NULL);

    LCH_Delta *delta = malloc(sizeof(LCH_Delta));
    if (delta == NULL) {
        LCH_LOG_ERROR("Failed to allocate memory for delta: %s", strerror(errno));
        return NULL;
    }

    delta->insertions = LCH_DictSetMinus(new_state, old_state, (void *(*)(const void *))strdup);
    if (delta->insertions == NULL) {
        LCH_LOG_ERROR("Failed to compute insertions for delta.");
        free(delta);
        return NULL;
    }

    delta->deletions = LCH_DictSetMinus(old_state, new_state, (void *(*)(const void *))strdup);
    if (delta->deletions == NULL) {
        LCH_LOG_ERROR("Failed to compute deletions for delta.");
        LCH_DictDestroy(delta->insertions);
        free(delta);
        return NULL;
    }

    delta->modifications = LCH_DictSetChangedIntersection(new_state, old_state, (void *(*)(const void *))strdup, (int (*)(const void *, const void *))strcmp);
    if (delta->modifications == NULL) {
        LCH_LOG_ERROR("Failed to compute modifications for delta.");
        LCH_DictDestroy(delta->deletions);
        LCH_DictDestroy(delta->insertions);
        free(delta);
        return NULL;
    }

    return delta;
}

void LCH_DeltaDestroy(LCH_Delta *const delta) {
    if (delta != NULL) {
        assert(delta->insertions != NULL);
        LCH_DictDestroy(delta->insertions);

        assert(delta->deletions != NULL);
        LCH_DictDestroy(delta->deletions);

        assert(delta->modifications != NULL);
        LCH_DictDestroy(delta->modifications);

        free(delta);
    }
}

static bool MarshalDeltaOperation(const LCH_Buffer *const buffer, const LCH_Dict *const dict, const char symbol, const bool keep_value) {
    LCH_DictIter *iter = LCH_DictIterCreate(dict);
    if (iter == NULL) {
        LCH_LOG_ERROR("Failed to create iterator for marshaling delta.");
        LCH_BufferDestroy(buffer);
        return NULL;
    }

    while (LCH_DictIterNext(iter)) {
        const char *const key = LCH_DictIterGetKey(iter);
        assert(key != NULL);

        if (keep_value) {
            const char *const value = (char *)LCH_DictIterGetValue(iter);
            assert(value != NULL);

            if (!LCH_BufferAppend(buffer, "%c,%s,%s\r\n", symbol, key, value)) {
                LCH_LOG_ERROR("Failed to append data to buffer while marshaling delta.");
                free(iter);
                return false;
            }
            continue;
        }
        if (!LCH_BufferAppend(buffer, "%c,%s\r\n", symbol, key)) {
            LCH_LOG_ERROR("Failed to append data to buffer while marshaling delta.");
            free(iter);
            return false;
        }
    }

    free(iter);
    return true;
}

char *LCH_DeltaMarshal(const LCH_Delta *const delta) {
    assert(delta != NULL);
    assert(delta->insertions != NULL);
    assert(delta->deletions != NULL);
    assert(delta->modifications != NULL);

    LCH_Buffer *buffer = LCH_BufferCreate();
    if (buffer == NULL) {
        LCH_LOG_ERROR("Failed to create buffer for marshaling delta.");
        return NULL;
    }

    if (!MarshalDeltaOperation(buffer, delta->insertions, '+', true)) {
        LCH_LOG_ERROR("Failed to marshal delta insertions.");
        LCH_BufferDestroy(buffer);
        return NULL;
    }

    if (!MarshalDeltaOperation(buffer, delta->deletions, '-', false)) {
        LCH_LOG_ERROR("Failed to marshal delta deletions.");
        LCH_BufferDestroy(buffer);
        return NULL;
    }

    if (!MarshalDeltaOperation(buffer, delta->modifications, '%', true)) {
        LCH_LOG_ERROR("Failed to marshal delta modifications.");
        LCH_BufferDestroy(buffer);
        return NULL;
    }

    char *const str = LCH_BufferGet(buffer);
    if (str == NULL) {
        LCH_LOG_ERROR("Failed to get string from buffer for marshaling delta.");
    }
    LCH_BufferDestroy(buffer);

    return str;
}

LCH_Delta *LCH_DeltaUnmarshal(const char *const data) {
    return NULL;
}

size_t LCH_DeltaGetNumInsertions(const LCH_Delta *const delta) {
    assert(delta != NULL);
    assert(delta->insertions != NULL);
    return LCH_DictLength(delta->insertions);
}

size_t LCH_DeltaGetNumDeletions(const LCH_Delta *const delta) {
    assert(delta != NULL);
    assert(delta->deletions != NULL);
    return LCH_DictLength(delta->deletions);
}

size_t LCH_DeltaGetNumModifications(const LCH_Delta *const delta) {
    assert(delta != NULL);
    assert(delta->modifications != NULL);
    return LCH_DictLength(delta->modifications);
}

bool LCH_DeltaPatchTable(const LCH_Delta *const delta, LCH_Dict *const table) {
    assert(delta != NULL);
    assert(table != NULL);
    return false;
}
