#include <utility> // swap
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free

#include <iostream>
using namespace std;


template <class Key, class Value>
struct HashTableTraits {
    typedef std::hash<Key> hash_type;
    typedef std::equal_to<Key> pred_type;
    static const int max_load_factor = 75; // percent
    static const int min_load_factor = 25; // percent
    static const int min_array_size = 8; // must be 2 ** n
};


template <class Key, class Value, class Traits = HashTableTraits<Key, Value> >
class HashTable {
public:
    struct Entry {
        size_t probe_distance;  // -1 means empty
        Key key;
        Value value;

        Entry(size_t probe_distance, Key && key, Value && value):
            probe_distance(probe_distance), key(std::move(key)), value(std::move(value)) {
        }
    };

    Entry * entries;
    size_t array_size;
    size_t entry_count;
    size_t bucket_mask;
    size_t grow_count;
    size_t shrink_count;

    HashTable():
        entries(NULL),
        array_size(0),
        entry_count(0)
    {
        rehash(Traits::min_array_size);
    }

    ~HashTable() {
        for (size_t i = 0, e = entry_count; e && i < array_size; ++i) {
            Entry & entry = entries[i];
            if (entry.probe_distance != -1) {
                entry.~Entry();
                --e;
            }
        }
        free(entries);
    }

    static size_t hash(const Key & key) {
        static const typename Traits::hash_type hasher;
        return hasher(key);
    }

    static bool pred(const Key & k1, const Key & k2) {
        static const typename Traits::pred_type preder;
        return preder(k1, k2);
    }

    bool rehash(size_t new_size) {
        if (new_size < Traits::min_array_size) {
            new_size = Traits::min_array_size;
        }

        if (new_size == array_size) {
            return false;
        }

        Entry * old_entries = entries;
        size_t old_size = array_size;

        array_size = new_size;
        entries = (Entry *) malloc(sizeof(Entry) * new_size);
        if (!entries) {
            throw std::bad_alloc();
        }
        for (size_t i = 0; i < new_size; ++i) {
            entries[i].probe_distance = -1;
        }
        bucket_mask = new_size - 1;
        grow_count = new_size * Traits::max_load_factor / 100;
        shrink_count = new_size * Traits::min_load_factor / 100;

        if (old_entries) {
            for (size_t i = 0, e = entry_count; e && i < old_size; ++i) {
                if (old_entries[i].probe_distance != -1) {
                    set_helper(std::move(old_entries[i].key), std::move(old_entries[i].value));
                    old_entries[i].~Entry();
                    --e;
                }
            }
            free(old_entries);
        }

        return true;
    }

    Entry * find(const Key & key) {
        size_t bucket = hash(key) & bucket_mask;
        size_t probe_distance = 0;

        for (;; bucket = (bucket + 1) & bucket_mask, ++probe_distance) {
            Entry & entry = entries[bucket];
            size_t entry_probe_distance = entry.probe_distance;

            if (entry_probe_distance == -1) {
                // we expected to find it here, but this slot is empty...
                return NULL;
            }

            if (entry_probe_distance == probe_distance) {
                // check for matching keys
                if (pred(key, entry.key)) {
                    return &entry;
                }

                // keys dont match, it might be the next one (because of collisions)
                continue;
            }

            if (entry_probe_distance < probe_distance) {
                // we should have found the entry already, must not be here
                return NULL;
            }

            // just keep looking...
        }
    }

    bool set_helper(Key && key, Value && value) {
        // returns if new element was added
        size_t bucket = hash(key) & bucket_mask;
        size_t probe_distance = 0;

        for (;; bucket = (bucket + 1) & bucket_mask, ++probe_distance) {
            Entry & entry = entries[bucket];
            size_t entry_probe_distance = entry.probe_distance;

            if (entry_probe_distance == -1) {
                // here's an empty spot! lets put it here
                new (&entry) Entry(probe_distance, std::move(key), std::move(value));
                return true;
            }

            if (entry_probe_distance == probe_distance) {
                // check for matching keys
                if (pred(key, entry.key)) {
                    std::swap(entry.value, value);
                    return false;
                }

                // keys dont match, it might be the next one (because of collisions)
                continue;
            }

            if (entry_probe_distance < probe_distance) {
                // this entry is closer than we would be, lets swap it out
                std::swap(entry.key, key);
                std::swap(entry.value, value);
                std::swap(entry.probe_distance, probe_distance);

                // and we'll find a new home for entry
                continue;
            }

            // this entry is further from its bucket than we are
            // leave it alone and keep looking for a spot
        }
    }

    Value * get(const Key & key) {
        Entry * entry = find(key);
        if (!entry)
            return NULL;
        return &entry->value;
    }

    bool set(Key key, Value value) {
        if (!set_helper(std::move(key), std::move(value))) {
            // no new element added
            return false;
        }

        if (++entry_count > grow_count)
            rehash(array_size << 1); // double

        return true;
    }

    bool del(const Key & key) {
        Entry * entry = find(key);
        if (!entry) {
            return false;
        }

        if (--entry_count < shrink_count) {
            entry->~Entry(); // destruct
            entry->probe_distance = -1; // set as empty
            if (rehash(array_size >> 1)) { // half
                return true;
            }
        }

        // TODO: move the rest down while they have probe distances

        // move items up
        // size_t bucket = (entry - entries + 1) & bucket_mastk;
        // for (;; bucket = (bucket + 1) & bucket_mask) {
        //     Entry & entry = entries[bucket];
        //     if (entry.probe_distance != -1)
        //         break;
        //     if (entry.probe_distance != -1) {
        //         Entry & left_entry = entries[(bucket + array_size - 1) & bucket_mask];
        //         --entry.probe_distance;

        //         std::move()
        //         std::move()
        //         std::swap()
        //     }
        // }

        return true;
    }
};


#include <cinttypes>
typedef HashTable<int64_t, int64_t> hash_t;
typedef HashTable<const char *, int64_t> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.set(key, value)
#define LOOKUP_INT_IN_HASH(key) hash.get(key) != NULL
#define DELETE_INT_FROM_HASH(key) hash.del(key)
#define INSERT_STR_INTO_HASH(key, value) str_hash.set(key, value)
#define LOOKUP_STR_IN_HASH(key) str_hash.get(key) != NULL
#define DELETE_STR_FROM_HASH(key) str_hash.del(key)

#if 1
#include "template.c"
#else

template <class H>
void dump(const H & hash_table) {
    cout << "-------- HASH TABLE ---------" << endl;
    cout << "array_size: " << hash_table.array_size << endl;
    cout << "entry_count: " << hash_table.entry_count << endl;
    cout << "bucket_mask: " << hash_table.bucket_mask << endl;
    cout << "grow_count: " << hash_table.grow_count << endl;
    cout << "\tprob.d\tkey\tvalue" << endl;
    for (size_t i = 0; i < hash_table.array_size; ++i) {
        cout << i << '\t';
        typename H::Entry & entry = hash_table.entries[i];
        if (entry.probe_distance == -1) {
            cout << "-\t-\t-";
        } else {
            cout << entry.probe_distance << '\t' << entry.key << '\t' << entry.value;
        }
        cout << endl;
    }
    cout << "------- END HASH TABLE ------" << endl;
}

int main() {
    hash_t h;
    dump(h);
    h.set(1, 1);
    dump(h);
    h.set(9, 9);
    dump(h);
    h.set(17, 17);
    dump(h);
    h.set(0, 0);
    dump(h);
    h.set(8, 8);
    dump(h);
    h.set(6, 6);
    dump(h);
}

#endif
