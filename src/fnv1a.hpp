#include <inttypes.h>
#include <stddef.h>
#include <functional>


template<class T>
struct fnv_1a_traits;


template<>
struct fnv_1a_traits<uint32_t> {
    static const uint32_t offset_basis = 2166136261u;
    static const uint32_t prime = 16777619u;
};


template<>
struct fnv_1a_traits<uint64_t> {
    static const uint64_t offset_basis = 14695981039346656037u;
    static const uint64_t prime = 1099511628211u;
};


template <class T>
T fnv_1a(const char * data, size_t size) {
    T hash = fnv_1a_traits<T>::offset_basis;
    while (size) {
        hash *= fnv_1a_traits<T>::prime;
        hash ^= data[0];
        ++data;
        --size;
    }
    return hash;
}


template <class T>
T fnv_1a(const char * str) {
    T hash = fnv_1a_traits<T>::offset_basis;
    char c;
    while ((c = *str)) {
        hash *= fnv_1a_traits<T>::prime;
        hash ^= c;
        ++str;
    }
    return hash;
}


namespace std {
    template<>
    struct hash<const char *> {
        size_t operator()(const char * s) const {
            return fnv_1a<uint64_t>(s);
        }
    };
}
