// Copyright (c) 2022 - Tim Blackstone

#pragma once

#include <magic.h>
#include <optional>
#include <string>
#include <string_view>

class MimeTypeDatabase {
public:
    MimeTypeDatabase()
    {
        m_magic = magic_open(MAGIC_MIME_TYPE);
        magic_load(m_magic, "/usr/share/misc/magic");
    }

    ~MimeTypeDatabase()
    {
        magic_close(m_magic);
    }

    std::optional<std::string> detect_mime_type(std::string_view filename)
    {
        auto result = magic_file(m_magic, std::string(filename).c_str());

        if (!result)
            return {};

        return result;
    }

private:
    magic_t m_magic;
};