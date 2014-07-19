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

#include "LFGMgr.h"
#include "Group.h"
#include "Player.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"

void WorldSession::HandleLookingForGroup(WorldPacket& recv_data)
{
    uint32 type, entry, unk;
    recv_data >> type >> entry >> unk;

    TC_LOG_DEBUG("lfg", "MSG_LOOKING_FOR_GROUP %s type: %u, entry: %u, unk %u", GetPlayerInfo().c_str(), type, entry, unk);

    if (sLFGMgr->IsAutoAdd(_player))
        sLFGMgr->AttemptAddMore(_player);

    if (sLFGMgr->IsAutoJoin(_player))
        sLFGMgr->AttemptJoin(_player);

    SendLfgResult(LfgType(type), entry, LFG_MODE);
}

void WorldSession::HandleLfgSetOpcode(WorldPacket& recv_data)
{
    uint32 slot, temp, entry, type;
    recv_data >> slot >> temp;

    entry = (temp & 0xFFFF);
    type = ((temp >> 24) & 0xFFFF);

    if (slot >= MAX_LOOKING_FOR_GROUP_SLOT)
        return;

    sLFGMgr->SetLfgSlot(_player->GetGUID(), slot, entry, type);
    TC_LOG_DEBUG("lfg", "CMSG_SET_LOOKING_FOR_GROUP %s looknumber: %u, temp: %X, type: %u, entry: %u", GetPlayerInfo().c_str(), slot, temp, type, entry);

    if (sLFGMgr->IsAutoJoin(_player))
        sLFGMgr->AttemptJoin(_player);

    SendLfgResult(LfgType(type), entry, LFG_MODE);
}

void WorldSession::HandleLfgSetCommentOpcode(WorldPacket& recv_data)
{
    std::string comment;
    recv_data >> comment;

    TC_LOG_DEBUG("lfg", "CMSG_LFG_SET_COMMENT %s comment: %s", GetPlayerInfo().c_str(), comment.c_str());

    sLFGMgr->SetComment(_player->GetGUID(), comment);
}

void WorldSession::HandleLfgSetAutoJoinOpcode(WorldPacket & /*recv_data*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_SET_LFG_COMMENT %s", GetPlayerInfo().c_str());
    LookingForGroup_auto_join = true;

    // needed because STATUS_AUTHED
    if (!_player)
        return;

    sLFGMgr->AttemptJoin(_player);
}

void WorldSession::HandleLfgClearAutoJoinOpcode(WorldPacket & /*recv_data*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFG_CLEAR_AUTOJOIN %s", GetPlayerInfo().c_str());
    sLFGMgr->SetAutoJoin(_player, false);
}

void WorldSession::HandleLfgClearOpcode(WorldPacket & /*recv_data */)
{
    TC_LOG_DEBUG("lfg", "CMSG_CLEAR_LOOKING_FOR_GROUP %s", GetPlayerInfo().c_str());

    sLFGMgr->ClearLfg(_player->GetGUID());

    //if (sWorld->getBoolConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && _player->GetSession()->GetSecurity() == SEC_PLAYER)
    //    _player->LeaveLFGChannel();
}

void WorldSession::HandleLfmSetOpcode(WorldPacket& recv_data)
{
    uint32 temp;
    recv_data >> temp;

    uint32 entry = (temp & 0xFFFF);
    uint32 type = ((temp >> 24) & 0xFFFF);

    sLFGMgr->SetLfmSlot(_player->GetGUID(), entry, type);
    TC_LOG_DEBUG("lfg", "CMSG_SET_LOOKING_FOR_MORE %s temp: %u, zone: %u, type: %u", GetPlayerInfo().c_str(), temp, entry, type);

    if (sLFGMgr->IsAutoAdd(_player))
        sLFGMgr->AttemptAddMore(_player);

    SendLfgResult(LfgType(type), entry, LFM_MODE);
}

void WorldSession::HandleLfmSetAutoFillOpcode(WorldPacket & /*recv_data*/)
{
    TC_LOG_DEBUG("lfg", "CMSG_LFM_SET_AUTOFILL %s", GetPlayerInfo().c_str());
    LookingForGroup_auto_add = true;

    // needed because STATUS_AUTHED
    if (!_player)
        return;
    
    sLFGMgr->AttemptAddMore(_player);
}

void WorldSession::HandleLfmClearAutoFillOpcode(WorldPacket & /*recv_data*/)
{
    TC_LOG_DEBUG("dungeon", "CMSG_LFM_CLEAR_AUTOFILL %s", GetPlayerInfo().c_str());
    sLFGMgr->SetAutoAdd(_player, false);
}

void WorldSession::HandleLfmClearOpcode(WorldPacket & /*recv_data */)
{
    TC_LOG_DEBUG("lfg", "CMSG_CLEAR_LOOKING_FOR_MORE %s", GetPlayerInfo().c_str());
    sLFGMgr->ClearLfm(_player->GetGUID());
}

void WorldSession::SendLfgResult(LfgType type, uint32 entry, LfgMode lfg_mode)
{
    uint32 number = 0;

    WorldPacket data(MSG_LOOKING_FOR_GROUP);
    data << uint32(type);                                   // type
    data << uint32(entry);                                  // entry from LFGDungeons.dbc
    data << uint32(0);                                      // count, placeholder
    data << uint32(0);                                      // count again, strange, placeholder

    boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

    HashMapHolder<Player>::MapType const& m = sObjectAccessor->GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        Player* plr = itr->second;

        if (!plr || plr->GetTeam() != _player->GetTeam())
            continue;

        if (!plr->IsInWorld())
            continue;
        
        if ((plr->GetGroup() || !sLFGMgr->IsInLfgSlot(plr->GetGUID(), entry, type)) && (!plr->GetGroup() || !sLFGMgr->IsInLfmSlot(plr->GetGUID(), entry, type)))
            continue;

        ++number;

        data.append(plr->GetPackGUID());                    // packed guid
        data << uint32(plr->getLevel());                    // level
        data << uint32(plr->GetZoneId());                   // current zone

        if (sLFGMgr->IsInLfgSlot(plr->GetGUID(), entry, type))
        {
            data << uint8(LFG_MODE);                        // 0x00 - LFG, 0x01 - LFM
            for (uint8 j = 0; j < MAX_LOOKING_FOR_GROUP_SLOT; ++j)
                data << uint32(sLFGMgr->GetLfgSlot(plr->GetGUID(), j).entry | (sLFGMgr->GetLfgSlot(plr->GetGUID(), j).type << 24));
        }
        else
        {
            data << uint8(LFM_MODE);                        // 0x00 - LFG, 0x01 - LFM
            data << uint32(sLFGMgr->GetLfmSlot(plr->GetGUID()).entry | (sLFGMgr->GetLfmSlot(plr->GetGUID()).type << 24));
            data << uint32(0);
            data << uint32(0);
        }
            

        

        data << sLFGMgr->GetComment(plr->GetGUID());

        Group* group = plr->GetGroup();
        if (group)
        {
            data << uint32(group->GetMembersCount() - 1);   // count of group members without group leader
            for (GroupReference* itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->GetGUID() != plr->GetGUID())
                {
                    data.append(member->GetPackGUID());     // packed guid
                    data << uint32(member->getLevel());     // player level
                }
            }
        }
        else
        {
            data << uint32(0x00);
        }
    }

    // fill count placeholders
    data.put<uint32>(4 + 4, number);
    data.put<uint32>(4 + 4 + 4, number);

    SendPacket(&data);
}
