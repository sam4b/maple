#include "imfilebrowser.hpp"
#include <Windows.h>

std::uint32_t ImGui::FileBrowser::GetDrivesBitMask()
{
    const DWORD mask = GetLogicalDrives();
    std::uint32_t ret = 0;
    for (int i = 0; i < 26; ++i)
    {
        if (!(mask & (1 << i)))
        {
            continue;
        }
        const char rootName[4] = { static_cast<char>('A' + i), ':', '\\', '\0' };
        const UINT type = GetDriveTypeA(rootName);
        if (type == DRIVE_REMOVABLE || type == DRIVE_FIXED || type == DRIVE_REMOTE)
        {
            ret |= (1 << i);
        }
    }
    return ret;
}
