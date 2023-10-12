﻿#include "StdAfx.h"
#include "AdvancedXrayGameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"
#include "CustomBackpack.h"

bool	m_bKnifeSlotEnabled = false;
bool	m_bBinocularSlotEnabled = false;
bool	m_bTorchSlotEnabled = false;
bool	m_bBackpackSlotEnabled = false;
bool	m_bSecondHelmetSlotEnabled = false;
bool	m_bDosimeterSlotEnabled = false;
bool	m_bPantsSlotEnabled = false;
bool	m_bPdaSlotEnabled = false;
bool	m_bPistolSlotEnabled = false;
bool	m_bTorchUseBattery = false;
bool	m_bArtefactDetectorUseBattery = false;
bool	m_bAnomalyDetectorUseBattery = false;
bool	m_bLimitedBolts = false;
bool	m_bActorThirst = false;
bool	m_bActorIntoxication = false;
bool	m_bActorSleepeness = false;
bool	m_bActorAlcoholism = false;
bool	m_bActorNarcotism = false;
bool	m_bArtefactsDegradation = false;
bool	m_bMultiItemPickup = true;
bool	m_bShowWpnInfo = true;
bool	m_bJumpSpeedWeightCalc = false;
bool	m_bHideWeaponInInventory = false;
bool	m_bStopActorIfShoot = false;
bool	m_bReloadIfSprint = true;
bool	m_bColorizeValues = false;
bool	m_bArtefactsRanks = false;
bool	m_bUseFilters = false;
bool	m_bHideHudOnMaster = false;
bool	m_bActorSkills = false;
bool	m_bSleepInfluenceOnPsyHealth = false;
bool	m_bUseHQ_Icons = false;
bool	m_bAfPanelEnabled = false;
bool	m_bHUD_UsedItemText = true;
bool	m_bLimitedInventory = false;
bool	m_bBackpackAnimsEnabled = false;
bool	m_bFoodIrradiation = false;
bool	m_bFoodRotting = false;
int		m_iArtefactsCount = 5;
int		m_i_CMD_Count = 1;
int		m_B_CMD_Count = 1;
float	m_fDistantSndDistance = 150.f;
Fvector4 m_FV4RedColor = Fvector4().set(255, 0, 0, 255);
Fvector4 m_FV4GreenColor = Fvector4().set(0, 255, 0, 255);
Fvector4 m_FV4NeutralColor = Fvector4().set(170, 170, 170, 255);
LPCSTR	m_sAfInfluenceMode = "from_belt";
LPCSTR	m_sArtefactsDegradationMode = "from_belt";
LPCSTR	m_sMoonPhasesMode = "off";
//SSFX DoF
Fvector4 m_FV4DefaultDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);
Fvector4 m_FV4FocusDoF = Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f);
bool	m_bEnableBoreDoF = true;

