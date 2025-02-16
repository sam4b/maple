#pragma once
#include "ECSStorage.hpp"


/*
 * An ECS query object. Acts as a copy of the components in Args..., and once the system has finished operating on the components, the changes are merged with the real component storage.
 */
template <typename... Args>
//requires is set, are components
class Query {
public:
    Query(const std::unordered_set<uint64_t>& ids, const ECSStorage<Args>&... sets) {
        //assert map size is same

        /*64 bug is insane, need to get the min*/

        //If it's in one, it must be in all ovthers
        for (auto i : ids) {
            if ((sets.mapToIndex.contains(i) && ...)) {
                storage.data.emplace_back(std::make_tuple(sets.data[sets.mapToIndex.at(i)]...));
                storage.mapToIndex[i] = storage.data.size() - 1;
                storage.ids.push_back(i);
            }
        }
    }

    void PushChangesToOriginal(ECSStorage<Args>& ...sets) const noexcept {
        ((sets.Synchronise(storage)), ...);
    }

    std::vector<std::tuple<Args...>>& iterate() {
        return storage.data;
    }

    const std::vector<uint32_t>& getIDs() const noexcept {
        return storage.getIDs();
    }
private:
    ECSStorage<std::tuple<Args...>> storage;
};