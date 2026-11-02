#include "MaterialInspectorWindow.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetNode.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialInstance.h"
#include "modules/material/MaterialNode.h"
#include "modules/material/MaterialState.h"
#include "modules/material/presenter/IMaterialEditorPresenter.h"
#include "ui/ToolBoxDesc.h"

#include <OgreCodec.h>
#include <OgreImageCodec.h>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <optional>
#include <utility>

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

struct TAddressOption
{
  const char *                Label;
  Ogre::TextureAddressingMode AddressMode;
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

constexpr std::array<TAddressOption, 3> AddressOptions{{
  {"Wrap",   Ogre::TAM_WRAP  },
  {"Clamp",  Ogre::TAM_CLAMP },
  {"Mirror", Ogre::TAM_MIRROR}
}};

template <class TOptions>
std::vector<std::string> CollectLabels(const TOptions & _Options)
{
  std::vector<std::string> Labels;
  Labels.reserve(_Options.size());

  for (const auto & Option : _Options)
    Labels.emplace_back(Option.Label);

  return Labels;
}

std::optional<std::size_t> FindBlendIndex(const TPassState & _State)
{
  for (std::size_t OptionIndex = 0; OptionIndex < BlendOptions.size(); ++OptionIndex)
  {
    const auto & Option = BlendOptions[OptionIndex];
    if (Option.SourceBlend == _State.SourceBlend && Option.DestBlend == _State.DestBlend)
      return OptionIndex;
  }

  return std::nullopt;
}

std::optional<std::size_t> FindCullingIndex(Ogre::CullingMode _Culling)
{
  for (std::size_t OptionIndex = 0; OptionIndex < CullingOptions.size(); ++OptionIndex)
  {
    if (CullingOptions[OptionIndex].Culling == _Culling)
      return OptionIndex;
  }

  return std::nullopt;
}

bool IsAddressingMixed(const TTextureUnitState & _State)
{
  return _State.AddressU != _State.AddressV || _State.AddressU != _State.AddressW;
}

std::optional<std::size_t> FindAddressIndex(const TTextureUnitState & _State)
{
  if (IsAddressingMixed(_State))
    return std::nullopt;

  for (std::size_t OptionIndex = 0; OptionIndex < AddressOptions.size(); ++OptionIndex)
  {
    if (AddressOptions[OptionIndex].AddressMode == _State.AddressU)
      return OptionIndex;
  }

  return std::nullopt;
}

std::optional<std::size_t> FindNameIndex(const std::vector<std::string> & _Names, const std::string & _Name)
{
  const auto NameIterator = std::ranges::find(_Names, _Name);
  if (NameIterator == _Names.end())
    return std::nullopt;

  return static_cast<std::size_t>(std::distance(_Names.begin(), NameIterator));
}

ImVec4 ToImVec4(const Ogre::ColourValue & _Colour)
{
  return ImVec4(_Colour.r, _Colour.g, _Colour.b, _Colour.a);
}

Ogre::ColourValue ToColourValue(const ImVec4 & _Colour)
{
  return Ogre::ColourValue(_Colour.x, _Colour.y, _Colour.z, _Colour.w);
}

bool IsImageFile(const std::filesystem::path & _Path)
{
  std::string Extension = _Path.extension().string();
  if (Extension.size() < 2)
    return false;

  Extension.erase(0, 1);
  std::transform(Extension.begin(), Extension.end(), Extension.begin(), [](unsigned char _Character)
  {
    return static_cast<char>(std::tolower(_Character));
  });

  return Ogre::Codec::isCodecRegistered(Extension) && dynamic_cast<Ogre::ImageCodec *>(Ogre::Codec::getCodec(Extension));
}

void CollectImages(const TAssetNode & _Folder, std::vector<std::string> & _Names)
{
  for (const auto & Child : _Folder.Children)
  {
    if (Child.IsDirectory)
      CollectImages(Child, _Names);
    else
    if (IsImageFile(Child.Record.RelativePath))
      _Names.push_back(Child.Record.RelativePath.generic_string());
  }
}

} // namespace

//
// Constructor
//

