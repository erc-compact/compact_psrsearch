#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <new>
#include <algorithm>

template <typename T>
inline T *safeNew1D(unsigned long N, std::string error_position)
{
    try
    {
        T *data = new T[N];
        return data;
    }
    catch (std::bad_alloc &ba)
    {
        fprintf(stderr, "bad_alloc exception on allocating %d*%ld memory at this place: %s  ", N, sizeof(T), error_position.c_str());
        throw ba;
    }
}

template <typename T>
inline T *safe_new_2D(int dim1, int dim2, std::string error_position)
{
    try
    {
        T **data = new T *[dim1];
        for (int x = 0; x < dim1; x++)
            data[x] = new T[dim2];
        return data;
    }
    catch (std::bad_alloc &ba)
    {
        fprintf(stderr, "bad_alloc exception at allocating memory at this place: %s ", error_position.c_str());
        throw ba;
    }
}


template<typename K,typename V> const K& key(const std::pair<K, V>& keyValue)
{
    return keyValue.first;
}
template<typename K,typename V> const V& value(const std::pair<K, V>& keyValue)
{
    return keyValue.second;
}

template <typename K, typename V, typename C>
inline int split_map_into_vectors(std::map<K, V, C> *map, std::vector<K> *keys, std::vector<V> *values)
{
    std::transform(map->begin(), map->end(), keys->begin(), key<K, V>);
    std::transform(map->begin(), map->end(), values->begin(), value<K, V>);
    return EXIT_SUCCESS;
}
template <typename K, typename V>
inline int split_map_into_vectors(std::map<K, V> *map, std::vector<K> *keys, std::vector<V> *values)
{
    std::transform(map->begin(), map->end(), keys->begin(), key<K, V>);
    std::transform(map->begin(), map->end(), values->begin(), value<K, V>);
    return EXIT_SUCCESS;
}

template <typename K, typename V>
inline int get_key_vector_from_map(std::map<K, V> *map, std::vector<K> *keys)
{

    std::transform(map->begin(), map->end(), keys->begin(), key<K, V>);
    return EXIT_SUCCESS;
}

template <typename K, typename V>
inline void clone_and_cast(K *src, V *dest, int n)
{
    for (int i = 0; i < n; i++)
        dest[i] = (V)src[i];
}

int fileOpen(FILE **file, const std::string absolutename, const std::string mode);
bool flleExists(const std::string name);
int checkSize(unsigned long req, unsigned long got);
std::string removeWhiteSpace(const std::string& str);
bool caseInsensitiveCompare(const std::string& s1, const std::string& s2);