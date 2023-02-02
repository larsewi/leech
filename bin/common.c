#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/leech_csv.h"

#define WORK_DIR ".leech"
#define UNIQUE_ID "unique"

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

LCH_Instance *SetupInstance(void) {
  LCH_Instance *instance = NULL;
  {  // Create instance
    LCH_InstanceCreateInfo createInfo = {
        .identifier = UNIQUE_ID,
        .work_dir = WORK_DIR,
    };

    instance = LCH_InstanceCreate(&createInfo);
    if (instance == NULL) {
      LCH_LOG_ERROR("LCH_InstanceCreate");
      return NULL;
    }
  }

  {  // Add beatles.csv table
    LCH_TableCreateInfo createInfo = {
        .identifier = "beatles",
        .primary_fields = "lastname,firstname",
        .subsidiary_fields = "born,role",
        .read_locator = "beatles.csv",
        .write_locator = "beatles.dest.csv",
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

  // {  // Add prikfloyd.csv table
  //   LCH_TableCreateInfo createInfo = {
  //       .identifier = "pinkfloyd",
  //       .primary_fields = "id",
  //       .subsidiary_fields = "firstname,lastname,role",
  //       .read_locator = "pinkfloyd.csv",
  //       .write_locator = "pinkfloyd.dest.csv",
  //       .read_callback = LCH_TableReadCallbackCSV,
  //       .write_callback = LCH_TableWriteCallbackCSV,
  //   };

  //   LCH_Table *table = LCH_TableCreate(&createInfo);
  //   if (table == NULL) {
  //     LCH_LOG_ERROR("LCH_TableCreate");
  //     return NULL;
  //   }

  //   if (!LCH_InstanceAddTable(instance, table)) {
  //     LCH_LOG_ERROR("LCH_InstanceAddTable");
  //     return NULL;
  //   }
  // }

  return instance;
}
