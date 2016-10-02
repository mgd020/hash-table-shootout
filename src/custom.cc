#include <cinttypes>


#include <utility> // swap, pair
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free
#include <stdexcept> // out_of_range
#include <strings.h> // bzero
#include <cassert> // assert


#include <ctime>
#include <iostream>


template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K> >
class Custom {
public:

    typedef std::pair<K, V> value_type;

    explicit Custom(size_t capacity=4):
        _capacity(capacity),
        _size(0) {
        assert(capacity);
        alloc();
    }

    ~Custom() {
        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i]) {
                destruct(_kv[i]);
            }
        }
        free(_h);
        free(_kv);
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _capacity;
    }

    bool empty() const {
        return !_size;
    }

    V & get(const K & k) {
        if (!_size) {
            throw std::out_of_range("");
        }

        size_t h = hash_key(k);
        size_t i = bucket(h);
        size_t dist = 0;

        while (true) {
            size_t hash_i = _h[i];

            if (!hash_i) {
                throw std::out_of_range("");
            }

            if (hash_i == h) {
                value_type & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    return kv.second;
                }
            } else {
                size_t dist_i = probe_distance(hash_i, i);
                if (dist > dist_i) {
                    throw std::out_of_range("");
                }
            }

            i = (i + 1) % _capacity;
            ++dist;
        }
    }

    inline const V & get(const K & k) const {
        return const_cast<Custom *>(this)->get(k);
    }

    void set(value_type && kv) {
        _set(hash_key(kv.first), std::move(kv));
    }

    void del(const K & k) {
        if (!_size) {
            return;
        }

        size_t h = hash_key(k);
        size_t i = bucket(h);
        size_t dist = 0;

        while (true) {
            size_t hash_i = _h[i];

            if (!hash_i) {
                return;
            }

            if (hash_i == h) {
                value_type & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    destruct(kv);
                    _h[i] = 0;
                    --_size;
                    break;
                }
            } else {
                size_t dist_i = probe_distance(hash_i, i);
                if (dist > dist_i) {
                    return;
                }
            }

            i = (i + 1) % _capacity;
            ++dist;
        }

        if (_size < _shrink) {
            rehash(_capacity / 2);
        } else {
            while (true) {
                i = (i + 1) % _capacity;

                size_t hash_i = _h[i];
                if (!hash_i) {
                    break;
                }

                size_t dist_i = probe_distance(hash_i, i);
                if (!dist_i) {
                    break;
                }

                std::swap(_h[i], _h[i - 1]);
                std::swap(_kv[i], _kv[i - 1]);
            }
        }
    }

    double load_factor() const {
        return 1.0 * _size / _capacity;
    }

// private:

    void rehash(size_t new_capacity) {
        auto old_capacity = _capacity;
        auto h = _h;
        auto kv = _kv;

        _capacity = new_capacity;
        _size = 0;
        alloc();

        for (size_t i = 0; i < old_capacity; ++i) {
            if (h[i]) {
                _set(h[i], std::move(kv[i]));
            }
        }

        free(h);
        free(kv);
    }

    void alloc() {
        _h = (size_t *)malloc(sizeof(size_t) * _capacity);
        _kv = (value_type *)malloc(sizeof(value_type) * _capacity);
        bzero(_h, sizeof(size_t) * _capacity);
        _grow = 75 * _capacity / 100;
        _shrink = 75 * _capacity / 400;
    }

    void _set(size_t h, value_type && kv) {
        size_t i = bucket(h);
        size_t dist = 0;

        if (_size >= _grow) {
            rehash(_capacity * 2);
        }

        while (true) {
            size_t hash_i = _h[i];

            if (!hash_i) {
                new (&_kv[i]) value_type(kv);
                _h[i] = h;
                ++_size;
                return;
            }

            if (hash_i == h) {
                value_type & kv_i = _kv[i];
                if (keys_equal(kv.first, kv_i.first)) {
                    kv_i.second = std::move(kv.second);
                    return;
                }
            } else {
                size_t dist_i = probe_distance(hash_i, i);
                if (dist > dist_i) {
                    std::swap(_h[i], h);
                    std::swap(_kv[i], kv);
                    dist = dist_i;
                }
            }

            i = (i + 1) % _capacity;
            ++dist;
        }
    }

    inline static size_t hash_key(const K & k) {
        static H h;
        size_t hk = h(k);
        return hk ?: -1; // 0 reserved for empty
    }

    inline size_t bucket(size_t h) {
        return h % _capacity;
    }

    inline size_t probe_distance(size_t h, size_t i) {
        return (i + _capacity - bucket(h)) % _capacity;
    }

    inline static bool keys_equal(const K & k1, const K & k2) {
        static P p;
        return p(k1, k2);
    }

    template <class T>
    inline void destruct(T & t) {
        t.~T();
    }

    size_t *_h;
    value_type *_kv;
    size_t _capacity; // length of arrays
    size_t _size; // number of items stored
    size_t _grow;
    size_t _shrink;
};


// using namespace std;
typedef Custom<int64_t, int64_t> hash_t;
typedef Custom<const char *, int64_t> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.set(std::make_pair(key, value))
#define DELETE_INT_FROM_HASH(key) try { hash.del(key); } catch (std::out_of_range &) {}
#define INSERT_STR_INTO_HASH(key, value) str_hash.set(std::make_pair(key, value))
#define DELETE_STR_FROM_HASH(key) try { str_hash.del(key); } catch (std::out_of_range &) {}

#if 1
#include "template.c"
#else

int main() {
    srand(time(0));
    using namespace std;
    SETUP
    cout << "in:" << endl;
    for(int i = 0; i < 20; i++) {
        int k = random() % 200;
        cout << k << " " << flush;
        INSERT_INT_INTO_HASH(k, 0);
        INSERT_INT_INTO_HASH(k + 1, 0);
        // for (size_t i = 0; i < hash._capacity; ++i) {
        //     if (hash._h[i]) {
        //         cout << hash._h[i] << ' ';
        //     } else {
        //         cout << '.' << ' ';
        //     }
        // }
        // cout << endl;
    }
    cout << endl;
    cout << "\nout:" << endl;
    for (size_t i = 0; i < hash._capacity; ++i) {
        if (hash._h[i]) {
            cout << hash._h[i] << ' ';
        } else {
            cout << '.' << ' ';
        }
    }
    cout << endl;
    // 0 1 2 3 6 7 9 10 12 13 15 16 17 19
    while (hash.size() > 1) {
        int k = random() % 200;
        size_t old_size = hash.size();
        DELETE_INT_FROM_HASH(k);
        if (hash.size() == old_size) {
            continue;
        }
        cout << "delete:" << k << endl;
        for (size_t i = 0; i < hash._capacity; ++i) {
            if (hash._h[i]) {
                cout << hash._h[i] << '(' << hash.probe_distance(hash._h[i], i) << ") ";
            } else {
                cout << '.' << ' ';
            }
        }
        cout << endl;
    }
}

#endif
