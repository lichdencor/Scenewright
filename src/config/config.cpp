#include "config/config.h"

#include <string>

#include <SimpleIni.h>

namespace SW::Config {

bool IsDevModeEnabled(std::string_view iniPath) {
    CSimpleIniA ini;
    ini.SetUnicode();

    if (ini.LoadFile(std::string(iniPath).c_str()) < 0) {
        return false;
    }
    return ini.GetBoolValue("Debug", "EnableLiveLink", false);
}

}  // namespace SW::Config
