//
// Created by sigsegv on 4/9/24.
//

#ifndef DRWHATSNOT_DEBOUNCER_H
#define DRWHATSNOT_DEBOUNCER_H

#include <functional>
#include <thread>
#include <semaphore>
#include <mutex>

class WxDebouncer {
private:
    std::function<void ()> apply{[] () {}};
    std::function<void ()> func;
    std::binary_semaphore sema{0};
    std::mutex mtx{};
    bool stop{false};
    bool scheduled{false};
    std::thread thread;
public:
    WxDebouncer() : func([] () {}), thread([this] () { Worker(); }) { }
    template <class T> void Func(std::function<T ()> func, std::function<void (const T &)> apply) {
        this->func = [this, func, apply] () {
            T result = func();
            this->apply = [apply, result] () {
                apply(result);
            };
            Apply();
        };
    }
private:
    void Worker();
    void Apply();
public:
    void Schedule();
    ~WxDebouncer();
    WxDebouncer(const WxDebouncer &) = delete;
    WxDebouncer(WxDebouncer &&) = delete;
    WxDebouncer &operator=(const WxDebouncer &) = delete;
    WxDebouncer &operator=(WxDebouncer &&) = delete;
};


#endif //DRWHATSNOT_DEBOUNCER_H
