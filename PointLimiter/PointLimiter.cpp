﻿#define _CRT_SECURE_NO_WARNINGS
#include "../../json.hpp"

#include <SQLiteCpp/Database.h>

#include <API/ARK/Ark.h>
#include <IApiUtils.h>
#include <API/UE/Math/ColorList.h>

#include <fstream>
#include <iostream>

#pragma comment(lib, "ArkApi.lib")



DECLARE_HOOK(UPrimalCharacterStatusComponent_ServerApplyLevelUp, void, UPrimalCharacterStatusComponent*,
EPrimalCharacterStatusValue::Type, AShooterPlayerController*);

nlohmann::json config;

float getCap(const std::string& key, bool bIsPlayer)
{
	const std::string select = bIsPlayer ? "PlayerCap" : "DinoCap";
	const float configCap = config[select].value(key, FLT_MAX);
	return (configCap < 0) ? FLT_MAX : configCap;
}

void loadConfig()
{
	const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/PointLimiter/config.json";
	std::ifstream file(config_path);
	if (!file.is_open()) throw std::runtime_error("Can't open config.json");
	file >> config;
	file.close();
}

float getCurrentMaximumValues(UPrimalCharacterStatusComponent* charStatus, EPrimalCharacterStatusValue::Type value)
{
	const auto fieldArray = charStatus->MaxStatusValuesField()();

	switch (value)
	{
	case EPrimalCharacterStatusValue::Health: return fieldArray[0];
	case EPrimalCharacterStatusValue::Stamina: return fieldArray[1];
	case EPrimalCharacterStatusValue::Oxygen: return fieldArray[3];
	case EPrimalCharacterStatusValue::Food: return fieldArray[4];
	case EPrimalCharacterStatusValue::Water: return fieldArray[5];
	case EPrimalCharacterStatusValue::Weight: return fieldArray[7];
	case EPrimalCharacterStatusValue::MeleeDamageMultiplier: return (1 + fieldArray[8]) * 100;
	case EPrimalCharacterStatusValue::SpeedMultiplier: return (1 + fieldArray[9]) * 100;
	case EPrimalCharacterStatusValue::TemperatureFortitude: return fieldArray[10];
	case EPrimalCharacterStatusValue::CraftingSpeedMultiplier: return (1 + fieldArray[11]) * 100;
	default: return 0;
	}
}

void Hook_UPrimalCharacterStatusComponent_ServerApplyLevelUp(UPrimalCharacterStatusComponent* charStatus,
	EPrimalCharacterStatusValue::Type levelUpValueType,
	AShooterPlayerController* byPC)
{
	const float current = getCurrentMaximumValues(charStatus, levelUpValueType);
	bool allowed = false;

	if (byPC->GetPlayerCharacter()->MyCharacterStatusComponentField() == charStatus)
	{
		// It's a player
		switch (levelUpValueType)
		{
		case EPrimalCharacterStatusValue::Health:
			if (getCap("Health", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Stamina:
			if (getCap("Stamina", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Oxygen:
			if (getCap("Oxygen", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Food:
			if (getCap("Food", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Water:
			if (getCap("Water", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Weight:
			if (getCap("Weight", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::MeleeDamageMultiplier:
			if (getCap("MeleeDamageMultiplier", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::SpeedMultiplier:
			if (getCap("SpeedMultiplier", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::TemperatureFortitude:
			if (getCap("TemperatureFortitude", true) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::CraftingSpeedMultiplier:
			if (getCap("CraftingSpeedMultiplier", true) > current)
			{
				allowed = true;
			}
			break;
		default:
		{
			Log::GetLog()->info("Failed to decide case for players [{}]", levelUpValueType);
			return;
		}
		}
	}
	else
	{
		// It's a dino
		switch (levelUpValueType)
		{
		case EPrimalCharacterStatusValue::Health:
			if (getCap("Health", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Stamina:
			if (getCap("Stamina", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Oxygen:
			if (getCap("Oxygen", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Food:
			if (getCap("Food", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::Weight:
			if (getCap("Weight", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::MeleeDamageMultiplier:
			if (getCap("MeleeDamageMultiplier", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::SpeedMultiplier:
			if (getCap("SpeedMultiplier", false) > current)
			{
				allowed = true;
			}
			break;
		case EPrimalCharacterStatusValue::CraftingSpeedMultiplier:
			if (getCap("CraftingSpeedMultiplier", false) > current)
			{
				allowed = true;
			}
			break;
		default:
		{
			Log::GetLog()->info("Failed to decide case for dinos [{}]", levelUpValueType);
		}
		}
	}
	if (allowed)
	{
		return UPrimalCharacterStatusComponent_ServerApplyLevelUp_original(charStatus, levelUpValueType, byPC);
	}
}

void Load()
{
	Log::Get().Init("PointLimiter");
	loadConfig();
	ArkApi::GetHooks().SetHook("UPrimalCharacterStatusComponent.ServerApplyLevelUp",
		&Hook_UPrimalCharacterStatusComponent_ServerApplyLevelUp,
		&UPrimalCharacterStatusComponent_ServerApplyLevelUp_original);
}

void Unload()
{
	ArkApi::GetHooks().DisableHook("UPrimalCharacterStatusComponent.ServerApplyLevelUp",
		&Hook_UPrimalCharacterStatusComponent_ServerApplyLevelUp);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		Unload();
		break;
	}
	return TRUE;
}
