#include <benchmark/benchmark.h>
#include <stdlib.h>  // srandom, random


/*
    set missing items
    set existing items
    lookup existing items
    lookup missing items
    delete missing items
    delete existing items
    set then delete
    set sequential missing items
    set random missing items
    set sequential existing/missing items
    set random existing/missing items
*/


static void BM_Set(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        for (int i = 0; i < state.range(0); ++i) {
            long value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        for (int i = 0; i < state.range(1); ++i) {
            long value = random();
            state.ResumeTiming();
            INSERT_INT_INTO_HASH(value, value);
            state.PauseTiming();
        }

        state.ResumeTiming();
    }
}


BENCHMARK(BM_Set)->RangeMultiplier(10)->Ranges({{1, 1000}, {1, 1000}});


static void BM_SetNew(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        for (int i = 0; i < state.range(0); ++i) {
            long value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        long missing_value;
        do {
            missing_value = random();
        } while (LOOKUP_INT_IN_HASH(missing_value));

        state.ResumeTiming();

        INSERT_INT_INTO_HASH(missing_value, missing_value);
    }
}


BENCHMARK(BM_SetNew)->RangeMultiplier(10)->Range(1, 1000);


static void BM_SetExisting(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        long value;
        for (int i = 0; i < state.range(0); ++i) {
            value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        state.ResumeTiming();

        INSERT_INT_INTO_HASH(value, value);
    }
}


BENCHMARK(BM_SetExisting)->RangeMultiplier(10)->Range(1, 1000);


static void BM_LookupMissing(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        for (int i = 0; i < state.range(0); ++i) {
            long value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        long missing_value;
        do {
            missing_value = random();
        } while (LOOKUP_INT_IN_HASH(missing_value));

        state.ResumeTiming();

        LOOKUP_INT_IN_HASH(missing_value);
    }
}


BENCHMARK(BM_LookupMissing)->RangeMultiplier(10)->Range(1, 1000);


static void BM_LookupExisting(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        long value;
        for (int i = 0; i < state.range(0); ++i) {
            value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        state.ResumeTiming();

        LOOKUP_INT_IN_HASH(value);
    }
}


BENCHMARK(BM_LookupExisting)->RangeMultiplier(10)->Range(1, 1000);


static void BM_DeleteMissing(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        for (int i = 0; i < state.range(0); ++i) {
            long value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        long missing_value;
        do {
            missing_value = random();
        } while (LOOKUP_INT_IN_HASH(missing_value));

        state.ResumeTiming();

        DELETE_INT_FROM_HASH(missing_value);
    }
}


BENCHMARK(BM_DeleteMissing)->RangeMultiplier(10)->Range(1, 1000);


static void BM_DeleteExisting(benchmark::State& state) {
    while (state.KeepRunning()) {
        state.PauseTiming();

        SETUP
        srandom(1);

        long value;
        for (int i = 0; i < state.range(0); ++i) {
            value = random();
            INSERT_INT_INTO_HASH(value, value);
        }

        state.ResumeTiming();

        DELETE_INT_FROM_HASH(value);
    }
}


BENCHMARK(BM_DeleteExisting)->RangeMultiplier(10)->Range(1, 1000);


BENCHMARK_MAIN()
