#pragma once

#include <iostream>
#include <memory>

template <size_t N>
class alignas(max_align_t) StackStorage {
  private:
    char data[N];
    size_t shift = 0;

  public:
    StackStorage() = default;

    char* MakeAllignment(size_t sz, size_t align) {
        if ((shift % align) != 0u) {
            shift += align - (shift % align);
        }
        shift += sz;
        return data + shift - sz;
    }
};

template <typename T, size_t N>
class StackAllocator {
  private:
    StackStorage<N>* storage = nullptr;

  public:
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackStorage<N>* getStorage() const {
        return storage;
    }

    StackAllocator() = default;

    StackAllocator(StackStorage<N>& storage_)
        : storage(&storage_) {}

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other)
        : storage(other.getStorage()) {}

    template <typename U, size_t M>
    StackAllocator& operator=(StackAllocator<U, M> other) {
        storage = other.getStorage();
        return *this;
    }

    T* allocate(size_t n) {
        return reinterpret_cast<T*>(storage->MakeAllignment(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* /*unused*/, size_t /*unused*/) {}

    template <typename U, size_t M>
    bool operator==(const StackAllocator<U, M>& other) const {
        return storage == other.getStorage();
    }

    template <typename U, size_t M>
    bool operator!=(const StackAllocator<U, M>& other) const {
        return !(*this == other);
    }

    ~StackAllocator() = default;
};
