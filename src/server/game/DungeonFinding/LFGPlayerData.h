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

#ifndef _LFGPLAYERDATA_H
#define _LFGPLAYERDATA_H

namespace lfg
{

struct LookingForGroupSlot
{
    LookingForGroupSlot() : entry(0), type(0) {}
    bool Empty() const { return !entry && !type; }
    void Set(uint32 _entry, uint32 _type) { entry = _entry; type = _type; }
    bool Is(uint32 _entry, uint32 _type) const { return entry == _entry && type == _type; }
    bool CanAutoJoin() const { return entry && (type == LFG_TYPE_DUNGEON || type == LFG_TYPE_HEROIC_DUNGEON); }

    uint32 entry;
    uint32 type;
};

/**
    Stores all lfg data needed about the player.
*/
class LfgPlayerData
{
    public:
        LfgPlayerData();
        ~LfgPlayerData();

        std::string const& GetComment() const;
        void SetComment(std::string const& comment);

        LookingForGroupSlot const& GetLfmSlot() const;
        void SetLfmSlot(uint32 entry, uint32 type);
           
        LookingForGroupSlot const& GetLfgSlot(uint8 slot) const;
        void SetLfgSlot(uint8 slot, uint32 entry, uint32 type);

    private:
        LookingForGroupSlot m_Lfm;
        LookingForGroupSlot m_Lfg[MAX_LOOKING_FOR_GROUP_SLOT];

        std::string m_Comment;
        bool m_AutoJoin;            // init value send by client CMSG_LFG_SET_AUTOJOIN, before player login
        bool m_AutoAdd;             // init value send by client CMSG_LFM_CLEAR_AUTOFILL, before player login
};

} // namespace lfg

#endif
