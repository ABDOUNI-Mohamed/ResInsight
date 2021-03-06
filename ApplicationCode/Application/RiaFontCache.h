/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cafFixedAtlasFont.h"

#include "cvfObject.h"

#include <map>

namespace caf
{
template <typename T>
class AppEnum;
}

class RimSummaryCaseCollection;

//==================================================================================================
///
//==================================================================================================
class RiaFontCache
{
public:
    enum FontSize
    {
        INVALID       = -1,
        MIN_FONT_SIZE = 8,
        FONT_SIZE_8   = 8,
        FONT_SIZE_10  = 10,
        FONT_SIZE_12  = 12,
        FONT_SIZE_14  = 14,
        FONT_SIZE_16  = 16,
        FONT_SIZE_24  = 24,
        FONT_SIZE_32  = 32,
        MAX_FONT_SIZE = FONT_SIZE_32
    };

    typedef caf::AppEnum<FontSize> FontSizeType;

    static cvf::ref<caf::FixedAtlasFont> getFont( FontSize fontSize );
    static FontSize                      legacyEnumToPointSize( int enumValue );
    static FontSize                      fontSizeEnumFromPointSize( int pointSize );
    static int                           pointSizeToPixelSize( int pointSize );
    static int                           pixelSizeToPointSize( int pixelSize );

    static void clear();

private:
    static std::map<FontSize, cvf::ref<caf::FixedAtlasFont>> ms_fonts;
};
