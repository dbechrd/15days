// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SAVEFILE_SAVEFILE_H_
#define FLATBUFFERS_GENERATED_SAVEFILE_SAVEFILE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 2 &&
              FLATBUFFERS_VERSION_MINOR == 0 &&
              FLATBUFFERS_VERSION_REVISION == 8,
             "Non-compatible flatbuffers version included");

namespace SaveFile {

struct Material;
struct MaterialBuilder;

struct Root;
struct RootBuilder;

enum MaterialStates : uint32_t {
  MaterialStates_OnFire = 1,
  MaterialStates_NONE = 0,
  MaterialStates_ANY = 1
};

inline const MaterialStates (&EnumValuesMaterialStates())[1] {
  static const MaterialStates values[] = {
    MaterialStates_OnFire
  };
  return values;
}

inline const char * const *EnumNamesMaterialStates() {
  static const char * const names[2] = {
    "OnFire",
    nullptr
  };
  return names;
}

inline const char *EnumNameMaterialStates(MaterialStates e) {
  if (flatbuffers::IsOutRange(e, MaterialStates_OnFire, MaterialStates_OnFire)) return "";
  const size_t index = static_cast<size_t>(e) - static_cast<size_t>(MaterialStates_OnFire);
  return EnumNamesMaterialStates()[index];
}

struct Material FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MaterialBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "SaveFile.Material";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MATERIAL_PROTO = 4,
    VT_STATES = 6
  };
  const flatbuffers::String *material_proto() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL_PROTO);
  }
  SaveFile::MaterialStates states() const {
    return static_cast<SaveFile::MaterialStates>(GetField<uint32_t>(VT_STATES, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_MATERIAL_PROTO) &&
           verifier.VerifyString(material_proto()) &&
           VerifyField<uint32_t>(verifier, VT_STATES, 4) &&
           verifier.EndTable();
  }
};

struct MaterialBuilder {
  typedef Material Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_material_proto(flatbuffers::Offset<flatbuffers::String> material_proto) {
    fbb_.AddOffset(Material::VT_MATERIAL_PROTO, material_proto);
  }
  void add_states(SaveFile::MaterialStates states) {
    fbb_.AddElement<uint32_t>(Material::VT_STATES, static_cast<uint32_t>(states), 0);
  }
  explicit MaterialBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Material> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Material>(end);
    return o;
  }
};

inline flatbuffers::Offset<Material> CreateMaterial(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> material_proto = 0,
    SaveFile::MaterialStates states = static_cast<SaveFile::MaterialStates>(0)) {
  MaterialBuilder builder_(_fbb);
  builder_.add_states(states);
  builder_.add_material_proto(material_proto);
  return builder_.Finish();
}

inline flatbuffers::Offset<Material> CreateMaterialDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *material_proto = nullptr,
    SaveFile::MaterialStates states = static_cast<SaveFile::MaterialStates>(0)) {
  auto material_proto__ = material_proto ? _fbb.CreateString(material_proto) : 0;
  return SaveFile::CreateMaterial(
      _fbb,
      material_proto__,
      states);
}

struct Root FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RootBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "SaveFile.Root";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_MATERIALS = 6
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  bool KeyCompareLessThan(const Root *o) const {
    return *name() < *o->name();
  }
  int KeyCompareWithValue(const char *_name) const {
    return strcmp(name()->c_str(), _name);
  }
  const flatbuffers::Vector<flatbuffers::Offset<SaveFile::Material>> *materials() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<SaveFile::Material>> *>(VT_MATERIALS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_MATERIALS) &&
           verifier.VerifyVector(materials()) &&
           verifier.VerifyVectorOfTables(materials()) &&
           verifier.EndTable();
  }
};

struct RootBuilder {
  typedef Root Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Root::VT_NAME, name);
  }
  void add_materials(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<SaveFile::Material>>> materials) {
    fbb_.AddOffset(Root::VT_MATERIALS, materials);
  }
  explicit RootBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Root> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Root>(end);
    fbb_.Required(o, Root::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<Root> CreateRoot(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<SaveFile::Material>>> materials = 0) {
  RootBuilder builder_(_fbb);
  builder_.add_materials(materials);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<Root> CreateRootDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<flatbuffers::Offset<SaveFile::Material>> *materials = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto materials__ = materials ? _fbb.CreateVector<flatbuffers::Offset<SaveFile::Material>>(*materials) : 0;
  return SaveFile::CreateRoot(
      _fbb,
      name__,
      materials__);
}

inline const SaveFile::Root *GetRoot(const void *buf) {
  return flatbuffers::GetRoot<SaveFile::Root>(buf);
}

inline const SaveFile::Root *GetSizePrefixedRoot(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<SaveFile::Root>(buf);
}

inline const char *RootIdentifier() {
  return "15dy";
}

inline bool RootBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, RootIdentifier());
}

inline bool SizePrefixedRootBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, RootIdentifier(), true);
}

inline bool VerifyRootBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<SaveFile::Root>(RootIdentifier());
}

inline bool VerifySizePrefixedRootBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<SaveFile::Root>(RootIdentifier());
}

inline const char *RootExtension() {
  return "fbb";
}

inline void FinishRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SaveFile::Root> root) {
  fbb.Finish(root, RootIdentifier());
}

inline void FinishSizePrefixedRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SaveFile::Root> root) {
  fbb.FinishSizePrefixed(root, RootIdentifier());
}

}  // namespace SaveFile

#endif  // FLATBUFFERS_GENERATED_SAVEFILE_SAVEFILE_H_
