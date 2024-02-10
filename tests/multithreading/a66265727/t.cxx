#include <thread>
#include <atomic>
#include <cassert>

// size that's at least as big as a cache line
constexpr size_t cache_line_size = 256;

extern bool exchange_lrsc(std::atomic<bool> *varp, bool new_value_inp);

static void take_lock(std::atomic<bool> &me, std::atomic<bool> &other) {
    alignas(cache_line_size) bool uncached_true = true;
    for (;;) {
        // Evict uncached_true from cache.
        asm volatile("dc civac, %0" : : "r" (&uncached_true) : "memory");

        // So the release store to `me` may be delayed while
        // `uncached_true` is loaded.  This should give the machine
        // time to proceed with the load of `other`, which is not
        // forbidden by the release semantics of the store to `me`.

        (void) exchange_lrsc(&me, uncached_true);
        //- me.exchange(uncached_true, std::memory_order_seq_cst);
        if (other.load(std::memory_order_relaxed) == false) {
            // taken!
            std::atomic_thread_fence(std::memory_order_seq_cst);
            return;
        }
        // start over
        me.store(false, std::memory_order_seq_cst);
    }
}

static void drop_lock(std::atomic<bool> &me) {
    me.store(false, std::memory_order_seq_cst);
}

alignas(cache_line_size) std::atomic<int> counter{0};

static void critical_section(void) {
    // We should be the only thread inside here.
    int tmp = counter.fetch_add(1, std::memory_order_seq_cst);
    assert(tmp == 0);

    // Delay to give the other thread a chance to try the lock
    for (int i = 0; i < 100; i++)
        asm volatile("");

    tmp = counter.fetch_sub(1, std::memory_order_seq_cst);
    assert(tmp == 1);
}

static void busy(std::atomic<bool> *me, std::atomic<bool> *other)
{
    unsigned long max_attempts = 30000000ull;
    unsigned long i;
    for (i = 0; i < max_attempts; ++i) {
        take_lock(*me, *other);
        std::atomic_thread_fence(std::memory_order_seq_cst); // paranoia
        critical_section();
        std::atomic_thread_fence(std::memory_order_seq_cst); // paranoia
        drop_lock(*me);
    }
}


// The two flags need to be on separate cache lines.
alignas(cache_line_size) std::atomic<bool> flag1{false}, flag2{false};

int main()
{
    std::thread t1(busy, &flag1, &flag2);
    std::thread t2(busy, &flag2, &flag1);

    t1.join(); // will never happen
    t2.join();
    return 0;
}
