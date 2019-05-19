// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_TAGGED_IMPL_H_
#define V8_OBJECTS_TAGGED_IMPL_H_

#include "include/v8-internal.h"
#include "include/v8.h"
#include "src/globals.h"

namespace v8 {
namespace internal {

// An TaggedImpl is a base class for Object (which is either a Smi or a strong
// reference to a HeapObject) and MaybeObject (which is either a Smi, a strong
// reference to a HeapObject, a weak reference to a HeapObject, or a cleared
// weak reference.
// This class provides storage and one canonical implementation of various
// predicates that check Smi and heap object tags' values and also take into
// account whether the tagged value is expected to be weak reference to a
// HeapObject or cleared weak reference.
template <HeapObjectReferenceType kRefType, typename StorageType>
class TaggedImpl {
 public:
  static const bool kCanBeWeak = kRefType == HeapObjectReferenceType::WEAK;

  constexpr TaggedImpl() : ptr_{} {}
  explicit constexpr TaggedImpl(StorageType ptr) : ptr_(ptr) {}

  // Make clang on Linux catch what MSVC complains about on Windows:
  operator bool() const = delete;

  constexpr bool operator==(TaggedImpl other) const {
    return ptr_ == other.ptr_;
  }
  constexpr bool operator!=(TaggedImpl other) const {
    return ptr_ != other.ptr_;
  }

  // For using in std::set and std::map.
  constexpr bool operator<(TaggedImpl other) const {
    return ptr_ < other.ptr();
  }

  constexpr StorageType ptr() const { return ptr_; }

  // Returns true if this tagged value is a strong pointer to a HeapObject or
  // Smi.
  constexpr inline bool IsObject() const { return !IsWeakOrCleared(); }

  // Returns true if this tagged value is a Smi.
  constexpr bool IsSmi() const { return HAS_SMI_TAG(ptr_); }
  inline bool ToSmi(Smi* value) const;
  inline Smi ToSmi() const;

  // Returns true if this tagged value is a strong pointer to a HeapObject.
  constexpr inline bool IsHeapObject() const { return IsStrong(); }

  // Returns true if this tagged value is a cleared weak reference.
  constexpr inline bool IsCleared() const {
    return kCanBeWeak &&
           (static_cast<uint32_t>(ptr_) == kClearedWeakHeapObjectLower32);
  }

  // Returns true if this tagged value is a strong or weak pointer to a
  // HeapObject.
  constexpr inline bool IsStrongOrWeak() const {
    return !IsSmi() && !IsCleared();
  }

  // Returns true if this tagged value is a strong pointer to a HeapObject.
  constexpr inline bool IsStrong() const {
#ifdef V8_CAN_HAVE_DCHECK_IN_CONSTEXPR
    DCHECK_IMPLIES(!kCanBeWeak, !IsSmi() == HAS_STRONG_HEAP_OBJECT_TAG(ptr_));
#endif
    return kCanBeWeak ? HAS_STRONG_HEAP_OBJECT_TAG(ptr_) : !IsSmi();
  }

  // Returns true if this tagged value is a weak pointer to a HeapObject.
  constexpr inline bool IsWeak() const {
    return IsWeakOrCleared() && !IsCleared();
  }

  // Returns true if this tagged value is a weak pointer to a HeapObject or
  // cleared weak reference.
  constexpr inline bool IsWeakOrCleared() const {
    return kCanBeWeak && HAS_WEAK_HEAP_OBJECT_TAG(ptr_);
  }

  // If this tagged value is a strong pointer to a HeapObject, returns true and
  // sets *result. Otherwise returns false.
  inline bool GetHeapObjectIfStrong(HeapObject* result) const;

  // DCHECKs that this tagged value is a strong pointer to a HeapObject and
  // returns the HeapObject.
  inline HeapObject GetHeapObjectAssumeStrong() const;

  // If this tagged value is a weak pointer to a HeapObject, returns true and
  // sets *result. Otherwise returns false.
  inline bool GetHeapObjectIfWeak(HeapObject* result) const;

  // DCHECKs that this tagged value is a weak pointer to a HeapObject and
  // returns the HeapObject.
  inline HeapObject GetHeapObjectAssumeWeak() const;

  // If this tagged value is a strong or weak pointer to a HeapObject, returns
  // true and sets *result. Otherwise returns false.
  inline bool GetHeapObject(HeapObject* result) const;
  inline bool GetHeapObject(HeapObject* result,
                            HeapObjectReferenceType* reference_type) const;

  // DCHECKs that this tagged value is a strong or a weak pointer to a
  // HeapObject and returns the HeapObject.
  inline HeapObject GetHeapObject() const;

  // DCHECKs that this tagged value is a strong or a weak pointer to a
  // HeapObject or a Smi and returns the HeapObject or Smi.
  inline Object GetHeapObjectOrSmi() const;

  template <typename T>
  T cast() const {
    DCHECK(!HAS_WEAK_HEAP_OBJECT_TAG(ptr_));
    return T::cast(Object(ptr_));
  }

  // Prints this object without details.
  void ShortPrint(FILE* out = stdout);

  // Prints this object without details to a message accumulator.
  void ShortPrint(StringStream* accumulator);

  void ShortPrint(std::ostream& os);

#ifdef OBJECT_PRINT
  void Print();
  void Print(std::ostream& os);
#else
  void Print() { ShortPrint(); }
  void Print(std::ostream& os) { ShortPrint(os); }
#endif

 private:
  friend class CompressedObjectSlot;
  friend class FullObjectSlot;

  StorageType ptr_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_OBJECTS_TAGGED_IMPL_H_
