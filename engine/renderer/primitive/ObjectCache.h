#pragma once

#include <unordered_map>

namespace Vision
{

using ID = std::size_t;

// simple non-threadsafe wrapper around std::unordered_map for storing render IDs
template <typename T>
class ObjectCache
{
public:
  ObjectCache() = default;
  ~ObjectCache() {
    Clear();
  }

  void Clear()
  {
    for (auto pair : cache)
    {
      delete pair.second;
    }
    cache.clear();
  }

  void Add(ID id, T* object) { cache.emplace(id, object); }
  bool Exists(ID id) { return cache.contains(id); }
  T* Get(ID id) { return cache.at(id); }
  void Destroy(ID id) { delete cache[id]; cache.erase(id); }

private:
  std::unordered_map<ID, T*> cache;
};

}