CMaterialInspectorWindow::CMaterialInspectorWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<IMaterialEditorPresenter> _Presenter, std::shared_ptr<CAssetManager> _AssetManager):
  CDockedWindow(_Desc.Title, "MaterialInspector", _Desc.Visible),
  m_Presenter(std::move(_Presenter)),
  m_AssetManager(std::move(_AssetManager)),
  m_AmbientEdit("Ambient"),
  m_DiffuseEdit("Diffuse"),
  m_SpecularEdit("Specular"),
  m_EmissiveEdit("Emissive"),
  m_ShininessDrag("Shininess"),
  m_BlendingCombo("Blending"),
  m_DepthWriteCheck("Depth Write"),
  m_CullingCombo("Culling"),
  m_ImageCombo("Image"),
  m_AddressingCombo("Addressing")
{
  SetupPassElements();
  SetupTextureUnitElements();
}

//
// CWindow
//

void CMaterialInspectorWindow::OnGui()
{
  const auto Document = m_Presenter->GetDocument();
  if (!Document)
  {
    ImGui::TextUnformatted("No material is open");
    return;
  }

  const TMaterialNode Node = Document->GetSelection();
  if (Node.MaterialIndex >= 0)
  {
    ImGui::TextUnformatted(Document->GetInstance().GetMaterialName(static_cast<std::size_t>(Node.MaterialIndex)).c_str());
    ImGui::Separator();
  }

  if (const auto PassState = Document->GetPassState(Node))
  {
    DrawPassState(*PassState);
  }
  else
  if (const auto TextureUnitState = Document->GetTextureUnitState(Node))
  {
    DrawTextureUnitState(*TextureUnitState);
  }
  else
  {
    ImGui::TextDisabled("Select a pass or a texture in the outline.");
  }
}

//
// Implementation
//

void CMaterialInspectorWindow::SetupPassElements()
{
  const auto BindColor = [this](ui::CColorEdit & _Edit, Ogre::ColourValue TPassState::* _Field, bool _IsAlphaEnabled)
  {
    _Edit.SetAlphaEnabled(_IsAlphaEnabled).OnChanged([this, _Field](const ImVec4 & _Colour)
    {
      EditPassState([&_Colour, _Field](TPassState & _State)
      {
        _State.*_Field = ToColourValue(_Colour);
      });
    }).OnEditEnd([this]
    {
      CommitEdit();
    });
  };

  BindColor(m_AmbientEdit,  &TPassState::Ambient,  false);
  BindColor(m_DiffuseEdit,  &TPassState::Diffuse,  true);
  BindColor(m_SpecularEdit, &TPassState::Specular, false);
  BindColor(m_EmissiveEdit, &TPassState::Emissive, false);

  m_ShininessDrag.SetRange(0.0f, 128.0f).SetSpeed(0.5f).SetFormat("%.1f").OnChanged([this](float _Shininess)
  {
    EditPassState([_Shininess](TPassState & _State)
    {
      _State.Shininess = _Shininess;
    });
  }).OnEditEnd([this]
  {
    CommitEdit();
  });

  m_BlendingCombo.SetItems(CollectLabels(BlendOptions)).SetPlaceholder("Custom").OnSelected([this](std::size_t _OptionIndex)
  {
    EditPassState([_OptionIndex](TPassState & _State)
    {
      _State.SourceBlend = BlendOptions[_OptionIndex].SourceBlend;
      _State.DestBlend   = BlendOptions[_OptionIndex].DestBlend;
    });

    CommitEdit();
  });

  m_DepthWriteCheck.OnToggled([this](bool _IsEnabled)
  {
    EditPassState([_IsEnabled](TPassState & _State)
    {
      _State.IsDepthWriteEnabled = _IsEnabled;
    });

    CommitEdit();
  });

  m_CullingCombo.SetItems(CollectLabels(CullingOptions)).SetPlaceholder("Unknown").OnSelected([this](std::size_t _OptionIndex)
  {
    EditPassState([_OptionIndex](TPassState & _State)
    {
      _State.Culling = CullingOptions[_OptionIndex].Culling;
    });

    CommitEdit();
  });
}

