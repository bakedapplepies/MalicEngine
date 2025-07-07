#include "Engine/core/Filesystem.h"

#include <fmt/format.h>

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

File::File(const char* path)
{
    m_path = std::filesystem::path(MLC_ROOT_DIR);
    m_path /= path;

    MLC_ASSERT(m_path.has_filename(), fmt::format("{} is not a file.", path));
}

const char* File::GetPath() const
{
    return m_path.c_str();
}

MLC_NAMESPACE_END