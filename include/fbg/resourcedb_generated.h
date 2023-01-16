// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RESOURCEDB_RESOURCEDB_H_
#define FLATBUFFERS_GENERATED_RESOURCEDB_RESOURCEDB_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 2 &&
              FLATBUFFERS_VERSION_MINOR == 0 &&
              FLATBUFFERS_VERSION_REVISION == 8,
             "Non-compatible flatbuffers version included");

namespace ResourceDB {

struct Animation;
struct AnimationBuilder;

struct Spritesheet;
struct SpritesheetBuilder;

struct MaterialProto;
struct MaterialProtoBuilder;

struct CardProto;
struct CardProtoBuilder;

struct Root;
struct RootBuilder;

enum MaterialAttribs : uint32_t {
  MaterialAttribs_Flammable = 1,
  MaterialAttribs_NONE = 0,
  MaterialAttribs_ANY = 1
};

inline const MaterialAttribs (&EnumValuesMaterialAttribs())[1] {
  static const MaterialAttribs values[] = {
    MaterialAttribs_Flammable
  };
  return values;
}

inline const char * const *EnumNamesMaterialAttribs() {
  static const char * const names[2] = {
    "Flammable",
    nullptr
  };
  return names;
}

inline const char *EnumNameMaterialAttribs(MaterialAttribs e) {
  if (flatbuffers::IsOutRange(e, MaterialAttribs_Flammable, MaterialAttribs_Flammable)) return "";
  const size_t index = static_cast<size_t>(e) - static_cast<size_t>(MaterialAttribs_Flammable);
  return EnumNamesMaterialAttribs()[index];
}

enum EffectType : uint32_t {
  EffectType_IgniteFlammable = 0,
  EffectType_ExtinguishFlammable = 1,
  EffectType_MIN = EffectType_IgniteFlammable,
  EffectType_MAX = EffectType_ExtinguishFlammable
};

inline const EffectType (&EnumValuesEffectType())[2] {
  static const EffectType values[] = {
    EffectType_IgniteFlammable,
    EffectType_ExtinguishFlammable
  };
  return values;
}

inline const char * const *EnumNamesEffectType() {
  static const char * const names[3] = {
    "IgniteFlammable",
    "ExtinguishFlammable",
    nullptr
  };
  return names;
}

inline const char *EnumNameEffectType(EffectType e) {
  if (flatbuffers::IsOutRange(e, EffectType_IgniteFlammable, EffectType_ExtinguishFlammable)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesEffectType()[index];
}

struct Animation FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef AnimationBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "ResourceDB.Animation";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_DESC = 6,
    VT_FRAME_START = 8,
    VT_FRAME_COUNT = 10
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::String *desc() const {
    return GetPointer<const flatbuffers::String *>(VT_DESC);
  }
  int32_t frame_start() const {
    return GetField<int32_t>(VT_FRAME_START, 0);
  }
  int32_t frame_count() const {
    return GetField<int32_t>(VT_FRAME_COUNT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_DESC) &&
           verifier.VerifyString(desc()) &&
           VerifyField<int32_t>(verifier, VT_FRAME_START, 4) &&
           VerifyField<int32_t>(verifier, VT_FRAME_COUNT, 4) &&
           verifier.EndTable();
  }
};

struct AnimationBuilder {
  typedef Animation Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Animation::VT_NAME, name);
  }
  void add_desc(flatbuffers::Offset<flatbuffers::String> desc) {
    fbb_.AddOffset(Animation::VT_DESC, desc);
  }
  void add_frame_start(int32_t frame_start) {
    fbb_.AddElement<int32_t>(Animation::VT_FRAME_START, frame_start, 0);
  }
  void add_frame_count(int32_t frame_count) {
    fbb_.AddElement<int32_t>(Animation::VT_FRAME_COUNT, frame_count, 0);
  }
  explicit AnimationBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Animation> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Animation>(end);
    return o;
  }
};

inline flatbuffers::Offset<Animation> CreateAnimation(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> desc = 0,
    int32_t frame_start = 0,
    int32_t frame_count = 0) {
  AnimationBuilder builder_(_fbb);
  builder_.add_frame_count(frame_count);
  builder_.add_frame_start(frame_start);
  builder_.add_desc(desc);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<Animation> CreateAnimationDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *desc = nullptr,
    int32_t frame_start = 0,
    int32_t frame_count = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto desc__ = desc ? _fbb.CreateString(desc) : 0;
  return ResourceDB::CreateAnimation(
      _fbb,
      name__,
      desc__,
      frame_start,
      frame_count);
}

