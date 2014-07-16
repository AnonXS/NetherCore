/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "LFGPlayerData.h"

namespace lfg
{

    LfgPlayerData::LfgPlayerData(): m_Comment("")
    { }

    LfgPlayerData::~LfgPlayerData() { }

    std::string const& LfgPlayerData::GetComment() const
    {
        return m_Comment;
    }

    void LfgPlayerData::SetComment(std::string const& comment)
    {
        m_Comment = comment;
    }

    // LFM
    LookingForGroupSlot const& LfgPlayerData::GetLfmSlot() const
    {
        return m_Lfm;
    }

    void LfgPlayerData::SetLfmSlot(uint32 entry, uint32 type)
    {
        m_Lfm.Set(entry,type);
    }

    // LFG
    LookingForGroupSlot const& LfgPlayerData::GetLfgSlot(uint8 slot) const
    {
        return m_Lfg[slot];
    }

    void LfgPlayerData::SetLfgSlot(uint8 slot, uint32 entry, uint32 type)
    {
        m_Lfg[slot].Set(entry,type);
    }

} // namespace lfg
