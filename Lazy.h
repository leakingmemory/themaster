//
// Created by sigsegv on 11/1/24.
//

#ifndef THEMASTER_LAZY_H
#define THEMASTER_LAZY_H

#include <utility>
#include <latch>
#include <mutex>

template <typename T> class LazyGeneratorContainer {
private:
    T generator;
public:
    typedef decltype(generator()) result_type;
    result_type result{};
private:
    bool generatorStarted{false};
public:
    constexpr LazyGeneratorContainer(T generatorF) : generator(generatorF) {}
    void Generate() {
        result = generator();
    }
};

template <typename T> concept LazyGeneratorFunc = requires (T func) {
    { LazyGeneratorContainer(func).Generate() };
};

class LazyLogic {
private:
    std::mutex mtx{};
    std::latch latch{1};
    bool generatorStarted{false};
public:
    constexpr LazyLogic() {}
    void Generate();
    virtual void Generator() = 0;
};

class SingleThreadedLazyLogic {
private:
    bool generatorStarted{false};
public:
    constexpr SingleThreadedLazyLogic() {}
    void Generate();
    virtual void Generator() = 0;
};

template <LazyGeneratorFunc T> class Lazy : private LazyLogic {
private:
    LazyGeneratorContainer<T> generator;
public:
    typedef LazyGeneratorContainer<T>::result_type result_type;
    constexpr Lazy(T func) : generator(func) {}
private:
    void Generator() override {
        generator.Generate();
    }
public:
    explicit operator result_type () {
        Generate();
        return generator.result;
    }
    template <typename V> operator V () {
        Generate();
        return generator.result;
    }
};

template <LazyGeneratorFunc T> class SingleThreadedLazy : private SingleThreadedLazyLogic {
private:
    LazyGeneratorContainer<T> generator;
public:
    typedef LazyGeneratorContainer<T>::result_type result_type;
    constexpr SingleThreadedLazy(T func) : generator(func) {}
private:
    void Generator() override {
        generator.Generate();
    }
public:
    explicit operator result_type () {
        Generate();
        return generator.result;
    }
    template <typename V> operator V () {
        Generate();
        return generator.result;
    }
};

#endif //THEMASTER_LAZY_H
