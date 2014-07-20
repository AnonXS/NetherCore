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

#include "Common.h"
#include "SharedDefines.h"
#include "LFGMgr.h"
#include "LFGScripts.h"
#include "LFGPlayerData.h"
#include "Group.h"
#include "Player.h"
#include "GroupMgr.h"
#include "WorldSession.h"
#include "ChannelMgr.h"

namespace lfg
{

LFGMgr::LFGMgr()
{
    new LFGPlayerScript();
}

LFGMgr::~LFGMgr() { }

void LFGMgr::AttemptJoin(Player* _player)
{
    // skip if player dosnt have auto join enabled or is in a group
    if (!sLFGMgr->IsAutoJoin(_player) || _player->GetGroup())
        return;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    HashMapHolder<Player>::MapType const& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        Player* plr = itr->second;
        LookingForGroupSlot lfmSlot = sLFGMgr->GetLfmSlot(plr->GetGUID());

        // skip enemies and self
        if (!plr || plr == _player || plr->GetTeam() != _player->GetTeam())
            continue;

        // skip if the player is not in world
        if (!plr->IsInWorld())
            continue;
            
        // skip if auto add is not enabled or the player is not the group leader
        if (!sLFGMgr->IsAutoAdd(plr) || (plr->GetGroup() && plr->GetGroup()->GetLeaderGUID() != plr->GetGUID()))
            continue;

        // skip if type dosnt allow auto join or the slots dosnt fit
        if (!lfmSlot.CanAutoJoin() || !sLFGMgr->IsInLFGSlot(_player->GetGUID(), lfmSlot.entry, lfmSlot.type))
            continue;

        // attempt create group, or skip
        if (!plr->GetGroup())
        {
            Group* group = new Group;
            if (!group->Create(plr))
            {
                delete group;
                continue;
            }
            sGroupMgr->AddGroup(group);
        }
        
        plr->GetGroup()->AddMember(_player);
    }
}

void LFGMgr::AttemptAddMore(Player* _player)
{
    LookingForGroupSlot lfmSlot = sLFGMgr->GetLfmSlot(_player->GetGUID());
        
    // skip if the player is not the group leader
    if (_player->GetGroup() && _player->GetGroup()->GetLeaderGUID() != _player->GetGUID())
        return;

    // skip if type dosnt allow auto join
    if (!lfmSlot.CanAutoJoin())
        return;

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    HashMapHolder<Player>::MapType const& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        Player* plr = itr->second;

        // skip enemies and self
        if (!plr || plr == _player || plr->GetTeam() != _player->GetTeam())
            continue;

        // skip if the player is not in world
        if (!plr->IsInWorld())
            continue;

        // skip if player dosnt have auto join enabled or is in a group
        if (!sLFGMgr->IsAutoJoin(plr) || plr->GetGroup())
            continue;
            
        // skip if slots dosnt fit
        if (!sLFGMgr->IsInLFGSlot(plr->GetGUID(), lfmSlot.entry, lfmSlot.type))
            continue;

        // attempt create group if need, or stop attempts
        if (!_player->GetGroup())
        {
            Group* group = new Group;
            if (!group->Create(_player))
            {
                delete group;
                return;
            }
            sGroupMgr->AddGroup(group);
        }

        _player->GetGroup()->AddMember(plr);
    }
}

const std::string& LFGMgr::GetComment(uint64 guid)
{
    TC_LOG_TRACE("lfg.data.player.comment.get", "Player: %u, Comment: %s", GUID_LOPART(guid), PlayersStore[guid].GetComment().c_str());
    return PlayersStore[guid].GetComment();
}

