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

// static LCH_Instance *SetupInstance(void) {
//   LCH_Instance *instance = NULL;
//   {  // Create instance
//     LCH_InstanceCreateInfo createInfo = {
//         .instanceID = UNIQUE_ID,
//         .workDir = WORK_DIR,
//     };

//     instance = LCH_InstanceCreate(&createInfo);
//     if (instance == NULL) {
//       LCH_LOG_ERROR("LCH_InstanceCreate");
//       return NULL;
//     }
//   }

//   {  // Add CSV table
//     LCH_TableCreateInfo createInfo = {
//         .readLocator = READ_LOCATOR,
//         .readCallback = LCH_TableReadCallbackCSV,
//         .writeLocator = WRITE_LOCATOR,
//         .writeCallback = LCH_TableWriteCallbackCSV,
//     };

//     LCH_Table *table = LCH_TableCreate(&createInfo);
//     if (table == NULL) {
//       LCH_LOG_ERROR("LCH_TableCreate");
//       return NULL;
//     }

//     // TODO: Add table to instance
//     LCH_TableDestroy(table);
//   }

//   return instance;
// }
