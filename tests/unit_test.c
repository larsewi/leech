#include <assert.h>
#include <check.h>
#include <stdio.h>

#include "../lib/leech.h"

Suite *BlockSuite(void);
Suite *BufferSuite(void);
Suite *CSVSuite(void);
Suite *JSONSuite(void);
Suite *DeltaSuite(void);
Suite *DictSuite(void);
Suite *HeadSuite(void);
Suite *ListSuite(void);
Suite *TableSuite(void);
Suite *UtilsSuite(void);
Suite *InstanceSuite(void);
Suite *FilesSuite(void);
Suite *StringLibSuite(void);
Suite *PatchSuite(void);

int main(int argc, char *argv[]) {
  SRunner *sr = srunner_create(BufferSuite());
  srunner_add_suite(sr, DictSuite());
  srunner_add_suite(sr, ListSuite());
  srunner_add_suite(sr, CSVSuite());
  srunner_add_suite(sr, JSONSuite());
  srunner_add_suite(sr, UtilsSuite());
  srunner_add_suite(sr, DeltaSuite());
  srunner_add_suite(sr, BlockSuite());
  srunner_add_suite(sr, TableSuite());
  srunner_add_suite(sr, InstanceSuite());
  srunner_add_suite(sr, PatchSuite());

  if (argc > 1 && strcmp(argv[1], "no-fork") == 0) {
    srunner_set_fork_status(sr, CK_NOFORK);
  }

  srunner_run_all(sr, CK_VERBOSE);
  int number_failed = srunner_ntests_failed(sr);

  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
