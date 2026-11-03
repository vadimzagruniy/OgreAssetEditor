#include "PassPropertySource.h"
#include "editor/inspector/PropertySchema.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialState.h"
#include "modules/material/presenter/IMaterialEditorPresenter.h"

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

namespace editor
{

namespace
{

struct TBlendOption
{
  const char *           Label;
  Ogre::SceneBlendFactor SourceBlend;
  Ogre::SceneBlendFactor DestBlend;
};

struct TCullingOption
{
  const char *      Label;
  Ogre::CullingMode Culling;
};

constexpr std::array<TBlendOption, 3> BlendOptions{{
  {"Opaque",      Ogre::SBF_ONE,          Ogre::SBF_ZERO                  },
  {"Alpha Blend", Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA},
  {"Additive",    Ogre::SBF_ONE,          Ogre::SBF_ONE                   }
}};

constexpr std::array<TCullingOption, 3> CullingOptions{{
  {"Back Faces",  Ogre::CULL_CLOCKWISE    },
  {"Front Faces", Ogre::CULL_ANTICLOCKWISE},
  {"None",        Ogre::CULL_NONE         }
}};

const std::string AmbientIdentifier    = "Lighting.Ambient";
const std::string DiffuseIdentifier    = "Lighting.Diffuse";
const std::string SpecularIdentifier   = "Lighting.Specular";
const std::string EmissiveIdentifier   = "Lighting.Emissive";
const std::string ShininessIdentifier  = "Lighting.Shininess";
const std::string BlendingIdentifier   = "Output.Blending";
const std::string DepthWriteIdentifier = "Output.DepthWrite";
const std::string CullingIdentifier    = "Output.Culling";

template <class TOptions>
std::vector<std::string> CollectLabels(const TOptions & _Options)
{
  std::vector<std::string> Labels;
  Labels.reserve(_Options.size());

  for (const auto & Option : _Options)
    Labels.emplace_back(Option.Label);

  return Labels;
}

template <class TField>
bool AssignValue(TField & _Field, const TPropertyValue & _Value)
{
  const auto * Value = std::get_if<TField>(&_Value);
  if (!Value)
    return false;

  _Field = *Value;
  return true;
}

TPropertySchema MakePassSchema()
{
  TPropertyDesc Lighting{"Lighting", "Lighting", EPropertyType::Group};
  Lighting.Children.push_back({AmbientIdentifier,   "Ambient",   EPropertyType::Color, {}});
  Lighting.Children.push_back({DiffuseIdentifier,   "Diffuse",   EPropertyType::Color, {.IsAlphaEnabled = true}});
  Lighting.Children.push_back({SpecularIdentifier,  "Specular",  EPropertyType::Color, {}});
  Lighting.Children.push_back({EmissiveIdentifier,  "Emissive",  EPropertyType::Color, {}});
  Lighting.Children.push_back({ShininessIdentifier, "Shininess", EPropertyType::Float, {.Minimum = 0.0f, .Maximum = 128.0f, .Speed = 0.5f, .Format = "%.1f"}});

  TPropertyDesc Output{"Output", "Output", EPropertyType::Group};
  Output.Children.push_back({BlendingIdentifier,   "Blending",    EPropertyType::Enum, {.Hint = "Custom", .Options = CollectLabels(BlendOptions)}});
  Output.Children.push_back({DepthWriteIdentifier, "Depth Write", EPropertyType::Bool, {}});
  Output.Children.push_back({CullingIdentifier,    "Culling",     EPropertyType::Enum, {.Hint = "Unknown", .Options = CollectLabels(CullingOptions)}});

  return {"material.pass", {std::move(Lighting), std::move(Output)}};
}

Ogre::ColourValue TPassState::* FindColourField(const std::string & _Identifier)
{
  if (_Identifier == AmbientIdentifier)
    return &TPassState::Ambient;

  if (_Identifier == DiffuseIdentifier)
    return &TPassState::Diffuse;

  if (_Identifier == SpecularIdentifier)
    return &TPassState::Specular;

  if (_Identifier == EmissiveIdentifier)
    return &TPassState::Emissive;

  return nullptr;
}

TPropertyValue FindBlendIndex(const TPassState & _State)
{
  for (std::size_t OptionIndex = 0; OptionIndex < BlendOptions.size(); ++OptionIndex)
  {
    const auto & Option = BlendOptions[OptionIndex];
    if (Option.SourceBlend == _State.SourceBlend && Option.DestBlend == _State.DestBlend)
      return OptionIndex;
  }

  return {};
}

TPropertyValue FindCullingIndex(const TPassState & _State)
{
  for (std::size_t OptionIndex = 0; OptionIndex < CullingOptions.size(); ++OptionIndex)
  {
    if (CullingOptions[OptionIndex].Culling == _State.Culling)
      return OptionIndex;
  }

  return {};
}

bool ApplyBlending(TPassState & _State, const TPropertyValue & _Value)
{
  const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
  if (!OptionIndex || *OptionIndex >= BlendOptions.size())
    return false;

  _State.SourceBlend = BlendOptions[*OptionIndex].SourceBlend;
  _State.DestBlend   = BlendOptions[*OptionIndex].DestBlend;
  return true;
}

bool ApplyCulling(TPassState & _State, const TPropertyValue & _Value)
{
  const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
  if (!OptionIndex || *OptionIndex >= CullingOptions.size())
    return false;

  _State.Culling = CullingOptions[*OptionIndex].Culling;
  return true;
}

bool ApplyValue(TPassState & _State, const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (const auto ColourField = FindColourField(_Identifier))
    return AssignValue(_State.*ColourField, _Value);

  if (_Identifier == ShininessIdentifier)
    return AssignValue(_State.Shininess, _Value);

  if (_Identifier == DepthWriteIdentifier)
    return AssignValue(_State.IsDepthWriteEnabled, _Value);

  if (_Identifier == BlendingIdentifier)
    return ApplyBlending(_State, _Value);

  if (_Identifier == CullingIdentifier)
    return ApplyCulling(_State, _Value);

  return false;
}

} // namespace

//
// Constructor
//

CPassPropertySource::CPassPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter):
  CMaterialPropertySource(std::move(_Presenter))
{
  // Empty
}

//
// IPropertySource
//

const TPropertySchema & CPassPropertySource::GetSchema() const
{
  static const TPropertySchema Schema = MakePassSchema();
  return Schema;
}

TPropertyValue CPassPropertySource::GetValue(const std::string & _Identifier) const
{
  const auto State = GetState();
  if (!State)
    return {};

  if (const auto ColourField = FindColourField(_Identifier))
    return (*State).*ColourField;

  if (_Identifier == ShininessIdentifier)
    return State->Shininess;

  if (_Identifier == DepthWriteIdentifier)
    return State->IsDepthWriteEnabled;

  if (_Identifier == BlendingIdentifier)
    return FindBlendIndex(*State);

  if (_Identifier == CullingIdentifier)
    return FindCullingIndex(*State);

  return {};
}

void CPassPropertySource::Preview(const std::string & _Identifier, const TPropertyValue & _Value)
{
  auto State = GetState();
  if (!State || !ApplyValue(*State, _Identifier, _Value))
    return;

  GetPresenter().Preview(GetDocument()->GetSelection(), *State);
}

//
// Implementation
//

std::optional<TPassState> CPassPropertySource::GetState() const
{
  const auto Document = GetDocument();
  if (!Document)
    return std::nullopt;

  return Document->GetPassState(Document->GetSelection());
}

} // namespace editor
