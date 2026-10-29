#pragma once

namespace editor::ui
{

class CModalWindow
{
public: // Destructor

  virtual ~CModalWindow();

public: // Interface

  // Return false to close and remove the modal.
  virtual bool Draw() = 0;
};

} // namespace editor::ui
