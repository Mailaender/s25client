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

#include "defines.h" // IWYU pragma: keep
#include "LuaInterfaceSettings.h"
#include "GameServerInterface.h"
#include "LuaServerPlayer.h"
#include "GlobalGameSettings.h"
#include "lua/LuaHelpers.h"
#include "gameTypes/GameSettingTypes.h"
#include "addons/Addon.h"
#include "addons/const_addons.h"
#include "libutil/src/Log.h"

LuaInterfaceSettings::LuaInterfaceSettings(GameServerInterface& gameServer): gameServer_(gameServer)
{
    Register(lua);
    LuaServerPlayer::Register(lua);
    lua["rttr"] = this;
}

LuaInterfaceSettings::~LuaInterfaceSettings()
{}

void LuaInterfaceSettings::Register(kaguya::State& state)
{
    state["RTTRSettings"].setClass(kaguya::UserdataMetatable<LuaInterfaceSettings, LuaInterfaceBase>()
        .addFunction("GetPlayerCount", &LuaInterfaceSettings::GetPlayerCount)
        .addFunction("GetPlayer", &LuaInterfaceSettings::GetPlayer)
        .addOverloadedFunctions("SetAddon", &LuaInterfaceSettings::SetAddon, &LuaInterfaceSettings::SetBoolAddon)
        .addFunction("ResetAddons", &LuaInterfaceSettings::ResetAddons)
        .addFunction("ResetGameSettings", &LuaInterfaceSettings::ResetGameSettings)
        .addFunction("SetGameSettings", &LuaInterfaceSettings::SetGameSettings)
    );

    state["AddonId"].setClass(kaguya::UserdataMetatable<AddonId>());

    for(unsigned i = 0; i < AddonId::count_; ++i)
    {
        AddonId id = AddonId::type_(AddonId::values_()[i]);
        state[std::string("ADDON_") + id.toString()] = id;
    }

#pragma region ConstDefs
#define ADD_LUA_CONST(name) state[#name] = name

    ADD_LUA_CONST(GS_VERYSLOW);
    ADD_LUA_CONST(GS_SLOW);
    ADD_LUA_CONST(GS_NORMAL);
    ADD_LUA_CONST(GS_FAST);
    ADD_LUA_CONST(GS_VERYFAST);

    ADD_LUA_CONST(GO_NONE);
    ADD_LUA_CONST(GO_CONQUER3_4);
    ADD_LUA_CONST(GO_TOTALDOMINATION);

    ADD_LUA_CONST(SWR_VLOW);
    ADD_LUA_CONST(SWR_LOW);
    ADD_LUA_CONST(SWR_NORMAL);
    ADD_LUA_CONST(SWR_ALOT);

    ADD_LUA_CONST(EXP_DISABLED);
    ADD_LUA_CONST(EXP_CLASSIC);
    ADD_LUA_CONST(EXP_FOGOFWAR);
    ADD_LUA_CONST(EXP_FOGOFWARE_EXPLORED);

#undef ADD_LUA_CONST
#pragma endregion ConstDefs
}

unsigned LuaInterfaceSettings::GetPlayerCount()
{
    RTTR_Assert(gameServer_.IsRunning());
    return gameServer_.GetMaxPlayerCount();
}

LuaServerPlayer LuaInterfaceSettings::GetPlayer(unsigned idx)
{
    RTTR_Assert(gameServer_.IsRunning());
    lua::assertTrue(idx < GetPlayerCount(), "Invalid player idx");
    return LuaServerPlayer(gameServer_, idx);
}

void LuaInterfaceSettings::SetAddon(AddonId id, unsigned value)
{
    RTTR_Assert(gameServer_.IsRunning());
    GlobalGameSettings ggs = gameServer_.GetGGS();
    ggs.setSelection(id, value);
    gameServer_.ChangeGlobalGameSettings(ggs);
}

void LuaInterfaceSettings::SetBoolAddon(AddonId id, bool value)
{
    RTTR_Assert(gameServer_.IsRunning());
    SetAddon(id, value ? 1 : 0);
}

void LuaInterfaceSettings::ResetAddons()
{
    RTTR_Assert(gameServer_.IsRunning());
    GlobalGameSettings ggs = gameServer_.GetGGS();
    for(unsigned i = 0; i < ggs.getNumAddons(); ++i)
    {
        const Addon* addon = ggs.getAddon(i);
        ggs.setSelection(addon->getId(), addon->getDefaultStatus());
    }
    gameServer_.ChangeGlobalGameSettings(ggs);
}

void LuaInterfaceSettings::ResetGameSettings()
{
    RTTR_Assert(gameServer_.IsRunning());
    // Simply create a new instance
    gameServer_.ChangeGlobalGameSettings(GlobalGameSettings());
}

