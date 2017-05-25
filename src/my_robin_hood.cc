#include <utility> // swap
#include <functional> // hash
#include <cstdlib> // malloc, realloc, free

#include <iostream>
using namespace std;


template <class Key, class Value, class Hash = std::hash<Key>, class Pred = std::equal_to<Key> >
class HashTable {
public:
    struct Entry {
        size_t probe_distance;  // -1 means empty
        Key key;
        Value value;

        Entry(size_t probe_distance, const Key & key, const Value & value):
            probe_distance(probe_distance), key(key), value(value) {
        }
    };

    Entry * entries;
    size_t array_size;
    size_t entry_count;
    size_t bucket_mask;
    size_t grow_count;
    Hash hash;
    Pred pred;

    HashTable():
        entry_count(0) {
        resize(8);
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

    void resize(size_t new_size) {
        /* Allocate entries array, setting array_size and bucket_mask. */
        array_size = new_size;
        entries = (Entry *) malloc(sizeof(Entry) * new_size);
        if (!entries) {
            throw std::bad_alloc();
        }
        for (size_t i = 0; i < new_size; ++i) {
            entries[i].probe_distance = -1;
        }
        bucket_mask = new_size - 1;
        grow_count = new_size * 75 / 100;
    }

    Value * get(const Key & key) {
        for (size_t bucket = hash(key) & bucket_mask, probe_distance = 0;; bucket = (bucket + 1) & bucket_mask, ++probe_distance) {
            Entry & entry = entries[bucket];
            size_t entry_probe_distance = entry.probe_distance;

            if (entry_probe_distance == -1) {
                // we expected to find it here, but this slot is empty...
                return NULL;
            }

            if (entry_probe_distance == probe_distance) {
                // check for matching keys
                if (pred(key, entry.key)) {
                    return &entry.value;
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

    bool set(Key key, Value value) {
        for (size_t bucket = hash(key) & bucket_mask, probe_distance = 0;; bucket = (bucket + 1) & bucket_mask, ++probe_distance) {
            Entry & entry = entries[bucket];
            size_t entry_probe_distance = entry.probe_distance;

            if (entry_probe_distance == -1) {
                // here's an empty spot! lets put it here
                new (&entry) Entry(probe_distance, key, value);
                ++entry_count;
                break;
            }

            if (entry_probe_distance == probe_distance) {
                // check for matching keys
                if (pred(key, entry.key)) {
                    entry.value = value;
                    return true;
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

        if (entry_count < grow_count)
            return false;

        // grow
        Entry * old_entries = entries;
        size_t old_size = array_size;
        size_t old_entry_count = entry_count;
        entry_count = 0;
        resize(array_size << 1); // double
        for (size_t i = 0; old_entry_count && i < old_size; ++i) {
            if (old_entries[i].probe_distance != -1) {
                set(old_entries[i].key, old_entries[i].value);
                old_entries[i].~Entry();  // destruct
                --old_entry_count;
            }
        }
        free(old_entries);
        return false;
    }

    bool del(const Key & key) {
        for (size_t bucket = hash(key) & bucket_mask, probe_distance = 0;; bucket = (bucket + 1) & bucket_mask, ++probe_distance) {
            Entry & entry = entries[bucket];
            size_t entry_probe_distance = entry.probe_distance;

            if (entry_probe_distance == -1) {
                // we expected to find it here, but this slot is empty...
                return false;
            }

            if (entry_probe_distance == probe_distance) {
                // check for matching keys
                if (pred(key, entry.key)) {
                    entry.~Entry(); // destruct
                    entry.probe_distance = -1;
                    --entry_count;
                    return true;
                }

                // keys dont match, it might be the next one (because of collisions)
                continue;
            }

            if (entry_probe_distance < probe_distance) {
                // we should have found the entry already, must not be here
                return false;
            }

            // just keep looking...
        }
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