struct Spritesheet FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SpritesheetBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "ResourceDB.Spritesheet";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_TEXTURE_PATH = 6,
    VT_CELL_COUNT = 8,
    VT_CELL_WIDTH = 10,
    VT_CELL_HEIGHT = 12,
    VT_ANIMATIONS = 14
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  bool KeyCompareLessThan(const Spritesheet *o) const {
    return *name() < *o->name();
  }
  int KeyCompareWithValue(const char *_name) const {
    return strcmp(name()->c_str(), _name);
  }
  const flatbuffers::String *texture_path() const {
    return GetPointer<const flatbuffers::String *>(VT_TEXTURE_PATH);
  }
  int32_t cell_count() const {
    return GetField<int32_t>(VT_CELL_COUNT, 0);
  }
  int32_t cell_width() const {
    return GetField<int32_t>(VT_CELL_WIDTH, 0);
  }
  int32_t cell_height() const {
    return GetField<int32_t>(VT_CELL_HEIGHT, 0);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Animation>> *animations() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Animation>> *>(VT_ANIMATIONS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_TEXTURE_PATH) &&
           verifier.VerifyString(texture_path()) &&
           VerifyField<int32_t>(verifier, VT_CELL_COUNT, 4) &&
           VerifyField<int32_t>(verifier, VT_CELL_WIDTH, 4) &&
           VerifyField<int32_t>(verifier, VT_CELL_HEIGHT, 4) &&
           VerifyOffset(verifier, VT_ANIMATIONS) &&
           verifier.VerifyVector(animations()) &&
           verifier.VerifyVectorOfTables(animations()) &&
           verifier.EndTable();
  }
};

