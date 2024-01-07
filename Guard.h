//
// Created by sigsegv on 1/7/24.
//

#ifndef DRWHATSNOT_GUARD_H
#define DRWHATSNOT_GUARD_H

template <typename T> concept GuardFunc = requires (T func) {
    {func()};
};

template <GuardFunc Func> class Guard {
private:
    Func func;
public:
    Guard(Func func) : func(func) {}
    ~Guard() {
        func();
    }
};


#endif //DRWHATSNOT_GUARD_H