void LuaInterfaceSettings::SetGameSettings(const kaguya::LuaTable& settings)
{
    RTTR_Assert(gameServer_.IsRunning());
    GlobalGameSettings ggs = gameServer_.GetGGS();
    std::vector<std::string>keys = settings.keys<std::string>();

    if(std::find(keys.begin(), keys.end(), "speed") != keys.end())
    {
        GameSpeed speed = settings.getField("speed");
        lua::assertTrue(unsigned(speed) <= GS_VERYFAST, "Speed is invalid");
        ggs.speed = speed;
    }

    if(std::find(keys.begin(), keys.end(), "objective") != keys.end())
    {
        GameObjective objective = settings.getField("objective");
        lua::assertTrue(unsigned(objective) <= GO_TOTALDOMINATION, "Objective is invalid");
        ggs.objective = objective;
    }

    if(std::find(keys.begin(), keys.end(), "startWares") != keys.end())
    {
        StartWares wares = settings.getField("startWares");
        lua::assertTrue(unsigned(wares) <= SWR_ALOT, "Start wares is invalid");
        ggs.startWares = wares;
    }

    if(std::find(keys.begin(), keys.end(), "fow") != keys.end())
    {
        Exploration fow = settings.getField("fow");
        lua::assertTrue(unsigned(fow) <= EXP_FOGOFWARE_EXPLORED, "FoW is invalid");
        ggs.exploration = fow;
    }

    if(std::find(keys.begin(), keys.end(), "lockedTeams") != keys.end())
        ggs.lockedTeams = settings.getField("lockedTeams");

    if(std::find(keys.begin(), keys.end(), "teamView") != keys.end())
        ggs.teamView = settings.getField("teamView");

    if(std::find(keys.begin(), keys.end(), "randomStartPosition") != keys.end())
        ggs.randomStartPosition = settings.getField("randomStartPosition");

    gameServer_.ChangeGlobalGameSettings(ggs);
}

kaguya::LuaRef LuaInterfaceSettings::GetAllowedChanges()
{
    kaguya::LuaRef getGeneralConfig = lua["getAllowedChanges"];
    if(getGeneralConfig.type() == LUA_TFUNCTION)
    {
        kaguya::LuaRef cfg = getGeneralConfig.call<kaguya::LuaRef>();
        if(cfg.type() == LUA_TTABLE)
            return cfg;
    }
    return kaguya::LuaRef();
}

bool LuaInterfaceSettings::EventSettingsInit(bool isSinglePlayer, bool isSavegame)
{
    kaguya::LuaRef func = lua["onSettingsInit"];
    if(func.type() == LUA_TFUNCTION)
        return func.call<bool>(isSinglePlayer, isSavegame);
    else
        return true;
}

void LuaInterfaceSettings::EventSettingsReady()
{
    RTTR_Assert(gameServer_.IsRunning());
    kaguya::LuaRef func = lua["onSettingsReady"];
    if(func.type() == LUA_TFUNCTION)
        func.call<void>();
}

void LuaInterfaceSettings::EventPlayerJoined(unsigned playerIdx)
{
    RTTR_Assert(gameServer_.IsRunning());
    kaguya::LuaRef func = lua["onPlayerJoined"];
    if(func.type() == LUA_TFUNCTION)
        func.call<void>(playerIdx);
}

void LuaInterfaceSettings::EventPlayerLeft(unsigned playerIdx)
{
    RTTR_Assert(gameServer_.IsRunning());
    kaguya::LuaRef func = lua["onPlayerLeft"];
    if(func.type() == LUA_TFUNCTION)
        func.call<void>(playerIdx);
}

void LuaInterfaceSettings::EventPlayerReady(unsigned playerIdx)
{
    RTTR_Assert(gameServer_.IsRunning());
    kaguya::LuaRef func = lua["onPlayerReady"];
    if(func.type() == LUA_TFUNCTION)
        func.call<void>(playerIdx);
}

bool LuaInterfaceSettings::IsChangeAllowed(const std::string& name, const bool defaultVal/* = false*/)
{
    kaguya::LuaRef cfg = GetAllowedChanges();
    if(cfg.isNilref())
        return defaultVal;
    cfg = cfg.getField(name);
    if(cfg.typeTest<bool>())
        return cfg;
    else
        return defaultVal;
}

std::vector<AddonId> LuaInterfaceSettings::GetAllowedAddons()
{
    kaguya::LuaRef getAllowedAddons = lua["getAllowedAddons"];
    if(getAllowedAddons.type() == LUA_TFUNCTION)
    {
        kaguya::LuaRef addons = getAllowedAddons();
        if(addons.typeTest<std::vector<AddonId> >())
            return addons;
        else
            LOG.write("Invalid type returned by getAllowedAddons");
    }
    return std::vector<AddonId>();
}
