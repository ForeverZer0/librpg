#include "game.h"
#include "renderable.h"

void RPG_Batch_Init(RPGbatch *v) {
    v->capacity = BATCH_INIT_CAPACITY;
    v->total    = 0;
    v->items    = RPG_MALLOC(sizeof(void *) * BATCH_INIT_CAPACITY);
}

void RPG_Batch_Free(RPGbatch *v) {
    v->total = 0;
    RPG_FREE(v->items);
}

static void RPG_Batch_Resize(RPGbatch *v, int capacity) {
    RPGrenderable **items = RPG_REALLOC(v->items, sizeof(void *) * capacity);
    if (items) {
        v->items    = items;
        v->capacity = capacity;
    }
}

int RPG_Batch_Total(RPGbatch *v) { return v->total; }

void RPG_Batch_Add(RPGbatch *v, RPGrenderable *item) {
    if (v->capacity == v->total) {
        RPG_Batch_Resize(v, v->capacity * 2);
    }
    v->items[v->total++] = item;
    v->updated           = RPG_TRUE;
}

void RPG_Batch_Set(RPGbatch *v, int index, RPGrenderable *item) {
    if (index >= 0 && index < v->total) {
        v->items[index] = item;
        v->updated      = RPG_TRUE;
    }
}

RPGrenderable *RPG_Batch_Get(RPGbatch *v, int index) {
    if (index >= 0 && index < v->total) {
        return v->items[index];
    }
    return NULL;
}

void RPG_Batch_DeleteItem(RPGbatch *batch, RPGrenderable *item) {
    if (item == NULL) {
        return;
    }
    for (int i = 0; i < batch->total; i++) {
        if (batch->items[i] == item) {
            RPG_Batch_Delete(batch, i);
            break;
        }
    }
}

void RPG_Batch_Delete(RPGbatch *v, int index) {
    if (index < 0 || index >= v->total) {
        return;
    }

    v->items[index] = NULL;
    for (int i = index; i < v->total - 1; i++) {
        v->items[i]     = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total--;
    if (v->total > 0 && v->total == v->capacity / 4) {
        RPG_Batch_Resize(v, v->capacity / 2);
    }
}

static inline int RPG_Batch_MedianOfThree(int a, int b, int c) { return imax(imin(a, b), imin(imax(a, b), c)); }

void RPG_Batch_Sort(RPGbatch *v, int first, int last) {
    // Basic qsort algorithm using z-axis
    int i, j, pivot;
    RPGrenderable *temp;
    if (first < last) {
        pivot = ((last - first) / 2) + first;  // TODO: Use median?
        i     = first;
        j     = last;
        while (i < j) {
            while (v->items[i]->z <= v->items[pivot]->z && i < last) {
                i++;
            }
            while (v->items[j]->z > v->items[pivot]->z) {
                j--;
            }
            if (i < j) {
                temp        = v->items[i];
                v->items[i] = v->items[j];
                v->items[j] = temp;
            }
        }
        temp            = v->items[pivot];
        v->items[pivot] = v->items[j];
        v->items[j]     = temp;
        RPG_Batch_Sort(v, first, j - 1);
        RPG_Batch_Sort(v, j + 1, last);
    }
    v->updated = RPG_FALSE;
}