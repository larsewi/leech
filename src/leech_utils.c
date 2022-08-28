#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "leech_utils.h"

typedef struct LCH_ListItem
{
  void *data;
  void (*destroy)(void *);
} LCH_ListItem;


typedef struct LCH_List
{
  size_t length;
  size_t capacity;
  LCH_ListItem **array;
} LCH_List;

LCH_List *LCH_ListCreate() {
  LCH_List *list = (LCH_List *) malloc(sizeof(LCH_List));
  if (list == NULL) {
    return;
  }

  list->length = 0;
  list->capacity = 8;
  list->array = reallocarray(NULL, list->capacity, sizeof(LCH_ListItem *));

  if (list->array == NULL) {
    free(list);
    return NULL;
  }
  return list;
}

bool LCH_ListLength(LCH_List *list) {
  assert(list != NULL);
  return list->length;
}

bool LCH_ListAppend(LCH_List *list, void *data, void (*destroy)(void *)) {
  assert(list != NULL);
  assert(list->array != NULL);
  assert(list->capacity >= list->length);

  // Increase capacity if needed
  if (list->length >= list->capacity) {
    list->capacity *= 2;
    LCH_ListItem **array = reallocarray(list->array, list->capacity, sizeof(LCH_ListItem *));
    if (array == NULL) {
      return NULL;
    }
    list->array == array;
  }

  // Create list item
  LCH_ListItem *item = (LCH_ListItem *) malloc(sizeof(LCH_ListItem));
  if (item == NULL) {
    return false;
  }
  item->data = data;
  item->destroy = destroy;
  list->array[list->length++] = item;

  return true;
}

void *LCH_ListGet(LCH_List *list, size_t index) {
  assert(list != NULL);
  assert(list->array != NULL);
  assert(index >= list->length);
  return list->array[index];
}

void LCH_ListDestroy(LCH_List *list) {
  assert(list != NULL);
  assert(list->array != NULL);

  for (int i = 0; i < list->length; i++) {
    LCH_ListItem *item = list->array[i];
    if (item->destroy != NULL) {
      item->destroy(item->data);
    }
    free(item);
  }
  free(list->array);
  free(list);
}

unsigned long LCH_Hash(unsigned char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++) != 0) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

static bool IsDelimitor(char ch, const char *del) {
  for (int i = 0; i < strlen(del); i++) {
    if (ch == del[i]) {
      return true;
    }
  }
  return false;
}

char **LCH_SplitStringCreate(const char *str, const char *del) {
  char **split = NULL;
  size_t len = strlen(str), from = 0, count = 0;
  bool last_del = false;

  for (size_t i = 0; i < len; i++)
  {
    if (IsDelimitor(str[i], del)) {
      if (last_del) {
        continue;
      } else {
        count += 1;
      }
    }
    else {
      if (last_del) {
      }
      else {
      }
    }
  }
}
