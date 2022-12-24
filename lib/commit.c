#include "commit.h"

#include <assert.h>

#include "instance.h"

bool LCH_Commit(const LCH_Instance *const instance) {
  assert(instance != NULL);

    LCH_List *tables = LCH_InstanceGetTables(instance);
    size_t num_tables = LCH_ListLength(tables);

    for (size_t i = 0; i < num_tables; i++) {
        LCH_Table *table = LCH_ListGet(tables, i);
    }
}
