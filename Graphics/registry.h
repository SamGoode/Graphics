#pragma once


template<typename T>
struct registry {
    static inline int count;
    static inline T* entries[32];

    int index;

    registry() {
        if (count >= 32) { return; }

        index = count;
        entries[count] = static_cast<T*>(this);
        count++;
    }

    ~registry() {
        if (count < 1) { return; }

        entries[index] = entries[count - 1];
        count--;
    }
};