bool LFGMgr::IsQueued(Player* player)
{
    if (!player->GetGroup()) {
        for (int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        {
            if (PlayersStore[player->GetGUID()].GetLFGSlot(i).Used())
                return true;
        }
    }

    if (PlayersStore[player->GetGUID()].GetLFMSlot().Used())
        return true;

    return false;
}

bool LFGMgr::IsAutoJoin(Player* player)
{
    TC_LOG_TRACE("lfg.data.player.autojoin.get", "Player: %u, Value: %s", player->GetSession()->GetGuidLow(), player->GetSession()->LookingForGroup_auto_join);
    return player->GetSession()->LookingForGroup_auto_join;
}

bool LFGMgr::IsAutoAdd(Player* player)
{
    TC_LOG_TRACE("lfg.data.player.autoadd.get", "Player: %u, Value: %s", player->GetSession()->GetGuidLow(), player->GetSession()->LookingForGroup_auto_add);
    return player->GetSession()->LookingForGroup_auto_add;
}

void LFGMgr::SetComment(uint64 guid, std::string const& comment)
{
    TC_LOG_TRACE("lfg.data.player.comment.set", "Player: %u, Comment: %s", GUID_LOPART(guid), comment.c_str());
    PlayersStore[guid].SetComment(comment);
}

void LFGMgr::SetAutoJoin(Player* player, bool value)
{
    TC_LOG_TRACE("lfg.data.player.autojoin.set", "Player: %u, Value: %s", player->GetSession()->GetGuidLow(), value);
    player->GetSession()->LookingForGroup_auto_join = value;
}

void LFGMgr::SetAutoAdd(Player* player, bool value)
{
    TC_LOG_TRACE("lfg.data.player.autoadd.set", "Player: %u, Value: %s", player->GetSession()->GetGuidLow(), value);
    player->GetSession()->LookingForGroup_auto_add = value;
}

void LFGMgr::JoinLFGChannel(Player* player)
{
    ChannelMgr* cMgr = ChannelMgr::forTeam(player->GetTeam());

    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        if (ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i))
        {
            if (channel->flags & CHANNEL_DBC_FLAG_LFG && player->CanJoinConstantChannelInZone(channel, GetAreaEntryByAreaID(player->GetZoneId())))
            {
                Channel* ch = cMgr->GetJoinChannel(channel->pattern[player->GetSession()->GetSessionDbcLocale()], channel->ChannelID);
                ch->JoinChannel(player, "");
            }
        }
    }
}

void LFGMgr::LeaveLFGChannel(Player* player)
{
    ChannelMgr* cMgr = ChannelMgr::forTeam(player->GetTeam());

    if (Channel* ch = cMgr->GetChannel("LookingForGroup", player, false))
        if (ch->IsOn(player->GetGUID()))
            ch->LeaveChannel(player);
}

LookingForGroupSlot LFGMgr::GetLfmSlot(uint64 guid)
{
    TC_LOG_TRACE("lfg.data.player.lfmslot.get", "Player: %u", GUID_LOPART(guid));
    return PlayersStore[guid].GetLFMSlot();
}

LookingForGroupSlot LFGMgr::GetLfgSlot(uint64 guid, uint8 slot)
{
    TC_LOG_TRACE("lfg.data.player.lfgslot.get", "Player: %u", GUID_LOPART(guid));
    return PlayersStore[guid].GetLFGSlot(slot);
}

void LFGMgr::SetLFMSlot(uint64 guid, uint32 entry, uint32 type)
{
    TC_LOG_TRACE("lfg.data.player.lfmslot.set", "Player: %u, Entry: %s, Type: %s", GUID_LOPART(guid), entry, type);
    PlayersStore[guid].SetLFMSlot(entry, type);
}

void LFGMgr::SetLFGSlot(uint64 guid, uint8 slot, uint32 entry, uint32 type)
{
    TC_LOG_TRACE("lfg.data.player.lfgslot.set", "Player: %u, Slot: %s, Entry: %s, Type: %s", GUID_LOPART(guid), slot, entry, type);
    PlayersStore[guid].SetLFGSlot(slot, entry, type);
}

bool LFGMgr::IsInLFMSlot(uint64 guid, uint32 entry, uint32 type)
{
    return PlayersStore[guid].GetLFMSlot().Is(entry, type);
}

bool LFGMgr::IsInLFGSlot(uint64 guid, uint32 entry, uint32 type)
{
    for (int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
    if (PlayersStore[guid].GetLFGSlot(i).Is(entry, type))
        return true;
    return false;
}

void LFGMgr::ClearLFM(uint64 guid)
{
    PlayersStore[guid].SetLFMSlot(0, 0);
}

void LFGMgr::ClearLFG(uint64 guid)
{
    for (int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        PlayersStore[guid].SetLFGSlot(i, 0, 0);
}

void LFGMgr::LeaveLFG(uint64 guid)
{
    TC_LOG_DEBUG("lfg.leave", "%u left (player)", GUID_LOPART(guid));
    PlayersStore.erase(guid);
}

} // namespace lfg
