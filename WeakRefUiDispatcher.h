//
// Created by sigsegv on 1/5/24.
//

#ifndef DRWHATSNOT_WEAKREFUIDISPATCHER_H
#define DRWHATSNOT_WEAKREFUIDISPATCHER_H

#include <memory>
#include <functional>

template <class T> class WeakRefUiDispatcher;

template <class T> concept WeakRefUiDispatcherConcept = requires (T a)
{
    { a.GetObjectPointer() } -> std::convertible_to<void *>;
    { a.Invoke(std::declval<const std::function<void (typeof(a.GetObjectPointer()))>>()) };
};

template <class T> class WeakRefUiDispatcherRef {
private:
    std::weak_ptr<WeakRefUiDispatcher<T>> dispatcher;
public:
    WeakRefUiDispatcherRef(std::weak_ptr<WeakRefUiDispatcher<T>> dispatcher) : dispatcher(dispatcher) {}
private:
    template <WeakRefUiDispatcherConcept DispatcherType> static void InvokeSpecific(DispatcherType &dispatcher, const std::function<void (T *)> &func) {
        dispatcher.Invoke(func);
    }
public:
    void Invoke(const std::function<void (T *)> &func) const {
        auto lck = dispatcher.lock();
        if (lck) {
            InvokeSpecific<typeof(*lck)>(*lck, func);
        }
    }
};

template <class T> class WeakRefUiDispatcher : public std::enable_shared_from_this<WeakRefUiDispatcher<T>> {
private:
    T *ptr;
public:
    WeakRefUiDispatcher(T *ptr) : ptr(ptr) {}
    WeakRefUiDispatcherRef<T> GetRef() {
        std::shared_ptr<WeakRefUiDispatcher<T>> shptr = this->shared_from_this();
        return {std::weak_ptr<WeakRefUiDispatcher<T>>(shptr)};
    }
    T *GetObjectPointer() const {
        return ptr;
    }
    void Invoke(const std::function<void (T *)> &func) const {
        func(ptr);
    }
};

#endif //DRWHATSNOT_WEAKREFUIDISPATCHER_H
