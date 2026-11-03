#include "TextureUnitPropertySource.h"
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

struct TAddressOption
{
  const char *                Label;
  Ogre::TextureAddressingMode AddressMode;
};

constexpr std::array<TAddressOption, 3> AddressOptions{{
  {"Wrap",   Ogre::TAM_WRAP  },
  {"Clamp",  Ogre::TAM_CLAMP },
  {"Mirror", Ogre::TAM_MIRROR}
}};

const std::string ImageIdentifier      = "Texture.Image";
const std::string AddressingIdentifier = "Texture.Addressing";

TPropertySchema MakeTextureUnitSchema()
{
  std::vector<std::string> AddressLabels;
  for (const auto & Option : AddressOptions)
    AddressLabels.emplace_back(Option.Label);

  TPropertyDesc Texture{"Texture", "Texture", EPropertyType::Group};
  Texture.Children.push_back({ImageIdentifier,      "Image",      EPropertyType::Asset, {.AssetType = "texture"}});
  Texture.Children.push_back({AddressingIdentifier, "Addressing", EPropertyType::Enum,  {.Hint = "Custom", .Options = std::move(AddressLabels)}});

  return {"material.texture_unit", {std::move(Texture)}};
}

TPropertyValue FindAddressIndex(const TTextureUnitState & _State)
{
  if (_State.AddressU != _State.AddressV || _State.AddressU != _State.AddressW)
    return {};

  for (std::size_t OptionIndex = 0; OptionIndex < AddressOptions.size(); ++OptionIndex)
  {
    if (AddressOptions[OptionIndex].AddressMode == _State.AddressU)
      return OptionIndex;
  }

  return {};
}

bool ApplyAddressing(TTextureUnitState & _State, const TPropertyValue & _Value)
{
  const auto * OptionIndex = std::get_if<std::size_t>(&_Value);
  if (!OptionIndex || *OptionIndex >= AddressOptions.size())
    return false;

  const auto AddressMode = AddressOptions[*OptionIndex].AddressMode;

  _State.AddressU = AddressMode;
  _State.AddressV = AddressMode;
  _State.AddressW = AddressMode;
  return true;
}

bool ApplyValue(TTextureUnitState & _State, const std::string & _Identifier, const TPropertyValue & _Value)
{
  if (_Identifier == AddressingIdentifier)
    return ApplyAddressing(_State, _Value);

  const auto * TextureName = std::get_if<std::string>(&_Value);
  if (_Identifier != ImageIdentifier || !TextureName)
    return false;

  _State.TextureName = *TextureName;
  return true;
}

} // namespace

//
// Constructor
//

CTextureUnitPropertySource::CTextureUnitPropertySource(std::shared_ptr<IMaterialEditorPresenter> _Presenter):
  CMaterialPropertySource(std::move(_Presenter))
{
  // Empty
}

//
// IPropertySource
//

const TPropertySchema & CTextureUnitPropertySource::GetSchema() const
{
  static const TPropertySchema Schema = MakeTextureUnitSchema();
  return Schema;
}

TPropertyValue CTextureUnitPropertySource::GetValue(const std::string & _Identifier) const
{
  const auto State = GetState();
  if (!State)
    return {};

  if (_Identifier == ImageIdentifier)
    return State->TextureName;

  if (_Identifier == AddressingIdentifier)
    return FindAddressIndex(*State);

  return {};
}

void CTextureUnitPropertySource::Preview(const std::string & _Identifier, const TPropertyValue & _Value)
{
  auto State = GetState();
  if (!State || !ApplyValue(*State, _Identifier, _Value))
    return;

  GetPresenter().Preview(GetDocument()->GetSelection(), *State);
}

//
// Implementation
//

std::optional<TTextureUnitState> CTextureUnitPropertySource::GetState() const
{
  const auto Document = GetDocument();
  if (!Document)
    return std::nullopt;

  return Document->GetTextureUnitState(Document->GetSelection());
}

} // namespace editor
