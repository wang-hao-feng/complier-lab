#include "map.h"
#include <stdlib.h>

Map *NewMap()
{
    Map *map = (Map *)malloc(sizeof(Map));
    map->length = 0;
    map->max_length = MIN_MAP_MAX_LENGTH;
    map->keys = (int *)malloc(sizeof(int) * map->max_length);
    map->values = (void **)malloc(sizeof(void *) * map->max_length);
}

void DeleteMap(Map *map)
{
    free(map->keys);
    free(map->values);
    free(map);
}

int AtMap(Map *map, int key)
{
    if(map->length == 0)
        return -1;
    int start = 0, end = map->length;
    while(start < end)
    {
        int mid = start + ((end - start) / 2);
        int temp = map->keys[mid];
        if(temp < key)
            start = mid + 1;
        else if(temp > key)
            end = mid;
        else
            return mid;
    }
    return -1;
}

void UpdateMap(Map *map)
{
    int *temp_keys = (int *)malloc(sizeof(int) * map->max_length);
    void **temp_values = (void **)malloc(sizeof(void *) * map->max_length);
    memcpy(temp_keys, map->keys, sizeof(int) * map->length);
    memcpy(temp_values, map->values, sizeof(void *) * map->length);
    free(map->keys);
    free(map->values);
    map->keys = temp_keys;
    map->values = temp_values;
}

void *SearchMapValue(Map *map, int key)
{
    int place = AtMap(map, key);
    if(place != -1)
        return map->values[place];
    return NULL;
}

void *InsertPair(Map *map, int key, void *value)
{
    //如果key已存在则覆盖，并返回被覆盖的值
    int place = AtMap(map, key);
    if(place != -1)
    {
        void *ans = map->values[place];
        map->values[place] = value;
        return ans;
    }

    //插入到满足单调递增的位置
    map->keys[map->length] = key;
    map->values[map->length] = value;
    place = map->length++;
    while(place != 0 && map->keys[place-1] > map->keys[place])
    {
        int a = map->keys[place];
        map->keys[place] = map->keys[place-1];
        map->keys[place-1] = a;
        void *b = map->values[place];
        map->values[place] = map->values[place-1];
        map->values[place-1] = b;
        place--;
    }

    //扩容
    if(map->length == map->max_length)
    {
        map->max_length <<= 1;
        UpdateMap(map);
    }

    return NULL;
}

void *DeletePair(Map *map, int key)
{
    //如果key不存在则返回NULL
    int place = AtMap(map, key);
    if(place == -1)
        return NULL;
    
    //删除
    while(place < map->length - 1)
    {
        int a = map->keys[place];
        map->keys[place] = map->keys[place+1];
        map->keys[place+1] = a;
        void *b = map->values[place];
        map->values[place] = map->values[place+1];
        map->values[place+1] = b;
        place++;
    }
    map->length--;
    void *ans = map->values[map->length];

    //缩容
    if(map->max_length > MIN_MAP_MAX_LENGTH && map->length < (map->max_length >> 2))
    {
        map->max_length >>= 1;
        UpdateMap(map);
    }

    return ans;
}