struct SpritesheetBuilder {
  typedef Spritesheet Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Spritesheet::VT_NAME, name);
  }
  void add_texture_path(flatbuffers::Offset<flatbuffers::String> texture_path) {
    fbb_.AddOffset(Spritesheet::VT_TEXTURE_PATH, texture_path);
  }
  void add_cell_count(int32_t cell_count) {
    fbb_.AddElement<int32_t>(Spritesheet::VT_CELL_COUNT, cell_count, 0);
  }
  void add_cell_width(int32_t cell_width) {
    fbb_.AddElement<int32_t>(Spritesheet::VT_CELL_WIDTH, cell_width, 0);
  }
  void add_cell_height(int32_t cell_height) {
    fbb_.AddElement<int32_t>(Spritesheet::VT_CELL_HEIGHT, cell_height, 0);
  }
  void add_animations(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Animation>>> animations) {
    fbb_.AddOffset(Spritesheet::VT_ANIMATIONS, animations);
  }
  explicit SpritesheetBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Spritesheet> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Spritesheet>(end);
    fbb_.Required(o, Spritesheet::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<Spritesheet> CreateSpritesheet(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> texture_path = 0,
    int32_t cell_count = 0,
    int32_t cell_width = 0,
    int32_t cell_height = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Animation>>> animations = 0) {
  SpritesheetBuilder builder_(_fbb);
  builder_.add_animations(animations);
  builder_.add_cell_height(cell_height);
  builder_.add_cell_width(cell_width);
  builder_.add_cell_count(cell_count);
  builder_.add_texture_path(texture_path);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<Spritesheet> CreateSpritesheetDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *texture_path = nullptr,
    int32_t cell_count = 0,
    int32_t cell_width = 0,
    int32_t cell_height = 0,
    const std::vector<flatbuffers::Offset<ResourceDB::Animation>> *animations = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto texture_path__ = texture_path ? _fbb.CreateString(texture_path) : 0;
  auto animations__ = animations ? _fbb.CreateVector<flatbuffers::Offset<ResourceDB::Animation>>(*animations) : 0;
  return ResourceDB::CreateSpritesheet(
      _fbb,
      name__,
      texture_path__,
      cell_count,
      cell_width,
      cell_height,
      animations__);
}

struct MaterialProto FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MaterialProtoBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "ResourceDB.MaterialProto";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_ATTRIBS = 6
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  bool KeyCompareLessThan(const MaterialProto *o) const {
    return *name() < *o->name();
  }
  int KeyCompareWithValue(const char *_name) const {
    return strcmp(name()->c_str(), _name);
  }
  ResourceDB::MaterialAttribs attribs() const {
    return static_cast<ResourceDB::MaterialAttribs>(GetField<uint32_t>(VT_ATTRIBS, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint32_t>(verifier, VT_ATTRIBS, 4) &&
           verifier.EndTable();
  }
};

struct MaterialProtoBuilder {
  typedef MaterialProto Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(MaterialProto::VT_NAME, name);
  }
  void add_attribs(ResourceDB::MaterialAttribs attribs) {
    fbb_.AddElement<uint32_t>(MaterialProto::VT_ATTRIBS, static_cast<uint32_t>(attribs), 0);
  }
  explicit MaterialProtoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<MaterialProto> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<MaterialProto>(end);
    fbb_.Required(o, MaterialProto::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<MaterialProto> CreateMaterialProto(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    ResourceDB::MaterialAttribs attribs = static_cast<ResourceDB::MaterialAttribs>(0)) {
  MaterialProtoBuilder builder_(_fbb);
  builder_.add_attribs(attribs);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<MaterialProto> CreateMaterialProtoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    ResourceDB::MaterialAttribs attribs = static_cast<ResourceDB::MaterialAttribs>(0)) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return ResourceDB::CreateMaterialProto(
      _fbb,
      name__,
      attribs);
}

struct CardProto FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef CardProtoBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "ResourceDB.CardProto";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_SPRITESHEET = 6,
    VT_DEFAULT_ANIMATION = 8,
    VT_MATERIAL_PROTO = 10,
    VT_EFFECTS = 12
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  bool KeyCompareLessThan(const CardProto *o) const {
    return *name() < *o->name();
  }
  int KeyCompareWithValue(const char *_name) const {
    return strcmp(name()->c_str(), _name);
  }
  const flatbuffers::String *spritesheet() const {
    return GetPointer<const flatbuffers::String *>(VT_SPRITESHEET);
  }
  const flatbuffers::String *default_animation() const {
    return GetPointer<const flatbuffers::String *>(VT_DEFAULT_ANIMATION);
  }
  const flatbuffers::String *material_proto() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL_PROTO);
  }
  const flatbuffers::Vector<uint32_t> *effects() const {
    return GetPointer<const flatbuffers::Vector<uint32_t> *>(VT_EFFECTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_SPRITESHEET) &&
           verifier.VerifyString(spritesheet()) &&
           VerifyOffset(verifier, VT_DEFAULT_ANIMATION) &&
           verifier.VerifyString(default_animation()) &&
           VerifyOffset(verifier, VT_MATERIAL_PROTO) &&
           verifier.VerifyString(material_proto()) &&
           VerifyOffset(verifier, VT_EFFECTS) &&
           verifier.VerifyVector(effects()) &&
           verifier.EndTable();
  }
};

