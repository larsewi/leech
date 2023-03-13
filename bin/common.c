#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/leech_csv.h"

void PrintVersion(void) { printf("%s\n", PACKAGE_STRING); }

void PrintOptions(const struct option *const options,
                  const char *const *const descriptions) {
  size_t longest = 0;
  for (int i = 0; options[i].val != 0; i++) {
    const size_t length = strlen(options[i].name);
    longest = (length > longest) ? length : longest;
  }

  char format[512];
  int ret = snprintf(format, sizeof(format), "  --%%-%zus  %%s\n", longest);
  assert(ret >= 0 && sizeof(format) > (size_t)ret);

  printf("options:\n");
  for (int i = 0; options[i].val != 0; i++) {
    printf(format, options[i].name, descriptions[i]);
  }
}

void PrintBugreport(void) {
  printf("Report bugs to <%s>.\n", PACKAGE_BUGREPORT);
  printf("%s home page: <%s>.\n", PACKAGE_NAME, PACKAGE_URL);
}

LCH_Instance *SetupInstance(const char *const work_dir) {
  assert(work_dir != NULL);

  LCH_Instance *instance = NULL;
  {  // Create instance
    LCH_InstanceCreateInfo createInfo = {
        .work_dir = work_dir,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      LCH_LOG_ERROR("LCH_InstanceCreate");
      return NULL;
    }
  }

  {  // Add classes table

    LCH_TableCreateInfo createInfo = {
        .identifier = "CLD",
        .primary_fields = "name",
        .subsidiary_fields = "meta",
        .read_locator = "classes.cache",
        .write_locator = "classes.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  {  // Add variables table
    LCH_TableCreateInfo createInfo = {
        .identifier = "VAD",
        .primary_fields = "namespace,bundle,name",
        .subsidiary_fields = "type,value,meta",
        .read_locator = "variables.cache",
        .write_locator = "variables.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  {  // Add last seen hosts table
    LCH_TableCreateInfo createInfo = {
        .identifier = "LSD",
        .primary_fields = "direction,hostkey",
        .subsidiary_fields = "address,interval,lastseen",
        .read_locator = "lastseen.cache",
        .write_locator = "lastseen.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  {  // Add installed packages table
    LCH_TableCreateInfo createInfo = {
        .identifier = "SDI",
        .primary_fields = "name,version,architecture",
        .subsidiary_fields = "",
        .read_locator = "software.cache",
        .write_locator = "software.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  {  // Add available package upgrade table
    LCH_TableCreateInfo createInfo = {
        .identifier = "SPD",
        .primary_fields = "name,version,architecture",
        .subsidiary_fields = "status",
        .read_locator = "patch.cache",
        .write_locator = "patch.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  {  // Add available package upgrade table
    LCH_TableCreateInfo createInfo = {
        .identifier = "ELD",
        .primary_fields = "promise_hash",
        .subsidiary_fields = "policy_filename,release_id,promise_outcome,namespace,bundle,promise_type,promiser,stack_path,handle,promisee,messages,line_number,policy_file_hash",
        .read_locator = "execution_log.cache",
        .write_locator = "execution_log.csv",
        .read_callback = LCH_TableReadCallbackCSV,
        .write_callback = LCH_TableWriteCallbackCSV,
        .insert_callback = LCH_TableInsertCallbackCSV,
        .delete_callback = LCH_TableDeleteCallbackCSV,
        .update_callback = LCH_TableUpdateCallbackCSV,
    };

    LCH_Table *table = LCH_TableCreate(&createInfo);
    if (table == NULL) {
      LCH_LOG_ERROR("LCH_TableCreate");
      return NULL;
    }

    if (!LCH_InstanceAddTable(instance, table)) {
      LCH_LOG_ERROR("LCH_InstanceAddTable");
      return NULL;
    }
  }

  return instance;
}
