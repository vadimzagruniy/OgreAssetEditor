#include "platform/Application.h"

#include <OgreRoot.h>

#include <filesystem>
#include <iostream>
#include <string>

int main(int _Argc, char ** _Argv)
{
  const auto ProjectRoot = _Argc > 1 ? std::filesystem::path(_Argv[1]) : std::filesystem::current_path() / "sample_project";

  editor::platform::CApplication Application(ProjectRoot);
  std::string                    Error;

  if (!Application.Initialize(Error))
  {
    std::cerr << "Ogre Resource Editor failed: " << Error << '\n';
    return 1;
  }

  Application.initApp();
  Application.getRoot()->startRendering();
  Application.closeApp();

  return 0;
}
