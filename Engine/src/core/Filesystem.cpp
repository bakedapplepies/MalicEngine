#include "Engine/core/Filesystem.h"

#include <filesystem>

#include <fmt/format.h>

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

File::File(const char* path)
{
    std::filesystem::path fpath;
    fpath = std::filesystem::path(MLC_ROOT_DIR);
    fpath /= path;
    fpath = fpath.make_preferred();

    MLC_ASSERT(fpath.has_filename(), fmt::format("{} is not a file.", path));

    m_path = fpath.string();
}

const char* File::GetPath() const
{
    return m_path.c_str();
}

MLC_NAMESPACE_END