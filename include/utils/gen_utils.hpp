#pragma once
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <new>
#include <algorithm>
#include <memory>
#include "exceptions.hpp"

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


template <typename T>
void readFromFileAndVerify(FILE *file, std::size_t count, T *buffer) {
    std::size_t countRead = fread(buffer, sizeof(T), count, file);
    if (countRead != count) throw FileIOError(count, countRead, "read");
}

template <typename T>
void writeToFileAndVerify(FILE *file, std::size_t count, T *buffer) {
    std::size_t countRead = fwrite(buffer, sizeof(T), count, file);
    if (countRead != count) throw FileIOError(count, countRead, "write");
}

template <typename T>
void readFromFileStreamAndVerify(std::ifstream &file, std::size_t count, std::shared_ptr<std::vector<T>> buffer) {
    file.read(reinterpret_cast<char *>(buffer->data()), count * sizeof(T));
    if (file.gcount() != count * sizeof(T)) throw FileIOError(count, file.gcount() / sizeof(T), "read");
}

template <typename T>
void writeToFileStreamAndVerify(std::ofstream &file, std::size_t count, std::shared_ptr<std::vector<T>> buffer) {
    file.write(reinterpret_cast<char *>(buffer->data()), count * sizeof(T));
    if (file.fail()) throw FileIOError(count, 0, "write");
}




int fileOpen(FILE **file, const std::string absolutename, const std::string mode);
bool fileExists(const std::string name);
int checkSize(unsigned long req, unsigned long got);
bool caseInsensitiveCompare(const std::string& s1, const std::string& s2);
std::string removeWhiteSpace(const std::string& str);
std::string replaceExtension(const std::string& fileName, const std::string& newExtension);

