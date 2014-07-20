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

#ifndef _LFGMGR_H
#define _LFGMGR_H

#include "Field.h"
#include "LFGPlayerData.h"

namespace lfg
{

typedef std::map<uint64, LFGPlayerData> LFGPlayerDataContainer;

class LFGMgr
{
    private:
        LFGMgr();
        ~LFGMgr();

    public:
        static LFGMgr* instance()
        {
            static LFGMgr* instance = new LFGMgr();
            return instance;
        }

        void AttemptJoin(Player* _player);
        void AttemptAddMore(Player* _player);

        std::string const& GetComment(uint64 gguid);
        bool IsQueued(Player* player);
        bool IsAutoJoin(Player* player);
        bool IsAutoAdd(Player* player);
        void SetComment(uint64 guid, std::string const& comment);
        void SetAutoJoin(Player* player, bool value);
        void SetAutoAdd(Player* player, bool value);

        void JoinLFGChannel(Player* player);
        void LeaveLFGChannel(Player* player);

        LookingForGroupSlot GetLfmSlot(uint64 guid);
        LookingForGroupSlot GetLfgSlot(uint64 guid, uint8 slot);
        void SetLFMSlot(uint64 guid, uint32 entry, uint32 type);
        void SetLFGSlot(uint64 guid, uint8 slot, uint32 entry, uint32 type);
        bool IsInLFMSlot(uint64 guid, uint32 entry, uint32 type);
        bool IsInLFGSlot(uint64 guid, uint32 entry, uint32 type);
        void ClearLFM(uint64 guid);
        void ClearLFG(uint64 guid);

        void LeaveLFG(uint64 guid);

    private:
        uint32 m_options;                                  ///< Stores config options
        LFGPlayerDataContainer PlayersStore;               ///< Player data
};

} // namespace lfg

#define sLFGMgr lfg::LFGMgr::instance()
#endif
