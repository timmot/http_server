// Copyright (c) 2022 - Tim Blackstone

#pragma once

#if __has_include(<magic.h>) && USE_LIBMAGIC
#include <magic.h>
#endif
#include <optional>
#include <string>
#include <string_view>

class MimeTypeDatabase {
public:
    MimeTypeDatabase()
    {
#if __has_include(<magic.h>) && USE_LIBMAGIC
        m_magic = magic_open(MAGIC_MIME_TYPE);
        magic_load(m_magic, "/usr/share/misc/magic");
#endif
    }

    ~MimeTypeDatabase()
    {
#if __has_include(<magic.h>) && USE_LIBMAGIC
        magic_close(m_magic);
#endif
    }

    std::optional<std::string> detect_mime_type(std::string_view filename)
    {
#if __has_include(<magic.h>) && USE_LIBMAGIC
        auto result = magic_file(m_magic, std::string(filename).c_str());

        if (!result)
            return {};

        return result;
#else
        return {};
#endif
    }

private:
#if __has_include(<magic.h>) && USE_LIBMAGIC
    magic_t m_magic;
#endif
};