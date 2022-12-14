#pragma once

#include <random>

class PRNG {

public:

    static std::default_random_engine & get() {
        static PRNG instance;
        return instance.prng;
    }

    static void seed(unsigned seed) {
        get().seed(seed);
    }

private:
    PRNG() = default;
    std::default_random_engine prng;

public:
    PRNG(PRNG const &) = delete;
    void operator=(PRNG const &) = delete;
};