#include "mode.h"
#include "modemanager.h"

namespace schrandr {
    
    ModeManager::ModeManager()
    {}
    
    Mode ModeManager::get_current_mode()
    {
        current_mode_ = xmanager_.get_mode();
        return current_mode_;
    }
    
    void ModeManager::set_mode_(Mode mode)
    {}
} 
