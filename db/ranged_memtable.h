#ifndef STORAGE_LEVELDB_DB_RANGED_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_RANGED_MEMTABLE_H_


#include <string>

#include "db/dbformat.h"
#include "db/skiplist.h"
#include "leveldb/db.h"
#include "util/arena.h"



namespace leveldb {

class InternalKeyComparator;
class RangedMemTableIterator;

class RangedMemtable {
 public:
  // RangedMemtables are reference counted.  The initial reference count
  // is zero and the caller must call Ref() at least once.
  explicit RangedMemtable(const InternalKeyComparator& comparator);

  RangedMemtable(const RangedMemtable&) = delete;
  RangedMemtable& operator=(const RangedMemtable&) = delete;

  // Increase reference count.
  void Ref() { ++refs_; }

  // Drop reference count.  Delete if no more references exist.
  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  // Returns an estimate of the number of bytes of data in use by this
  // data structure. It is safe to call when RangedMemtable is being modified.
  size_t ApproximateMemoryUsage();

  // Return an iterator that yields the contents of the memtable.
  //
  // The caller must ensure that the underlying RangedMemtable remains live
  // while the returned iterator is live.  The keys returned by this
  // iterator are internal keys encoded by AppendInternalKey in the
  // db/format.{h,cc} module.
  Iterator* NewIterator();

  // Add an entry into memtable that maps key to value at the
  // specified sequence number and with the specified type.
  // Typically value will be empty if type==kTypeDeletion.
  void Add(SequenceNumber seq, ValueType type, const Slice& key,
           const Slice& value);

  // If memtable contains a value for key, store it in *value and return true.
  // If memtable contains a deletion for key, store a NotFound() error
  // in *status and return true.
  // Else, return false.
  bool Get(const LookupKey& key, std::string* value, Status* s);

  ~RangedMemtable();

  struct KeyRange {
    int begin = -1;
    int end = -1;
  };

  KeyRange keyRange_;

 private:
  friend class RangedMemtableIterator;
  friend class MemtableBackwardIterator;
  friend class Memtable;

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
    int operator()(const char* a, const char* b) const;
  };

  typedef SkipList<const char*, KeyComparator> Table;

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;

    // Private since only Unref() should be used to delete it
  
};

}  // namespace leveldb

#endif //STORAGE_LEVELDB_DB_RANGED_MEMTABLE_H_
