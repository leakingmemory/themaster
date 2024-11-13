//
// Created by sigsegv on 11/1/24.
//

#include "Lazy.h"

void LazyLogic::Generate() {
    bool waitOnLatch;
    {
        std::lock_guard lock{mtx};
        waitOnLatch = generatorStarted;
        if (!waitOnLatch) {
            generatorStarted = true;
        }
    }
    if (waitOnLatch) {
        latch.wait();
        return;
    }
    Generator();
    latch.count_down();
}

void SingleThreadedLazyLogic::Generate() {
    if (!generatorStarted) {
        generatorStarted = true;
        Generator();
    }
}
