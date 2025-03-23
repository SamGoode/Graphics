#pragma once

// this is cursed figure something else out

#define MAX_ENTRIES 64

template<typename T>
struct Registry {
    static inline const int maxCount = MAX_ENTRIES;
    static inline int count = 0;
    static inline T* entries[MAX_ENTRIES];

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