#ifndef UTIL_H_
#define UTIL_H_

#include <algorithm>
#include <vector>
#include <iostream>

namespace util {
    
template<typename T>
std::vector<T> intersect(const std::vector<T> &one, const std::vector<T> &two)
{
    std::vector<T> res;

    std::vector<T> copyOne = one;
    std::vector<T> copyTwo = two;
    std::sort(copyOne.begin(), copyOne.end());
    std::sort(copyTwo.begin(), copyTwo.end());
    std::set_intersection(copyOne.begin(), copyOne.end(), copyTwo.begin(),
        copyTwo.end(), std::back_inserter(res));
    
    return res;
}

template<typename T>
std::pair<std::vector<T>, std::vector<T> >
compareVectors(const std::vector<T> &pre, const std::vector<T> &current)
{
    std::vector<T> disconnected, connected;
    for (const auto &conn : pre) {
        if (std::find(current.begin(), current.end(), conn) == current.end()) {
            disconnected.push_back(conn);
        }
    }
    for (const auto &conn : current) {
        if (std::find(pre.begin(), pre.end(), conn) == pre.end()) {
            connected.push_back(conn);
        }
    }
    
    return std::make_pair(connected, disconnected);
}

template<typename Key, typename T>
std::map<Key, T> assignExclusive(const std::map<Key, std::vector<T> > &data)
{
    std::map<Key, T> res;
    
    std::vector<std::pair<Key, std::vector<T> > > dataAsVector;
    std::vector<T> used;
    for (const auto &entry : data) {
        dataAsVector.push_back(std::make_pair(entry.first, entry.second));
    }
    std::sort(dataAsVector.begin(), dataAsVector.end(),
              [](const std::pair<Key, std::vector<T> > &lhs,
                 const std::pair<Key, std::vector<T> > &rhs)
    {
        return lhs.second.size() < rhs.second.size();
    });
    for (const auto &dataPair : dataAsVector) {
        for (const auto &resource : dataPair.second) {
            if (std::find(used.begin(), used.end(), resource) == used.end()) {
                used.push_back(resource);
                res[dataPair.first] = resource;
                break;
            }
        }
    }
    
    return res;
}

template<typename T, typename Key>
std::map<Key, std::vector<T> > deleteResources(
    const std::map<Key, std::vector<T> > &data, const std::vector<T> &resources)
{
    std::map<Key, std::vector<T> > copy = data;
    std::vector<T> resCopy = resources;
    std::sort(resCopy.begin(), resCopy.end());
    for (auto &pair : copy) {
        pair.second.erase( std::remove_if(
            std::begin(pair.second), std::end(pair.second),
        [&](T x)
        { return std::binary_search(std::begin(resCopy), std::end(resCopy), x); }
        ), std::end(pair.second) );
    }
    
    return copy;
}

template<typename Key, typename T>
std::map<Key, std::vector<T> > keepOnlyKeys(
    const std::map<Key, std::vector<T> > &data, const std::vector<Key> &keys)
{
    std::map<Key, std::vector<T> > result;
    for (const auto &pair : data) {
        if (std::find(keys.begin(), keys.end(), pair.first) != keys.end()) {
            result[pair.first] = pair.second;
        }
    }
    
    return result;
}

} //util

#endif
