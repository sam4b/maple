#pragma once
#include <cassert>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "MapleContext.hpp"
#include <type_traits>

class TypeErasedStorage {
public:
    virtual ~TypeErasedStorage() = default;
    virtual void fromJSON(const nlohmann::json& json, Systems context) noexcept = 0;
    virtual nlohmann::json toJSON() const noexcept = 0;
    virtual void destroyIfContains(uint32_t id) noexcept = 0;
};


template <typename T>
//requires T implemenets CompoonentMEtaata
class ECSStorage {
public:
    T& get(const uint32_t id) {
        assert(mapToIndex.contains(id));
        return data[mapToIndex.at(id)];
    }

    T& add(const uint32_t id) {
        assert(!mapToIndex.contains(id));
        data.emplace_back();


        mapToIndex[id] = data.size() - 1;

        ids.emplace_back(id);

        return data[data.size() - 1];
    }

    void remove(const uint32_t id) {
        assert(mapToIndex.contains(id));

        //remove from component storage
        auto last = data.back();

        const int idx = mapToIndex.at(id);

        data[idx] = last;

        data.pop_back();
        //remove entity owner

        ids[idx] = ids[ids.size() - 1];
        ids.pop_back();
        //mark empty in map

        mapToIndex.erase(id);
    }

    auto getData() -> std::vector<T>& {
        return data;
    }

    template <typename... Args>
    void Synchronise(const ECSStorage<std::tuple<Args...>>& joined) noexcept {
        //How can I static assert that T is in Args... ?
        //Static assert Args... is a set too
        //Static assert all of Args... are components

        for (auto [id, idx] : joined.mapToIndex)
        {
            data[mapToIndex.at(id)] = std::get<T>(joined.data.at(idx));
        }
    }

    const auto& getIDs() const
    {
        return ids;
    }

    bool contains(const uint32_t id) {
        return mapToIndex.contains(id);
    }

    std::unordered_map <uint32_t, uint32_t> mapToIndex; //Maps to component index
    std::vector<T> data;

    //Maps component index -> the entity owning it.
    std::vector<uint32_t> ids;
};

/*
 *  Internally used in EntityManager to wrap the ECSStorage with serialization capabilities (ECSStorage<T> is double-used for Query<T>, so you can't have methods on a tuple).
 */
template <typename T>
class StorageWrapper : public TypeErasedStorage {
public:
    ECSStorage<T>& get() noexcept {
        return storage;
    }

    ~StorageWrapper() override = default;
    void fromJSON(const nlohmann::json& json, Systems context) noexcept override {
        for (const auto& jsonObj : json) {
            const uint64_t id = jsonObj["id"];

            T c;
            c.FromJson(jsonObj["data"], context);

            this->storage.add(id) = c;

        }
    }


    nlohmann::json toJSON() const noexcept override {
        nlohmann::json out;

        if constexpr (!std::is_class_v<T>)
        {
            assert(false);
            return {};
        }

        for (const auto [id, idx] : this->storage.mapToIndex)
        {
            nlohmann::json j;
            j["id"] = id;
            j["data"] = this->storage.data[idx].ToJson();
            out.push_back(j);
        }

        return out;
    }

    void destroyIfContains(uint32_t id) noexcept override {
        if (this->storage.contains(id)) {
            this->storage.remove(id);
        }
    }
private:
    ECSStorage<T> storage;
};
