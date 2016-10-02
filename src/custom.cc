#include <cinttypes>


#include <utility> // swap, pair
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free
#include <stdexcept> // out_of_range
#include <cstrings> // bzero


#include <iostream>


template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K> >
class Custom {
public:

    explicit Custom(size_t capacity = 1):
        _h(capacity),
        _kv(capacity),
        _capacity(capacity),
        _size(0) {

        bzero(&_h[0], sizeof(size_t) * capacity);
    }

    ~Custom() {
        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i]) {
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

            if (!hash_i) {
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

    void set(std::pair<K, V> && kv) {
        size_t h = hash_key(kv.first);
        size_t i = bucket(h);
        size_t dist = 0;
        bool rehashed = false;

        while (true) {
            size_t hash_i = _h[i];

            if (!hash_i) {
                if (!rehashed) {
                    ++_size;
                    if (rehash()) {
                        i = bucket(h);
                        continue;
                    }
                }
                _h[i] = h;
                new (&_kv[i]) std::pair<K, V>(kv);
                return;
            }

            if (hash_i == h) {
                std::pair<K, V> & kv_i = _kv[i];
                if (keys_equal(kv.first, kv_i.first)) {
                    kv_i.second = std::move(kv.second);
                    return;
                }
            }

            size_t dist_i = probe_distance(hash_i, i);
            if (dist > dist_i) {
                if (!rehashed) {
                    ++_size;
                    if (rehash()) {
                        i = bucket(h);
                        continue;
                    }
                }
                std::swap(_h[i], h);
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
        bool rehashed = false;

        while (true) {
            size_t hash_i = _h[i];

            if (!hash_i) {
                throw std::out_of_range("");
            }

            if (hash_i == h) {
                std::pair<K, V> & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    _h[i] = 0;
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

    bool rehash() {
        size_t capacity;

        if (_size == _capacity) {
            capacity = _capacity * 2;
        // } else if (_size < _capacity / 4) { // TODO: fix this, its broken
        //     capacity = _capacity / 2;
        } else {
            return false;
        }

        Custom c(capacity);

        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i]) {
                c.set(std::move(_kv[i]));
                _h[i] = 0;
            }
        }

        std::swap(_h, c._h);
        std::swap(_kv, c._kv);
        c._capacity = _capacity;
        _capacity = capacity;
        _size = c._size;
        c._size = 0;
        return true;
    }

// private:
    inline static size_t hash_key(const K & k) {
        static H h;
        size_t hk = h(k);
        return hk | (hk == 0); // 0 reserved for empty
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
        inline ~Array() { free(_buffer); }
    };

    Array<size_t> _h; // hashes
    Array<std::pair<K, V> > _kv; // key value pairs

    size_t _capacity; // length of arrays
    size_t _size; // number of items stored
};


// using namespace std;
typedef Custom<int64_t, int64_t> hash_t;
typedef Custom<const char *, int64_t> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.set(std::make_pair(key, value))
#define DELETE_INT_FROM_HASH(key) try { hash.del(key); } catch (std::out_of_range &) { std::cout << "missing key " << key << std::endl; }
#define INSERT_STR_INTO_HASH(key, value) str_hash.set(std::make_pair(key, value))
#define DELETE_STR_FROM_HASH(key) try { str_hash.del(key); } catch (std::out_of_range &) { std::cout << "missing key " << key << std::endl; }

#if 1
#include "template.c"
#else

int main() {
    using namespace std;
    SETUP
    cout << "in:" << endl;
    for(int i = 0; i < 20; i++) {
        int k = random() % 20;
        cout << k << " " << flush;
        INSERT_INT_INTO_HASH(k, 0);
        for (size_t i = 0; i < hash._capacity; ++i) {
            if (hash._h[i]) {
                cout << hash._h[i] << ' ';
            } else {
                cout << '.' << ' ';
            }
        }
        cout << endl;
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
}

#endif
