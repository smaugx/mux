// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: mux.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_mux_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_mux_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3013000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3013000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_mux_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_mux_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_mux_2eproto;
namespace mux {
namespace protobuf {
class MuxMessage;
class MuxMessageDefaultTypeInternal;
extern MuxMessageDefaultTypeInternal _MuxMessage_default_instance_;
}  // namespace protobuf
}  // namespace mux
PROTOBUF_NAMESPACE_OPEN
template<> ::mux::protobuf::MuxMessage* Arena::CreateMaybeMessage<::mux::protobuf::MuxMessage>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace mux {
namespace protobuf {

// ===================================================================

class MuxMessage PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:mux.protobuf.MuxMessage) */ {
 public:
  inline MuxMessage() : MuxMessage(nullptr) {}
  virtual ~MuxMessage();

  MuxMessage(const MuxMessage& from);
  MuxMessage(MuxMessage&& from) noexcept
    : MuxMessage() {
    *this = ::std::move(from);
  }

  inline MuxMessage& operator=(const MuxMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline MuxMessage& operator=(MuxMessage&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const MuxMessage& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const MuxMessage* internal_default_instance() {
    return reinterpret_cast<const MuxMessage*>(
               &_MuxMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(MuxMessage& a, MuxMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(MuxMessage* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(MuxMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline MuxMessage* New() const final {
    return CreateMaybeMessage<MuxMessage>(nullptr);
  }

  MuxMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<MuxMessage>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const MuxMessage& from);
  void MergeFrom(const MuxMessage& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(MuxMessage* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "mux.protobuf.MuxMessage";
  }
  protected:
  explicit MuxMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_mux_2eproto);
    return ::descriptor_table_mux_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kDataFieldNumber = 3,
    kVersionFieldNumber = 1,
    kTypeFieldNumber = 2,
  };
  // bytes data = 3;
  void clear_data();
  const std::string& data() const;
  void set_data(const std::string& value);
  void set_data(std::string&& value);
  void set_data(const char* value);
  void set_data(const void* value, size_t size);
  std::string* mutable_data();
  std::string* release_data();
  void set_allocated_data(std::string* data);
  private:
  const std::string& _internal_data() const;
  void _internal_set_data(const std::string& value);
  std::string* _internal_mutable_data();
  public:

  // uint32 version = 1;
  void clear_version();
  ::PROTOBUF_NAMESPACE_ID::uint32 version() const;
  void set_version(::PROTOBUF_NAMESPACE_ID::uint32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint32 _internal_version() const;
  void _internal_set_version(::PROTOBUF_NAMESPACE_ID::uint32 value);
  public:

  // uint32 type = 2;
  void clear_type();
  ::PROTOBUF_NAMESPACE_ID::uint32 type() const;
  void set_type(::PROTOBUF_NAMESPACE_ID::uint32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint32 _internal_type() const;
  void _internal_set_type(::PROTOBUF_NAMESPACE_ID::uint32 value);
  public:

  // @@protoc_insertion_point(class_scope:mux.protobuf.MuxMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr data_;
  ::PROTOBUF_NAMESPACE_ID::uint32 version_;
  ::PROTOBUF_NAMESPACE_ID::uint32 type_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_mux_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// MuxMessage

// uint32 version = 1;
inline void MuxMessage::clear_version() {
  version_ = 0u;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 MuxMessage::_internal_version() const {
  return version_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 MuxMessage::version() const {
  // @@protoc_insertion_point(field_get:mux.protobuf.MuxMessage.version)
  return _internal_version();
}
inline void MuxMessage::_internal_set_version(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  
  version_ = value;
}
inline void MuxMessage::set_version(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  _internal_set_version(value);
  // @@protoc_insertion_point(field_set:mux.protobuf.MuxMessage.version)
}

// uint32 type = 2;
inline void MuxMessage::clear_type() {
  type_ = 0u;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 MuxMessage::_internal_type() const {
  return type_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 MuxMessage::type() const {
  // @@protoc_insertion_point(field_get:mux.protobuf.MuxMessage.type)
  return _internal_type();
}
inline void MuxMessage::_internal_set_type(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  
  type_ = value;
}
inline void MuxMessage::set_type(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  _internal_set_type(value);
  // @@protoc_insertion_point(field_set:mux.protobuf.MuxMessage.type)
}

// bytes data = 3;
inline void MuxMessage::clear_data() {
  data_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& MuxMessage::data() const {
  // @@protoc_insertion_point(field_get:mux.protobuf.MuxMessage.data)
  return _internal_data();
}
inline void MuxMessage::set_data(const std::string& value) {
  _internal_set_data(value);
  // @@protoc_insertion_point(field_set:mux.protobuf.MuxMessage.data)
}
inline std::string* MuxMessage::mutable_data() {
  // @@protoc_insertion_point(field_mutable:mux.protobuf.MuxMessage.data)
  return _internal_mutable_data();
}
inline const std::string& MuxMessage::_internal_data() const {
  return data_.Get();
}
inline void MuxMessage::_internal_set_data(const std::string& value) {
  
  data_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void MuxMessage::set_data(std::string&& value) {
  
  data_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:mux.protobuf.MuxMessage.data)
}
inline void MuxMessage::set_data(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  data_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:mux.protobuf.MuxMessage.data)
}
inline void MuxMessage::set_data(const void* value,
    size_t size) {
  
  data_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:mux.protobuf.MuxMessage.data)
}
inline std::string* MuxMessage::_internal_mutable_data() {
  
  return data_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* MuxMessage::release_data() {
  // @@protoc_insertion_point(field_release:mux.protobuf.MuxMessage.data)
  return data_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void MuxMessage::set_allocated_data(std::string* data) {
  if (data != nullptr) {
    
  } else {
    
  }
  data_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), data,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:mux.protobuf.MuxMessage.data)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protobuf
}  // namespace mux

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_mux_2eproto
