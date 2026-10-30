#include "MaterialInspectorWindow.h"
#include "editor/assets/AssetManager.h"
#include "editor/assets/AssetNode.h"
#include "modules/material/MaterialDocument.h"
#include "modules/material/MaterialEditingSession.h"
#include "modules/material/MaterialInstance.h"
#include "modules/material/MaterialNode.h"
#include "modules/material/MaterialState.h"
#include "ui/ToolBoxDesc.h"

#include <OgreCodec.h>
#include <OgreImageCodec.h>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string_view>
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

const char * GetBlendLabel(const TPassState & _State)
{
  for (const auto & Option : BlendOptions)
  {
    if (Option.SourceBlend == _State.SourceBlend && Option.DestBlend == _State.DestBlend)
      return Option.Label;
  }

  return "Custom";
}

const char * GetCullingLabel(Ogre::CullingMode _Culling)
{
  for (const auto & Option : CullingOptions)
  {
    if (Option.Culling == _Culling)
      return Option.Label;
  }

  return "Unknown";
}

const char * GetAddressLabel(const TTextureUnitState & _State)
{
  if (_State.AddressU != _State.AddressV || _State.AddressU != _State.AddressW)
    return "Mixed";

  for (const auto & Option : AddressOptions)
  {
    if (Option.AddressMode == _State.AddressU)
      return Option.Label;
  }

  return "Border";
}

template <class TOptions, class TGetLabel>
std::optional<std::size_t> DrawCombo(const char * _Label, const char * _Preview, const TOptions & _Options, TGetLabel _GetLabel)
{
  if (!ImGui::BeginCombo(_Label, _Preview))
    return std::nullopt;

  std::optional<std::size_t> Choice;

  for (std::size_t OptionIndex = 0; OptionIndex < _Options.size(); ++OptionIndex)
  {
    const char * OptionLabel = _GetLabel(_Options[OptionIndex]);
    const bool   IsSelected  = std::string_view(OptionLabel) == _Preview;

    if (ImGui::Selectable(OptionLabel, IsSelected) && !IsSelected)
      Choice = OptionIndex;
  }

  ImGui::EndCombo();

  return Choice;
}

template <class TOption>
const char * GetOptionLabel(const TOption & _Option)
{
  return _Option.Label;
}

const char * GetNameLabel(const std::string & _Name)
{
  return _Name.c_str();
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

CMaterialInspectorWindow::CMaterialInspectorWindow(const ui::TToolBoxDesc & _Desc, std::shared_ptr<CMaterialEditingSession> _Session, std::shared_ptr<CAssetManager> _AssetManager):
  CDockedWindow(_Desc.Title, "MaterialInspector", _Desc.Visible),
  m_Session(std::move(_Session)),
  m_AssetManager(std::move(_AssetManager))
{
  // Empty
}

//
// CWindow
//

void CMaterialInspectorWindow::OnGui()
{
  const auto Document = m_Session->GetDocument();
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

  if (auto PassState = Document->GetPassState(Node))
  {
    if (DrawPassState(*PassState))
      Document->Preview(Node, *PassState);
  }
  else
  if (auto TextureUnitState = Document->GetTextureUnitState(Node))
  {
    if (DrawTextureUnitState(*TextureUnitState))
      Document->Preview(Node, *TextureUnitState);
  }
  else
  {
    ImGui::TextDisabled("Select a pass or a texture in the outline.");
  }

  if (!ImGui::IsAnyItemActive() && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
    Document->CommitPreview();
}

//
// Implementation
//

bool CMaterialInspectorWindow::DrawPassState(TPassState & _State)
{
  bool IsChanged = false;

  ImGui::SeparatorText("Lighting");

  IsChanged |= ImGui::ColorEdit3("Ambient", _State.Ambient.ptr());
  IsChanged |= ImGui::ColorEdit4("Diffuse", _State.Diffuse.ptr());
  IsChanged |= ImGui::ColorEdit3("Specular", _State.Specular.ptr());
  IsChanged |= ImGui::ColorEdit3("Emissive", _State.Emissive.ptr());
  IsChanged |= ImGui::DragFloat("Shininess", &_State.Shininess, 0.5f, 0.0f, 128.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

  ImGui::SeparatorText("Output");

  if (const auto Choice = DrawCombo("Blending", GetBlendLabel(_State), BlendOptions, GetOptionLabel<TBlendOption>))
  {
    _State.SourceBlend = BlendOptions[*Choice].SourceBlend;
    _State.DestBlend   = BlendOptions[*Choice].DestBlend;
    IsChanged          = true;
  }

  IsChanged |= ImGui::Checkbox("Depth Write", &_State.IsDepthWriteEnabled);

  if (const auto Choice = DrawCombo("Culling", GetCullingLabel(_State.Culling), CullingOptions, GetOptionLabel<TCullingOption>))
  {
    _State.Culling = CullingOptions[*Choice].Culling;
    IsChanged      = true;
  }

  return IsChanged;
}

bool CMaterialInspectorWindow::DrawTextureUnitState(TTextureUnitState & _State) const
{
  bool IsChanged = false;

  ImGui::SeparatorText("Texture");

  const char * ImagePreview = _State.TextureName.empty() ? "<none>" : _State.TextureName.c_str();
  const auto   TextureNames = CollectTextureNames();

  if (const auto Choice = DrawCombo("Image", ImagePreview, TextureNames, GetNameLabel))
  {
    _State.TextureName = TextureNames[*Choice];
    IsChanged          = true;
  }

  if (const auto Choice = DrawCombo("Addressing", GetAddressLabel(_State), AddressOptions, GetOptionLabel<TAddressOption>))
  {
    _State.AddressU = AddressOptions[*Choice].AddressMode;
    _State.AddressV = AddressOptions[*Choice].AddressMode;
    _State.AddressW = AddressOptions[*Choice].AddressMode;
    IsChanged       = true;
  }

  return IsChanged;
}

std::vector<std::string> CMaterialInspectorWindow::CollectTextureNames() const
{
  std::vector<std::string> Names;
  CollectImages(m_AssetManager->GetRootNode(), Names);

  std::sort(Names.begin(), Names.end());

  return Names;
}

} // namespace editor