struct CardProtoBuilder {
  typedef CardProto Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(CardProto::VT_NAME, name);
  }
  void add_spritesheet(flatbuffers::Offset<flatbuffers::String> spritesheet) {
    fbb_.AddOffset(CardProto::VT_SPRITESHEET, spritesheet);
  }
  void add_default_animation(flatbuffers::Offset<flatbuffers::String> default_animation) {
    fbb_.AddOffset(CardProto::VT_DEFAULT_ANIMATION, default_animation);
  }
  void add_material_proto(flatbuffers::Offset<flatbuffers::String> material_proto) {
    fbb_.AddOffset(CardProto::VT_MATERIAL_PROTO, material_proto);
  }
  void add_effects(flatbuffers::Offset<flatbuffers::Vector<uint32_t>> effects) {
    fbb_.AddOffset(CardProto::VT_EFFECTS, effects);
  }
  explicit CardProtoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<CardProto> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<CardProto>(end);
    fbb_.Required(o, CardProto::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<CardProto> CreateCardProto(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> spritesheet = 0,
    flatbuffers::Offset<flatbuffers::String> default_animation = 0,
    flatbuffers::Offset<flatbuffers::String> material_proto = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint32_t>> effects = 0) {
  CardProtoBuilder builder_(_fbb);
  builder_.add_effects(effects);
  builder_.add_material_proto(material_proto);
  builder_.add_default_animation(default_animation);
  builder_.add_spritesheet(spritesheet);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<CardProto> CreateCardProtoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *spritesheet = nullptr,
    const char *default_animation = nullptr,
    const char *material_proto = nullptr,
    const std::vector<uint32_t> *effects = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto spritesheet__ = spritesheet ? _fbb.CreateString(spritesheet) : 0;
  auto default_animation__ = default_animation ? _fbb.CreateString(default_animation) : 0;
  auto material_proto__ = material_proto ? _fbb.CreateString(material_proto) : 0;
  auto effects__ = effects ? _fbb.CreateVector<uint32_t>(*effects) : 0;
  return ResourceDB::CreateCardProto(
      _fbb,
      name__,
      spritesheet__,
      default_animation__,
      material_proto__,
      effects__);
}

struct Root FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RootBuilder Builder;
  static FLATBUFFERS_CONSTEXPR_CPP11 const char *GetFullyQualifiedName() {
    return "ResourceDB.Root";
  }
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_CARD_PROTOS = 6,
    VT_MATERIAL_PROTOS = 8,
    VT_SPRITESHEETS = 10
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::CardProto>> *card_protos() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::CardProto>> *>(VT_CARD_PROTOS);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::MaterialProto>> *material_protos() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::MaterialProto>> *>(VT_MATERIAL_PROTOS);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Spritesheet>> *spritesheets() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Spritesheet>> *>(VT_SPRITESHEETS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_CARD_PROTOS) &&
           verifier.VerifyVector(card_protos()) &&
           verifier.VerifyVectorOfTables(card_protos()) &&
           VerifyOffset(verifier, VT_MATERIAL_PROTOS) &&
           verifier.VerifyVector(material_protos()) &&
           verifier.VerifyVectorOfTables(material_protos()) &&
           VerifyOffset(verifier, VT_SPRITESHEETS) &&
           verifier.VerifyVector(spritesheets()) &&
           verifier.VerifyVectorOfTables(spritesheets()) &&
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
  void add_card_protos(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::CardProto>>> card_protos) {
    fbb_.AddOffset(Root::VT_CARD_PROTOS, card_protos);
  }
  void add_material_protos(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::MaterialProto>>> material_protos) {
    fbb_.AddOffset(Root::VT_MATERIAL_PROTOS, material_protos);
  }
  void add_spritesheets(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Spritesheet>>> spritesheets) {
    fbb_.AddOffset(Root::VT_SPRITESHEETS, spritesheets);
  }
  explicit RootBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Root> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Root>(end);
    return o;
  }
};

inline flatbuffers::Offset<Root> CreateRoot(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::CardProto>>> card_protos = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::MaterialProto>>> material_protos = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ResourceDB::Spritesheet>>> spritesheets = 0) {
  RootBuilder builder_(_fbb);
  builder_.add_spritesheets(spritesheets);
  builder_.add_material_protos(material_protos);
  builder_.add_card_protos(card_protos);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<Root> CreateRootDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    std::vector<flatbuffers::Offset<ResourceDB::CardProto>> *card_protos = nullptr,
    std::vector<flatbuffers::Offset<ResourceDB::MaterialProto>> *material_protos = nullptr,
    std::vector<flatbuffers::Offset<ResourceDB::Spritesheet>> *spritesheets = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto card_protos__ = card_protos ? _fbb.CreateVectorOfSortedTables<ResourceDB::CardProto>(card_protos) : 0;
  auto material_protos__ = material_protos ? _fbb.CreateVectorOfSortedTables<ResourceDB::MaterialProto>(material_protos) : 0;
  auto spritesheets__ = spritesheets ? _fbb.CreateVectorOfSortedTables<ResourceDB::Spritesheet>(spritesheets) : 0;
  return ResourceDB::CreateRoot(
      _fbb,
      name__,
      card_protos__,
      material_protos__,
      spritesheets__);
}

inline const ResourceDB::Root *GetRoot(const void *buf) {
  return flatbuffers::GetRoot<ResourceDB::Root>(buf);
}

inline const ResourceDB::Root *GetSizePrefixedRoot(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<ResourceDB::Root>(buf);
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
  return verifier.VerifyBuffer<ResourceDB::Root>(RootIdentifier());
}

inline bool VerifySizePrefixedRootBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<ResourceDB::Root>(RootIdentifier());
}

inline const char *RootExtension() {
  return "fbb";
}

inline void FinishRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<ResourceDB::Root> root) {
  fbb.Finish(root, RootIdentifier());
}

inline void FinishSizePrefixedRootBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<ResourceDB::Root> root) {
  fbb.FinishSizePrefixed(root, RootIdentifier());
}

}  // namespace ResourceDB

#endif  // FLATBUFFERS_GENERATED_RESOURCEDB_RESOURCEDB_H_
