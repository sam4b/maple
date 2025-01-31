#pragma once

template <typename T>
struct internal_type_id {
   inline static int id = -1;
};

struct TypeIdentifier {
    template <typename T>
    inline static int get() {
        assert(internal_type_id<T>::id != -1); //Registered.
        return internal_type_id<T>::id;
    }

    template  <typename T>
    inline static void registerType() {
        assert(internal_type_id<T>::id == -1); //Unregistered.
        internal_type_id<T>::id = id++;
    }
private:
    static int id;
};
