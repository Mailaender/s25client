// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef GLALLOCATOR_H_INCLUDED
#define GLALLOCATOR_H_INCLUDED

#pragma once

#include "libsiedler2/src/StandardAllocator.h"

class GlAllocator : public libsiedler2::StandardAllocator
{
public:
    libsiedler2::ArchivItem* create(libsiedler2::BobType type, libsiedler2::SoundType subtype = libsiedler2::SOUNDTYPE_NONE) const override;
    libsiedler2::ArchivItem* clone(const libsiedler2::ArchivItem& item) const override;
};

#endif // !GLALLOCATOR_H_INCLUDED
