#ifndef _MAP_H_
#define _MAP_H_

#define MIN_MAP_MAX_LENGTH 4

typedef struct Map
{
    int length;
    int max_length;
    int *keys;
    void **values;
}Map;

Map *NewMap();

void DeleteMap(Map *map);

int AtMap(Map *map, int key);

void *SearchMapValue(Map *map, int key);

void *InsertPair(Map *map, int key, void *value);

void *DeletePair(Map *map, int key);

#endif