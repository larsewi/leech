#include "head.h"

#include <errno.h>
#include <string.h>

#include "leech.h"
#include "utils.h"

// char *LCH_HeadGet(const char *const work_dir) {
//   char path[PATH_MAX];
//   if (!LCH_PathJoin(path, sizeof(path), 2, work_dir, "HEAD")) {
//     return NULL;
//   }

//   FILE *file = fopen(path, "r");
//   if (file == NULL) {
//     LCH_LOG_ERROR("Failed to read file '%s': %s", path, strerror(errno));
//     return NULL;
//   }

//   size_t size;
//   if (!LCH_FileSize(file, &size)) {
//     LCH_LOG_ERROR("Failed to get size of file '%s'.", file);
//     fclose(file);
//     return NULL;
//   }
//   return NULL;
// }

// bool LCH_HeadSet(const char *const workdir, const char *const block_id) {
// return false; }