namespace GameConstants
{
	void LoadConstants()
	{
		m_bTorchUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "torch_use_battery", false);
		m_bArtefactDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "artefact_detector_use_battery", false);
		m_bAnomalyDetectorUseBattery = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "anomaly_detector_use_battery", false);
		m_bKnifeSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_knife_slot", false);
		m_bBinocularSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_binocular_slot", false);
		m_bTorchSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_torch_slot", false);
		m_bBackpackSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_backpack_slot", false);
		m_bSecondHelmetSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_second_helmet_slot", false);
		m_bDosimeterSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_dosimeter_slot", false);
		m_bPantsSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pants_slot", false);
		m_bPdaSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pda_slot", false);
		m_bPistolSlotEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "inventory", "enable_pistol_slot", false);
		m_bLimitedBolts = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "limited_bolts", false);
		m_bActorThirst = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_thirst_enabled", false);
		m_bActorIntoxication = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_intoxication_enabled", false);
		m_bActorSleepeness = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_sleepeness_enabled", false);
		m_bActorAlcoholism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_alcoholism_enabled", false);
		m_bActorNarcotism = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_narcotism_enabled", false);
		m_bArtefactsDegradation = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "artefacts_degradation", false);
		m_bMultiItemPickup = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "multi_item_pickup", true);
		m_bShowWpnInfo = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "show_wpn_info", true);
		m_bJumpSpeedWeightCalc = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "jump_and_speed_weight_calc", false);
		m_bHideWeaponInInventory = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "hide_weapon_in_inventory", false);
		m_bStopActorIfShoot = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "stop_actor_sprint_if_shoot", false);
		m_bReloadIfSprint = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_reload_if_sprint", true);
		m_bArtefactsRanks = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_artefacts_ranks", false);
		m_bUseFilters = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_antigas_filters", false);
		m_bHideHudOnMaster = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "hide_hud_on_master", false);
		m_bActorSkills = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "actor_skills_enabled", false);
		m_bSleepInfluenceOnPsyHealth = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "sleepeness_infl_on_psy_health", false);
		m_bLimitedInventory = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_limited_inventory", false);
		m_bFoodIrradiation = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_food_irradiation", false);
		m_bFoodRotting = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "enable_food_rotting", false);
		m_bBackpackAnimsEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "actions_animations", "enable_backpack_animations", false);
		m_iArtefactsCount = READ_IF_EXISTS(pAdvancedSettings, r_u32, "inventory", "artefacts_count", 5);
		m_i_CMD_Count = READ_IF_EXISTS(pAdvancedSettings, r_u32, "custom_commands", "integer_cmd_count", 1);
		m_B_CMD_Count = READ_IF_EXISTS(pAdvancedSettings, r_u32, "custom_commands", "bool_cmd_count", 1);
		m_fDistantSndDistance = READ_IF_EXISTS(pAdvancedSettings, r_float, "gameplay", "distant_snd_distance", 150.f);
		m_bColorizeValues = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "colorize_values", false);
		m_FV4RedColor = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ui_settings", "colorize_values_red", Fvector4().set(255, 0, 0, 255));
		m_FV4GreenColor = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ui_settings", "colorize_values_green", Fvector4().set(0, 255, 0, 255));
		m_FV4NeutralColor = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ui_settings", "colorize_values_neutral", Fvector4().set(170, 170, 170, 255));
		m_bUseHQ_Icons = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "hq_icons", false);
		m_bAfPanelEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_artefact_panel", false);
		m_bHUD_UsedItemText = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ui_settings", "enable_hud_used_item_text", true);
		m_sAfInfluenceMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "gameplay", "artefacts_infl_mode", "from_belt"); //from_belt|from_ruck|from_ruck_only_rad
		m_sArtefactsDegradationMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "gameplay", "artefacts_degradation_mode", "from_belt"); //from_belt|from_ruck
		m_FV4DefaultDoF = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ssfx_dof", "default_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_FV4FocusDoF = READ_IF_EXISTS(pAdvancedSettings, r_fvector4, "ssfx_dof", "focus_dof", Fvector4().set(0.1f, 0.25f, 0.0f, 0.0f));
		m_bEnableBoreDoF = READ_IF_EXISTS(pAdvancedSettings, r_bool, "ssfx_dof", "bore_dof_enabled", true);
		m_sMoonPhasesMode = READ_IF_EXISTS(pAdvancedSettings, r_string, "environment", "moon_phases_mode", "off"); //off|8days|28days

		Msg("# Advanced X-Ray GameConstants are loaded");
	}

	bool GetKnifeSlotEnabled()
	{
		return m_bKnifeSlotEnabled;
	}

	bool GetBinocularSlotEnabled()
	{
		return m_bBinocularSlotEnabled;
	}

	bool GetTorchSlotEnabled()
	{
		return m_bTorchSlotEnabled;
	}

	bool GetBackpackSlotEnabled()
	{
		return m_bBackpackSlotEnabled;
	}

	bool GetSecondHelmetSlotEnabled()
	{
		return m_bSecondHelmetSlotEnabled;
	}

	bool GetDosimeterSlotEnabled()
	{
		return m_bDosimeterSlotEnabled;
	}

	bool GetPantsSlotEnabled()
	{
		return m_bPantsSlotEnabled;
	}

	bool GetPdaSlotEnabled()
	{
		return m_bPdaSlotEnabled;
	}

	bool GetPistolSlotEnabled()
	{
		return m_bPistolSlotEnabled;
	}

	bool GetTorchHasBattery()
	{
		return m_bTorchUseBattery;
	}

	bool GetArtDetectorUseBattery()
	{
		return m_bArtefactDetectorUseBattery;
	}

	bool GetAnoDetectorUseBattery()
	{
		return m_bAnomalyDetectorUseBattery;
	}

	bool GetLimitedBolts()
	{
		return m_bLimitedBolts;
	}

	bool GetActorThirst()
	{
		return m_bActorThirst;
	}

	bool GetActorIntoxication()
	{
		return m_bActorIntoxication;
	}

	bool GetActorSleepeness()
	{
		return m_bActorSleepeness;
	}

	bool GetActorAlcoholism()
	{
		return m_bActorAlcoholism;
	}

	bool GetActorNarcotism()
	{
		return m_bActorNarcotism;
	}

	bool GetArtefactsDegradation()
	{
		return m_bArtefactsDegradation;
	}

	bool GetMultiItemPickup()
	{
		return m_bMultiItemPickup;
	}

	bool GetShowWpnInfo()
	{
		return m_bShowWpnInfo;
	}

	bool GetJumpSpeedWeightCalc()
	{
		return m_bJumpSpeedWeightCalc;
	}

	bool GetHideWeaponInInventory()
	{
		CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(Actor()->inventory().ItemFromSlot(BACKPACK_SLOT));

		return (backpack) ? ((Actor()->inventory().ActiveItem() != backpack) && m_bHideWeaponInInventory) : m_bHideWeaponInInventory;
	}

	bool GetStopActorIfShoot()
	{
		return m_bStopActorIfShoot;
	}

	bool GetReloadIfSprint()
	{
		return m_bReloadIfSprint;
	}

	bool GetColorizeValues()
	{
		return m_bColorizeValues;
	}

	bool GetAfRanks()
	{
		return m_bArtefactsRanks;
	}

	bool GetOutfitUseFilters()
	{
		return m_bUseFilters;
	}

	bool GetHideHudOnMaster()
	{
		return m_bHideHudOnMaster && g_SingleGameDifficulty == egdMaster;
	}

	bool GetActorSkillsEnabled()
	{
		return m_bActorSkills;
	}

	bool GetUseHQ_Icons()
	{
		return m_bUseHQ_Icons;
	}

	bool GetSleepInfluenceOnPsyHealth()
	{
		return m_bSleepInfluenceOnPsyHealth;
	}

	bool GetArtefactPanelEnabled()
	{
		return m_bAfPanelEnabled;
	}

	bool GetHUD_UsedItemTextEnabled()
	{
		return m_bHUD_UsedItemText;
	}

	bool GetLimitedInventory()
	{
		return m_bLimitedInventory;
	}

	bool GetBackpackAnimsEnabled()
	{
		return m_bBackpackAnimsEnabled;
	}

	bool GetFoodIrradiation()
	{
		return m_bFoodIrradiation;
	}

	bool GetFoodRotting()
	{
		return m_bFoodRotting;
	}

	int GetArtefactsCount()
	{
		return m_iArtefactsCount;
	}

	int GetIntScriptCMDCount()
	{
		return m_i_CMD_Count;
	}

	int GetBOOLScriptCMDCount()
	{
		return m_B_CMD_Count;
	}

	float GetDistantSndDistance()
	{
		return m_fDistantSndDistance;
	}

	Fvector4 GetRedColor()
	{
		return m_FV4RedColor;
	}

	Fvector4 GetGreenColor()
	{
		return m_FV4GreenColor;
	}

	Fvector4 GetNeutralColor()
	{
		return m_FV4NeutralColor;
	}

	Fvector4 GetSSFX_DefaultDoF()
	{
		return m_FV4DefaultDoF;
	}

	Fvector4 GetSSFX_FocusDoF()
	{
		return m_FV4FocusDoF;
	}

	bool GetSSFX_EnableBoreDoF()
	{
		return m_bEnableBoreDoF;
	}

	LPCSTR GetAfInfluenceMode()
	{
		return m_sAfInfluenceMode;
	}

	LPCSTR GetArtefactDegradationMode()
	{
		return m_sArtefactsDegradationMode;
	}

	LPCSTR GetMoonPhasesMode()
	{
		return m_sMoonPhasesMode;
	}
}
