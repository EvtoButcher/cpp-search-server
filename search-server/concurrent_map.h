#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <iterator>
#include <mutex>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket
    {
        std::mutex value_mutex;
        std::map<Key, Value> part_map;
    };

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket)
            : guard(bucket.value_mutex)
            , ref_to_value(bucket.part_map[key]) {
        }
    };

    explicit ConcurrentMap(size_t bucket_count) : maps_(bucket_count) {
    };

    Access operator[](const Key& key) {
        auto& bucket = maps_[static_cast<uint64_t>(key) % maps_.size()];

        return { key, bucket };
    };

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> r_map;
        for (auto& [mutex, map] : maps_) {
            std::lock_guard g(mutex);
            r_map.insert(map.begin(), map.end());
        }
        return r_map;
    };

    void erase(const Key& key) {
        maps_[static_cast<uint64_t>(key) % maps_.size()].part_map.erase(key);
    }

private:
    std::vector<Bucket>maps_;

};
