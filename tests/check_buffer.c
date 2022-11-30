#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "../leech/buffer.h"
#include "../leech/debug_messenger.h"

START_TEST(test_LCH_Buffer) { 
  ck_assert(1);
}
END_TEST

Suite *BufferSuite(void) {
  Suite *s = suite_create("buffer");
  TCase *tc = tcase_create("LCH_Buffer*");
  tcase_add_test(tc, test_LCH_Buffer);
  suite_add_tcase(s, tc);
  return s;
}