void CMaterialInspectorWindow::SetupTextureUnitElements()
{
  m_ImageCombo.OnSelected([this](std::size_t _NameIndex)
  {
    const auto & Names = m_ImageCombo.GetItems();
    if (_NameIndex >= Names.size())
      return;

    EditTextureUnitState([&Name = Names[_NameIndex]](TTextureUnitState & _State)
    {
      _State.TextureName = Name;
    });

    CommitEdit();
  });

  m_AddressingCombo.SetItems(CollectLabels(AddressOptions)).OnSelected([this](std::size_t _OptionIndex)
  {
    EditTextureUnitState([_OptionIndex](TTextureUnitState & _State)
    {
      _State.AddressU = AddressOptions[_OptionIndex].AddressMode;
      _State.AddressV = AddressOptions[_OptionIndex].AddressMode;
      _State.AddressW = AddressOptions[_OptionIndex].AddressMode;
    });

    CommitEdit();
  });
}

void CMaterialInspectorWindow::DrawPassState(const TPassState & _State)
{
  m_AmbientEdit.SetValue(ToImVec4(_State.Ambient));
  m_DiffuseEdit.SetValue(ToImVec4(_State.Diffuse));
  m_SpecularEdit.SetValue(ToImVec4(_State.Specular));
  m_EmissiveEdit.SetValue(ToImVec4(_State.Emissive));
  m_ShininessDrag.SetValue(_State.Shininess);
  m_BlendingCombo.SetSelectedIndex(FindBlendIndex(_State));
  m_DepthWriteCheck.SetChecked(_State.IsDepthWriteEnabled);
  m_CullingCombo.SetSelectedIndex(FindCullingIndex(_State.Culling));

  ImGui::SeparatorText("Lighting");

  m_AmbientEdit.Draw();
  m_DiffuseEdit.Draw();
  m_SpecularEdit.Draw();
  m_EmissiveEdit.Draw();
  m_ShininessDrag.Draw();

  ImGui::SeparatorText("Output");

  m_BlendingCombo.Draw();
  m_DepthWriteCheck.Draw();
  m_CullingCombo.Draw();
}

void CMaterialInspectorWindow::DrawTextureUnitState(const TTextureUnitState & _State)
{
  auto TextureNames = CollectTextureNames();
  const auto NameIndex = FindNameIndex(TextureNames, _State.TextureName);

  m_ImageCombo.SetItems(std::move(TextureNames)).SetSelectedIndex(NameIndex).SetPlaceholder(_State.TextureName.empty() ? "<none>" : _State.TextureName);
  m_AddressingCombo.SetSelectedIndex(FindAddressIndex(_State)).SetPlaceholder(IsAddressingMixed(_State) ? "Mixed" : "Border");

  ImGui::SeparatorText("Texture");

  m_ImageCombo.Draw();
  m_AddressingCombo.Draw();
}

void CMaterialInspectorWindow::EditPassState(const std::function<void(TPassState &)> & _Edit) const
{
  const auto Document = m_Presenter->GetDocument();
  if (!Document)
    return;

  const TMaterialNode Node  = Document->GetSelection();
  auto                State = Document->GetPassState(Node);
  if (!State)
    return;

  _Edit(*State);
  m_Presenter->Preview(Node, *State);
}

void CMaterialInspectorWindow::EditTextureUnitState(const std::function<void(TTextureUnitState &)> & _Edit) const
{
  const auto Document = m_Presenter->GetDocument();
  if (!Document)
    return;

  const TMaterialNode Node  = Document->GetSelection();
  auto                State = Document->GetTextureUnitState(Node);
  if (!State)
    return;

  _Edit(*State);
  m_Presenter->Preview(Node, *State);
}

void CMaterialInspectorWindow::CommitEdit() const
{
  m_Presenter->CommitEdit();
}

std::vector<std::string> CMaterialInspectorWindow::CollectTextureNames() const
{
  std::vector<std::string> Names;
  CollectImages(m_AssetManager->GetRootNode(), Names);

  std::sort(Names.begin(), Names.end());

  return Names;
}

} // namespace editor
