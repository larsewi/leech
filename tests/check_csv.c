#include <check.h>
#include <stdlib.h>

#include "../leech/csv.h"
#include "../leech/debug_messenger.h"

START_TEST(Basic) { ck_assert(1); }
END_TEST

Suite *CSVSuite(void) {
    Suite *s = suite_create("csv");
    TCase *tc = tcase_create("Basic");
    tcase_add_test(tc, Basic);
    suite_add_tcase(s, tc);
    return s;
}
