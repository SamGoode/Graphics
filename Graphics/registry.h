#pragma once

// this is cursed figure something else out

template<typename T>
struct Registry {
    static inline int count;
    static inline T* entries[32];

    int index;

    Registry() {
        if (count >= 32) { return; }

        index = count;
        entries[count] = static_cast<T*>(this);
        count++;
    }

    ~Registry() {
        if (count < 1) { return; }

        entries[index] = entries[count - 1];
        static_cast<Registry<T>*>(entries[index])->index = index;

        count--;
    }
};