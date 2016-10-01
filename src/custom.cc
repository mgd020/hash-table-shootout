#include <cinttypes>


#include <utility> // swap, pair
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free
#include <stdexcept> // out_of_range


#include <iostream>


template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K> >
class Custom {
public:

    Custom(size_t capacity):
        _h(capacity),
        _kv(capacity),
        _capacity(capacity),
        _size(0) {
        memset(&_h[0], -1, sizeof(size_t) * capacity);
    }

    ~Custom() {
        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i] != -1) {
                destruct(_kv[i]);
            }
        }
    }

    size_t size() const {
        return _size;
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

            if (hash_i == -1) {
                throw std::out_of_range("");
            }

            if (hash_i == h) {
                std::pair<K, V> & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    return kv.second;
                }
            }

            size_t dist_i = probe_distance(hash_i, i);
            if (dist > dist_i) {
                throw std::out_of_range("");
            }

            i = (i + 1) % _capacity;
            ++dist;
        }
    }

    inline const V & get(const K & k) const {
        return const_cast<Custom *>(this)->get(k);
    }

    void set(K && k, V && v) {
        size_t h = hash_key(k);
        size_t i = bucket(h);
        size_t dist = 0;

        while (true) {
            size_t hash_i = _h[i];

            if (hash_i == -1) {
                // std::cout << "replacing empty at " << i << std::endl;
                _h[i] = h;
                new (&_kv[i]) std::pair<K, V>(k, v);
                ++_size;
                rehash();
                return;
            }

            if (hash_i == h) {
                std::pair<K, V> & kv_i = _kv[i];
                if (keys_equal(k, kv_i.first)) {
                    // std::cout << "replacing value at " << i << std::endl;
                    kv_i.second = std::move(v);
                    return;
                }
            }

            size_t dist_i = probe_distance(hash_i, i);
            if (dist > dist_i) {
                // std::cout << "swapping with " << i << std::endl;
                std::swap(_h[i], h);
                // std::swap(key(i), k);
                // std::swap(val(i), v);
                std::swap(_kv[i], kv);
                dist = dist_i;
            }

            i = (i + 1) % _capacity;
            ++dist;
        }
    }

    void del(const K & k) {
        if (!_size) {
            throw std::out_of_range("");
        }

        size_t h = hash_key(k);
        size_t i = bucket(h);
        size_t dist = 0;

        while (true) {
            size_t hash_i = _h[i];

            if (hash_i == -1) {
                throw std::out_of_range("");
            }

            if (hash_i == h) {
                std::pair<K, V> & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    _h[i] = -1;
                    destruct(kv);
                    --_size;
                    rehash();
                    return;
                }
            }

            size_t dist_i = probe_distance(hash_i, i);
            if (dist > dist_i) {
                throw std::out_of_range("");
            }

            i = (i + 1) % _capacity;
            ++dist;
        }
    }

    double load_factor() const {
        return 1.0 * _size / _capacity;
    }

    void rehash() {
        size_t capacity;

        if (_size == _capacity) {
            capacity = _capacity * 2;
        } else if (_size < _capacity / 4) {
            capacity = _capacity / 2;
        } else {
            return;
        }

        Custom c = Custom(capacity);

        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i] == -1) {
                continue;
            }
            std::pair<K, V> & kv = _kv[i];
            c.set(std::move(kv.first), std::move(kv.second));
        }

        memset(&_h[0], -1, sizeof(size_t) * capacity);c
        std::swap(*this, c);
    }

// private:
    inline static size_t hash_key(const K & k) {
        return H()(k) ?: -1; // 0 is reserved for empty
    }

    inline size_t bucket(size_t h) {
        return h % _capacity;
    }

    inline size_t probe_distance(size_t h, size_t i) {
        return (i + _capacity - bucket(h)) % _capacity;
    }

    inline static bool keys_equal(const K & k1, const K & k2) {
        return P()(k1, k2);
    }

    template <class T>
    inline void destruct(T & t) {
        t.~T();
    }

    template <class T>
    class Array {
        T * __restrict _buffer;
    public:
        inline Array(): _buffer(NULL) {}
        inline explicit Array(size_t s): _buffer(s ? (T *)malloc(sizeof(T) * s) : NULL) {}
        inline Array(const Array &) = delete;
        inline Array & operator=(const Array &) = delete;
        inline Array(Array && a): _buffer(a._buffer) { a._buffer = NULL; }
        inline Array & operator=(Array && a) { std::swap(_buffer, a._buffer); return *this; }
        inline T & operator[](size_t i) { return _buffer[i]; }
        inline const T & operator[](size_t i) const { return _buffer[i]; }
        inline void resize(size_t s) { _buffer = (T *)realloc(_buffer, sizeof(T) * s); }
        inline void swap(Array & a) { std::swap(_buffer, a._buffer); }
        inline ~Array() { free(_buffer); }
    };

    Array<size_t> _h; // hashes
    // Array<K> _k; // keys
    // Array<V> _v; // values
    Array<std::pair<K, V> > _kv;

    size_t _capacity; // length of arrays
    size_t _size; // number of items stored
    size_t _grow;
    size_t _shrink;
};


typedef Custom<int64_t, int64_t> hash_t;
typedef Custom<const char *, int64_t> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.set(key, value)
#define DELETE_INT_FROM_HASH(key) try { hash.del(key); } catch (std::out_of_range &) {}
#define INSERT_STR_INTO_HASH(key, value) str_hash.set(key, value)
#define DELETE_STR_FROM_HASH(key) try { str_hash.del(key); } catch (std::out_of_range &) {}
#include "template.c"

// int main() {
//     using namespace std;
//     SETUP
//     cout << "in:" << endl;
//     for(int i = 0; i < 20; i++) {
//         int k = random() % 20;
//         cout << k << " " << flush;
//         INSERT_INT_INTO_HASH(k, 0);
//         // for (size_t i = 0; i < hash._capacity; ++i) {
//         //     cout << hash._h[i] << ' ';
//         // }
//         // cout << endl;
//     }
//     cout << endl;
//     cout << "\nout:" << endl;
//     for (size_t i = 0; i < hash._capacity; ++i) {
//         cout << hash._h[i] << ' ';
//     }
//     cout << endl;
//     // 0 1 10 12 13 15 16 17 19 2 3 6 7 9 }
// }
