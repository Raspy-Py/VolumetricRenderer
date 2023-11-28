#ifndef VULKANMEMORY_H
#define VULKANMEMORY_H
#include <memory>

// Super thin and naive move-only pointer wrapper
template <typename T>
class Ref {
private:
    T* Ptr;

public:
    Ref() : Ptr(nullptr) {}
    Ref(T* ptr) : Ptr(ptr) {}
    Ref(Ref&& other) noexcept : Ptr(other.Ptr) { other.Ptr = nullptr; }
    Ref& operator=(Ref&& other) noexcept {
        if (this != &other) {
            Ptr = other.Ptr;
            other.Ptr = nullptr;
        }
        return *this;
    }
    ~Ref() { delete Ptr; }

    T& Get() const {
        return *Ptr;
    }

    T& operator*() const {
        return *Ptr;
    }

    T* operator->() const {
        return Ptr;
    }
};

#endif //VULKANMEMORY_H
