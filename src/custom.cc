// #include <cinttypes>

#include <utility> // swap, pair
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free
#include <stdexcept> // out_of_range
#include <strings.h> // bzero


template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K> >
class Custom {
public:
    typedef std::pair<K, V> value_type;

    explicit Custom():
        _capacity(4),
        _load_factor(90),
        _size(0) {
        alloc();
    }

    ~Custom() {
        for (size_t i = 0; i < _capacity; ++i) {
            if (_h[i] != -1) {
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

            if (hash_i == -1) {
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

            i = (i + 1) & _mask;
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

            if (hash_i == -1) {
                return;
            }

            if (hash_i == h) {
                value_type & kv = _kv[i];
                if (keys_equal(k, kv.first)) {
                    destruct(kv);
                    _h[i] = -1;
                    --_size;
                    break;
                }
            } else {
                if (dist > probe_distance(hash_i, i)) {
                    return;
                }
            }

            i = (i + 1) & _mask;
            ++dist;
        }

        if (_size == _shrink) {
            rehash(_capacity / 2);
        } else {
            while (true) {
                i = (i + 1) & _mask;

                size_t hash_i = _h[i];
                if (hash_i == -1) {
                    break;
                }

                if (!probe_distance(hash_i, i)) {
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
            if (h[i] != -1) {
                _set(h[i], std::move(kv[i]));
            }
        }

        free(h);
        free(kv);
    }

    void alloc() {
        _h = (size_t *)malloc(sizeof(size_t) * _capacity);
        _kv = (value_type *)malloc(sizeof(value_type) * _capacity);
        memset(_h, -1, sizeof(size_t) * _capacity);
        _grow = _load_factor * _capacity / 100;
        _shrink = _load_factor * _capacity / 400;
        _mask = _capacity - 1;
    }

    void _set(size_t h, value_type && kv) {
        if (_size == _grow) {
            rehash(_capacity * 2);
        }

        size_t i = bucket(h);
        size_t dist = 0;

        while (true) {
            size_t hash_i = _h[i];

            if (hash_i == -1) {
                new (&_kv[i]) value_type(std::move(kv));
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

            i = (i + 1) & _mask;
            ++dist;
        }
    }

    inline static size_t hash_key(const K & k) {
        static H h;
        size_t hk = h(k);
        return hk == -1 ? 0 : hk;
    }

    inline size_t bucket(size_t h) const {
        return h & _mask;
    }

    inline size_t probe_distance(size_t h, size_t i) const {
        return (i + _capacity - bucket(h)) & _mask;
    }

    inline static bool keys_equal(const K & k1, const K & k2) {
        static P p;
        return p(k1, k2);
    }

    template <class T>
    inline static void destruct(T & t) {
        t.~T();
    }

    size_t * __restrict _h; // hashes (0 is empty)
    value_type * __restrict _kv; // key value pairs
    size_t _capacity; // length of arrays
    size_t _size; // number of items stored
    size_t _load_factor; // maximum load factor before growing, /4 for minimum before shrinking
    size_t _grow; // when _size >= _grow, _capcity *= 2
    size_t _shrink; // when _size < _shrink, _capacity /= 2
    size_t _mask; // used instead of % _capacity for speed
};


// using namespace std;
#include <cinttypes>
typedef Custom<int64_t, int64_t> hash_t;
typedef Custom<const char *, int64_t> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.set(std::make_pair(key, value))
#define DELETE_INT_FROM_HASH(key) hash.del(key)
#define INSERT_STR_INTO_HASH(key, value) str_hash.set(std::make_pair(key, value))
#define DELETE_STR_FROM_HASH(key) str_hash.del(key)

#if 1
#include "template.c"
#else

#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

void dump(const hash_t & h) {
    for (size_t i = 0; i < h._capacity; ++i) {
        if (h._h[i] != -1) {
            cout << h._h[i] << ':' << h.probe_distance(h._h[i], i) << ' ';
        } else {
            cout << ". ";
        }
    }
    cout << endl;
}

int main() {
    cout << (size_t)-1 << endl;
    cout << (size_t)(0xFFFFFFFFFFFFFFFF) << endl;

    srand(time(0));
    vector<size_t> keys;
    size_t n = 20;
    for (int i = 0; i < n; ++i) {
        keys.push_back(random() % 20);
    }

    SETUP
    for(size_t i = 0; i < n; ++i) {
        INSERT_INT_INTO_HASH(keys[i], 0);
        cout << "insert " << keys[i] << " with hash:" << hash_t::hash_key(keys[i]) << " and bucket:" << hash.bucket(hash_t::hash_key(keys[i])) << endl;
        dump(hash);
    }

    for(size_t i = 0; i < n; ++i) {
        cout << "delete " << keys[i] << endl;
        DELETE_INT_FROM_HASH(keys[i]);
        dump(hash);
    }
}

#endif
