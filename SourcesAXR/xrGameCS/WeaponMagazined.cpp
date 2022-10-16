#include "pch_script.h"
#include "hudmanager.h"
#include "WeaponMagazined.h"
#include "entity.h"
#include "actor.h"
#include "ParticlesObject.h"
#include "scope.h"
#include "silencer.h"
#include "GrenadeLauncher.h"
#include "inventory.h"
#include "xrserver_objects_alife_items.h"
#include "ActorEffector.h"
#include "EffectorZoomInertion.h"
#include "xr_level_controller.h"
#include "level.h"
#include "object_broker.h"
#include "string_table.h"
#include "MPPlayersBag.h"
#include "ui/UIXmlInit.h"
#include "ui/UIStatic.h"
#include "AdvancedXrayGameConstants.h"

ENGINE_API	bool	g_dedicated_server;
ENGINE_API  extern float psHUD_FOV;
ENGINE_API  extern float psHUD_FOV_def;

//CUIXml*				pWpnScopeXml = NULL;

CWeaponMagazined::CWeaponMagazined(ESoundTypes eSoundType) : CWeapon()
{
	m_eSoundShow				= ESoundTypes(SOUND_TYPE_ITEM_TAKING | eSoundType);
	m_eSoundHide				= ESoundTypes(SOUND_TYPE_ITEM_HIDING | eSoundType);
	m_eSoundShot				= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING | eSoundType);
	m_eSoundEmptyClick			= ESoundTypes(SOUND_TYPE_WEAPON_EMPTY_CLICKING | eSoundType);
	m_eSoundReload				= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING | eSoundType);
	m_eSoundClose				= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);

	psWpnAnimsFlag = { 0 };
	
	m_sSilencerFlameParticles	= m_sSilencerSmokeParticles = NULL;

	m_bFireSingleShot			= false;
	m_iShotNum					= 0;
	m_iQueueSize				= WEAPON_ININITE_QUEUE;
	m_bLockType					= false;
	m_bAutoreloadEnabled = READ_IF_EXISTS(pAdvancedSettings, r_bool, "gameplay", "autoreload_enabled", true);
	m_bHasDistantShotSound		= false;
	m_bNeedBulletInGun			= false;
	m_bHasDifferentFireModes	= false;
	m_opened					= false;
	m_bUseFiremodeChangeAnim	= true;
}

CWeaponMagazined::~CWeaponMagazined()
{
	// sounds
}


void CWeaponMagazined::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponMagazined::SetAnimFlag(u32 flag, LPCSTR anim_name)
{
	if (pSettings->line_exist(hud_sect, anim_name))
		psWpnAnimsFlag.set(flag, TRUE);
	else
		psWpnAnimsFlag.set(flag, FALSE);
}

void CWeaponMagazined::Load	(LPCSTR section)
{
	inherited::Load		(section);

	SetAnimFlag(ANM_SHOW_EMPTY,		"anm_show_empty");
	SetAnimFlag(ANM_HIDE_EMPTY,		"anm_hide_empty");
	SetAnimFlag(ANM_IDLE_EMPTY,		"anm_idle_empty");
	SetAnimFlag(ANM_AIM_EMPTY,		"anm_idle_aim_empty");
	SetAnimFlag(ANM_BORE_EMPTY,		"anm_bore_empty");
	SetAnimFlag(ANM_SHOT_EMPTY,		"anm_shot_l");
	SetAnimFlag(ANM_SPRINT_EMPTY,	"anm_idle_sprint_empty");
	SetAnimFlag(ANM_MOVING_EMPTY,	"anm_idle_moving_empty");
	SetAnimFlag(ANM_RELOAD_EMPTY,	"anm_reload_empty");
	SetAnimFlag(ANM_MISFIRE,		"anm_reload_misfire");
	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");
		
	// Sounds
	m_sounds.LoadSound(section,"snd_draw", "sndShow"		, m_eSoundShow		);
	m_sounds.LoadSound(section,"snd_holster", "sndHide"		, m_eSoundHide		);
	m_sounds.LoadSound(section,"snd_shoot", "sndShot"		, m_eSoundShot		);
	m_sounds.LoadSound(section,"snd_empty", "sndEmptyClick"	, m_eSoundEmptyClick	);
	m_sounds.LoadSound(section,"snd_reload", "sndReload"		, m_eSoundReload		);
	m_sounds.LoadSound(section, "snd_reflect", "sndReflect",  m_eSoundReflect);

	if (WeaponSoundExist(section, "snd_changefiremode"))
		m_sounds.LoadSound(section, "snd_changefiremode", "sndFireModes", m_eSoundEmptyClick);

	if (WeaponSoundExist(section, "snd_close"))
		m_sounds.LoadSound(section, "snd_close", "sndClose", m_eSoundClose);

	if (WeaponSoundExist(section, "snd_reload_empty"))
		m_sounds.LoadSound(section, "snd_reload_empty", "sndReloadEmpty", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_misfire"))
		m_sounds.LoadSound(section, "snd_reload_misfire", "sndReloadMisfire", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_reload_jammed"))
		m_sounds.LoadSound(section, "snd_reload_jammed", "sndReloadJammed", m_eSoundReload);
	if (WeaponSoundExist(section, "snd_pump_gun"))
		m_sounds.LoadSound(section, "snd_pump_gun", "sndPumpGun", m_eSoundReload);
	
	if (pSettings->line_exist(section, "snd_shoot_dist")) // distant sound
	{
		m_sounds.LoadSound(section, "snd_shoot_distant", "sndShotDist", m_eSoundShot);
		m_sounds.LoadSound(section, "snd_shoot_distant_far", "sndShotDistFar", m_eSoundShot);
		m_bHasDistantShotSound = true;
	}

	//����� � �������� ���������, ����� ����� ����
	if ( m_eSilencerStatus == ALife::eAddonAttachable || m_eSilencerStatus == ALife::eAddonPermanent )
	{
		if(pSettings->line_exist(section, "silencer_flame_particles"))
			m_sSilencerFlameParticles = pSettings->r_string(section, "silencer_flame_particles");
		if(pSettings->line_exist(section, "silencer_smoke_particles"))
			m_sSilencerSmokeParticles = pSettings->r_string(section, "silencer_smoke_particles");
		
		m_sounds.LoadSound(section,"snd_silncer_shot", "sndSilencerShot", m_eSoundShot);
	}

	if (pSettings->line_exist(section, "dispersion_start"))
		m_iShootEffectorStart = pSettings->r_u8(section, "dispersion_start");
	else
		m_iShootEffectorStart = 0;

	if (pSettings->line_exist(section, "fire_modes"))
	{
		m_bHasDifferentFireModes = true;
		shared_str FireModesList = pSettings->r_string(section, "fire_modes");
		int ModesCount = _GetItemCount(FireModesList.c_str());
		m_aFireModes.clear();
		
		for (int i=0; i<ModesCount; i++)
		{
			string16 sItem;
			_GetItem(FireModesList.c_str(), i, sItem);
			m_aFireModes.push_back	((s8)atoi(sItem));
		}
		
		m_iCurFireMode = ModesCount - 1;
		m_iPrefferedFireMode = READ_IF_EXISTS(pSettings, r_s16,section,"preffered_fire_mode",-1);
	}
	else
	{
		m_bHasDifferentFireModes = false;
	}
	LoadSilencerKoeffs();

	m_bUseFiremodeChangeAnim = READ_IF_EXISTS(pSettings, r_bool, section, "use_firemode_change_anim", false);
}

bool CWeaponMagazined::UseScopeTexture()
{
	return bScopeIsHasTexture;
}

void CWeaponMagazined::FireStart		()
{
	if(!IsMisfire())
	{
		if(IsValid()) 
		{
			if(!IsWorking() || AllowFireWhileWorking())
			{
				if (GetState() == eReload)
					return;
				if (GetState() == eShowing)
					return;
				if (GetState() == eHiding)
					return;
				if (GetState() == eMisfire)
					return;
				if (GetState() == eUnMisfire)
					return;
				if (GetState() == eFiremodePrev)
					return;
				if (GetState() == eFiremodeNext)
					return;

				inherited::FireStart();
				
				if (iAmmoElapsed == 0) 
					OnMagazineEmpty();
				else{
					R_ASSERT(H_Parent());
					SwitchState(eFire);
				}
			}
		}else 
		{
			if(eReload!=GetState()) 
				OnMagazineEmpty();
		}
	}else
	{//misfire
		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
			HUD().GetUI()->AddInfoMessage("gun_jammed");

		OnEmptyClick();
	}
}

void CWeaponMagazined::FireEnd() 
{
	inherited::FireEnd();

	if (m_bAutoreloadEnabled)
	{
		CActor	*actor = smart_cast<CActor*>(H_Parent());

		if (Actor()->mstate_real & (mcSprint) && !GameConstants::GetReloadIfSprint())
			return;

		if (m_pInventory && !iAmmoElapsed && actor && GetState() != eReload)
			Reload();
	}
}

void CWeaponMagazined::Reload() 
{
	inherited::Reload();
	TryReload();
}

#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"

bool CWeaponMagazined::TryReload() 
{
	if(m_pInventory) 
	{
		if(IsGameTypeSingle() && ParentIsActor())
		{
			int	AC					= GetSuitableAmmoTotal();
			Actor()->callback(GameObject::eWeaponNoAmmoAvailable)(lua_game_object(), AC);
		}
		if (m_ammoType < m_ammoTypes.size())
			m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType] ));
		else
			m_pAmmo = NULL;

		if (IsMisfire() && iAmmoElapsed)
		{
			SetPending(TRUE);
			SwitchState(eUnMisfire);
			return				true;
		}

		if (m_pAmmo || unlimited_ammo())
		{
			SetPending			(TRUE);
			SwitchState			(eReload); 
			return				true;
		}
 
		else for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
		{
			for (u32 i = 0; i < m_ammoTypes.size(); ++i)
			{
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if (m_pAmmo)
				{
					m_ammoType = i;
					SetPending(TRUE);
					SwitchState(eReload);
					return true;
				}
			}
		}

	}
	
	if(GetState()!=eIdle)
		SwitchState(eIdle);

	return false;
}

bool CWeaponMagazined::IsAmmoAvailable()
{
	if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType])))
		return true;
	else
	{
		for (u32 i = 0; i < m_ammoTypes.size(); ++i)
		{
			if (smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i])))
				return true;
		}
	}
	return false;
}

void CWeaponMagazined::OnMagazineEmpty() 
{

	if(GetState() == eIdle) 
	{
		OnEmptyClick			();
		return;
	}

	if( GetNextState() != eMagEmpty && GetNextState() != eReload)
	{
		SwitchState(eMagEmpty);
	}

	inherited::OnMagazineEmpty();
}

void CWeaponMagazined::UnloadMagazine(bool spawn_ammo)
{
	xr_map<LPCSTR, u16> l_ammo;
	
	while(!m_magazine.empty()) 
	{
		CCartridge &l_cartridge = m_magazine.back();
		xr_map<LPCSTR, u16>::iterator l_it;
		for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
		{
            if(!xr_strcmp(*l_cartridge.m_ammoSect, l_it->first)) 
            { 
				 ++(l_it->second); 
				 break; 
			}
		}

		if(l_it == l_ammo.end()) l_ammo[*l_cartridge.m_ammoSect] = 1;
		m_magazine.pop_back(); 
		--iAmmoElapsed;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;
	
	if (!spawn_ammo)
		return;

	xr_map<LPCSTR, u16>::iterator l_it;
	for(l_it = l_ammo.begin(); l_ammo.end() != l_it; ++l_it) 
	{
		CWeaponAmmo *l_pA = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(l_it->first));
		if(l_pA) 
		{
			u16 l_free = l_pA->m_boxSize - l_pA->m_boxCurr;
			l_pA->m_boxCurr = l_pA->m_boxCurr + (l_free < l_it->second ? l_free : l_it->second);
			l_it->second = l_it->second - (l_free < l_it->second ? l_free : l_it->second);
		}
		if(l_it->second && !unlimited_ammo()) SpawnAmmo(l_it->second, l_it->first);
	}

	if (iAmmoElapsed < 0)
		iAmmoElapsed = 0;
}

void CWeaponMagazined::ReloadMagazine() 
{
	m_dwAmmoCurrentCalcFrame = 0;	

	//��������� ������ ��� �����������
	if(IsMisfire())	bMisfire = false;
	
	if (!m_bLockType) {
		m_ammoName	= NULL;
		m_pAmmo		= NULL;
	}
	
	if (!m_pInventory) return;

	if(m_set_next_ammoType_on_reload != u32(-1)){		
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u32(-1);
	}
	
	if(!unlimited_ammo()) 
	{
		//���������� ����� � ��������� ������� �������� ���� 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pAmmo && !m_bLockType) 
		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
			{
				//��������� ������� ���� ���������� �����
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pInventory->GetAny(*m_ammoTypes[i]));
				if(m_pAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
			}
		}
	}
	else
		m_ammoType = m_ammoType;


	//��� �������� ��� �����������
	if(!m_pAmmo && !unlimited_ammo() ) return;

	//��������� �������, ���� ��������� ��������� ������� ����
	if (!m_bLockType && !m_magazine.empty() && (!m_pAmmo || xr_strcmp(m_pAmmo->cNameSect(), *m_magazine.front().m_ammoSect)))
	{
		UnloadMagazine();
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], u8(m_ammoType));
	CCartridge l_cartridge = m_DefaultCartridge;
	while(iAmmoElapsed < iMagazineSize)
	{
		if (!unlimited_ammo())
		{
			if (!m_pAmmo->Get(l_cartridge)) break;
		}
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = u8(m_ammoType);
		m_magazine.push_back(l_cartridge);
	}
	m_ammoName = (m_pAmmo) ? m_pAmmo->m_nameShort : NULL;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//�������� ������� ��������, ���� ��� ������
	if(m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
		m_pAmmo->SetDropManual(TRUE);

	if(iMagazineSize > iAmmoElapsed)
	{ 
		m_bLockType = true; 
		ReloadMagazine(); 
		m_bLockType = false; 
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}

void CWeaponMagazined::OnStateSwitch	(u32 S)
{
	inherited::OnStateSwitch(S);
	switch (S)
	{
	case eFiremodeNext:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eFiremodePrev:
	{
		PlaySound("sndFireModes", get_LastFP());
		switch2_ChangeFireMode();
	}break;
	case eIdle:
		switch2_Idle	();
		break;
	case eFire:
		switch2_Fire	();
		break;
	case eUnMisfire:
		/*if (owner)
			m_sounds_enabled = owner->CanPlayShHdRldSounds();*/
		switch2_Unmis();
		break;
	case eMisfire:
		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
			HUD().GetUI()->AddInfoMessage("gun_jammed");
		break;
	case eMagEmpty:
		switch2_Empty	();
		break;
	case eReload:
		switch2_Reload	();
		break;
	case eShowing:
		switch2_Showing	();
		break;
	case eHiding:
		switch2_Hiding	();
		break;
	case eHidden:
		switch2_Hidden	();
		break;
	}
}


void CWeaponMagazined::UpdateCL			()
{
	inherited::UpdateCL	();
	float dt = Device.fTimeDelta;

	

	//����� ���������� ������ ��������� ������
	//������ ������� �� ������
	if(GetNextState() == GetState())
	{
		switch (GetState())
		{
		case eShowing:
		case eHiding:
		case eReload:
		case eIdle:
			{
				fShotTimeCounter	-=	dt;
				clamp				(fShotTimeCounter, 0.0f, flt_max);
			}break;
		case eFire:			
			{
				state_Fire		(dt);
			}break;
		case eMisfire:		state_Misfire	(dt);	break;
		case eMagEmpty:		state_MagEmpty	(dt);	break;
		case eHidden:		break;
		}
	}

	UpdateSounds		();
}

void CWeaponMagazined::UpdateSounds	()
{
	if (Device.dwFrame == dwUpdateSounds_Frame)  
		return;
	
	dwUpdateSounds_Frame = Device.dwFrame;

	Fvector P						= get_LastFP();
	m_sounds.SetPosition("sndShow", P);
	m_sounds.SetPosition("sndHide", P);
	if (psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		m_sounds.SetPosition("sndClose", P);
	if (WeaponSoundExist(m_section_id.c_str(), "snd_changefiremode"))
		m_sounds.SetPosition("sndFireModes", P);
//. nah	m_sounds.SetPosition("sndShot", P);
	m_sounds.SetPosition("sndReload", P);
//. nah	m_sounds.SetPosition("sndEmptyClick", P);
}

void CWeaponMagazined::state_Fire(float dt)
{
	if(iAmmoElapsed > 0)
	{
		VERIFY(fOneShotTime>0.f);

		Fvector					p1, d; 
		p1.set(get_LastFP());
		d.set(get_LastFD());

		if (!H_Parent()) return;
		if (smart_cast<CMPPlayersBag*>(H_Parent()) != NULL)
		{
			Msg("! WARNING: state_Fire of object [%d][%s] while parent is CMPPlayerBag...", ID(), cNameSect().c_str());
			return;
		}

		CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
		if(NULL == io->inventory().ActiveItem())
		{
				Log("current_state", GetState() );
				Log("next_state", GetNextState());
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
		}

		CEntity* E = smart_cast<CEntity*>(H_Parent());
		E->g_fireParams	(this, p1,d);

		if( !E->g_stateFire() )
			StopShooting();

		if (m_iShotNum == 0)
		{
			m_vStartPos = p1;
			m_vStartDir = d;
		};
		
		VERIFY(!m_magazine.empty());

		while (	!m_magazine.empty() && 
				fShotTimeCounter<0 && 
				(IsWorking() || m_bFireSingleShot) && 
				(m_iQueueSize<0 || m_iShotNum<m_iQueueSize)
			   )
		{
			if( CheckForMisfire() )
			{
				StopShooting();
				return;
			}

			m_bFireSingleShot		= false;

			fShotTimeCounter		+=	fOneShotTime;
			
			++m_iShotNum;
			
			OnShot					();

			if (m_iShotNum>m_iShootEffectorStart)
				FireTrace		(p1,d);
			else
				FireTrace		(m_vStartPos, m_vStartDir);
		}
	
		if(m_iShotNum == m_iQueueSize)
			m_bStopedAfterQueueFired = true;

		UpdateSounds			();
	}

	if(fShotTimeCounter<0)
	{
/*
		if(bDebug && H_Parent() && (H_Parent()->ID() != Actor()->ID()))
		{
			Msg("stop shooting w=[%s] magsize=[%d] sshot=[%s] qsize=[%d] shotnum=[%d]",
					IsWorking()?"true":"false", 
					m_magazine.size(),
					m_bFireSingleShot?"true":"false",
					m_iQueueSize,
					m_iShotNum);
		}
*/
		if(iAmmoElapsed == 0)
			OnMagazineEmpty();

		StopShooting();
	}
	else
	{
		fShotTimeCounter			-=	dt;
	}

	if (m_fFactor > 0)
		StopShooting();
}

void CWeaponMagazined::state_Misfire	(float dt)
{
	OnEmptyClick			();
	SwitchState				(eIdle);
	
	bMisfire				= true;

	UpdateSounds			();
}

void CWeaponMagazined::state_MagEmpty	(float dt)
{
}

void CWeaponMagazined::SetDefaults	()
{
	CWeapon::SetDefaults		();
}


void CWeaponMagazined::OnShot()
{
	// ���� ����� ����� - ������������� ���
	if (ParentIsActor() && GameConstants::GetStopActorIfShoot())
		Actor()->set_state_wishful(Actor()->get_state_wishful() & (~mcSprint));

	// Sound
	if (IsSilencerAttached() && SilencerAttachable()) //skyloader: dont touch SilencerAttachable(), it needs for pb, vss, val
		PlaySound("sndSilencerShot", get_LastFP());
	else
	{
		if (m_bHasDistantShotSound && GameConstants::GetDistantSoundsEnabled() && Position().distance_to(Device.vCameraPosition) > GameConstants::GetDistantSndDistance() && Position().distance_to(Device.vCameraPosition) < GameConstants::GetDistantSndDistanceFar())
			PlaySound("sndShotDist", get_LastFP());
		else if (m_bHasDistantShotSound && GameConstants::GetDistantSoundsEnabled() && Position().distance_to(Device.vCameraPosition) > GameConstants::GetDistantSndDistanceFar())
			PlaySound("sndShotDistFar", get_LastFP());
		else
			PlaySound("sndShot", get_LastFP());
	}

	// ��������� ���� ����� ��������, ���� �� ����� �������� �� ����� ������ ��� ������ � ��� ����
	if (m_sounds.FindSoundItem("sndPumpGun", false))
		PlaySound("sndPumpGun", get_LastFP());

	// Camera	
	AddShotEffector				();

	// Animation
	PlayAnimShoot				();
	
	// Shell Drop
	Fvector vel; 
	PHGetLinearVell				(vel);
	OnShellDrop					(get_LastSP(), vel);
	
	// ����� �� ������
	StartFlameParticles			();

	//��� �� ������
	ForceUpdateFireParticles	();
	StartSmokeParticles			(get_LastFP(), vel);

	// snd reflection
	if (IsSilencerAttached() == false)
	{
		bool bIndoor = false;
		if (H_Parent() != nullptr)
		{
			bIndoor = H_Parent()->renderable_ROS()->get_luminocity_hemi() < WEAPON_INDOOR_HEMI_FACTOR;
		}

		if (bIndoor && m_sounds.FindSoundItem("sndReflect", false))
		{
			if (IsHudModeNow())
			{
				HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(WEAPON_SND_REFLECTION_HUD_FACTOR);
			}
			PlaySound("sndReflect", get_LastFP());
			HUD_SOUND_ITEM::SetHudSndGlobalVolumeFactor(1.0f);
		}
	}
}


void CWeaponMagazined::OnEmptyClick	()
{
	PlaySound	("sndEmptyClick",get_LastFP());
}

void CWeaponMagazined::OnAnimationEnd(u32 state) 
{
	switch(state) 
	{
		case eReload:
		{
			CheckMagazine(); // �������� �� ��������� �� Lost Alpha: New Project
							 // ������: rafa & Kondr48

			CCartridge FirstBulletInGun;

			bool bNeedputBullet = iAmmoElapsed > 0;

			if (m_bNeedBulletInGun && bNeedputBullet)
			{
				FirstBulletInGun = m_magazine.back();
				m_magazine.pop_back();
				iAmmoElapsed--;
			}

			ReloadMagazine();

			if (m_bNeedBulletInGun && bNeedputBullet)
			{
				m_magazine.push_back(FirstBulletInGun);
				iAmmoElapsed++;
			}

			SwitchState(eIdle);

		}break;// End of reload animation
		case eHiding:	SwitchState(eHidden);   break;	// End of Hide
		case eShowing:	SwitchState(eIdle);		break;	// End of Show
		case eIdle:		switch2_Idle();			break;  // Keep showing idle
		case eUnMisfire:
		{
			bMisfire = false;
			iAmmoElapsed--;
			SwitchState(eIdle);
		}break; // End of UnMisfire animation
		case eFiremodePrev:
		{
			SwitchState(eIdle);
			break;
		}
		case eFiremodeNext:
		{
			SwitchState(eIdle);
			break;
		}
	}
	inherited::OnAnimationEnd(state);
}

void CWeaponMagazined::switch2_Idle	()
{
	SetPending			(FALSE);
	PlayAnimIdle		();
}

void CWeaponMagazined::switch2_ChangeFireMode()
{
	if (GetState() != eFiremodeNext && GetState() != eFiremodePrev)
		return;

	FireEnd();
	PlayAnimFireMode();
	SetPending(TRUE);
}

void CWeaponMagazined::PlayAnimFireMode()
{
	//string_path guns_firemode_anm{};
	//strconcat(sizeof(guns_firemode_anm), guns_firemode_anm, "anm_changefiremode_from_", (m_iCurFireMode == 0) ? "a_to_1" : (m_iCurFireMode == 1) ? "1_to_2" : (m_iCurFireMode == 2) ? "2_to_a" : "a_to_1"); //��� ����� �����-������ ����������

	if (IsGrenadeLauncherAttached())
		PlayHUDMotion("anm_changefiremode_from_1_to_a", true, this, GetState());
	else
		PlayHUDMotion("anm_changefiremode_from_1_to_a", true, this, GetState());
}

#ifdef DEBUG
#include "ai\stalker\ai_stalker.h"
#include "object_handler_planner.h"
#endif
void CWeaponMagazined::switch2_Fire	()
{
	CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
	CInventoryItem* ii		= smart_cast<CInventoryItem*>(this);
#ifdef DEBUG
	VERIFY2					(io,make_string("no inventory owner, item %s",*cName()));

	if (ii != io->inventory().ActiveItem())
		Msg					("! not an active item, item %s, owner %s, active item %s",*cName(),*H_Parent()->cName(),io->inventory().ActiveItem() ? *io->inventory().ActiveItem()->object().cName() : "no_active_item");

	if ( !(io && (ii == io->inventory().ActiveItem())) ) 
	{
		CAI_Stalker			*stalker = smart_cast<CAI_Stalker*>(H_Parent());
		if (stalker) {
			stalker->planner().show						();
			stalker->planner().show_current_world_state	();
			stalker->planner().show_target_world_state	();
		}
	}
#else
	if (!io)
		return;
#endif // DEBUG

//
//	VERIFY2(
//		io && (ii == io->inventory().ActiveItem()),
//		make_string(
//			"item[%s], parent[%s]",
//			*cName(),
//			H_Parent() ? *H_Parent()->cName() : "no_parent"
//		)
//	);

	m_bStopedAfterQueueFired = false;
	m_bFireSingleShot = true;
	m_iShotNum = 0;

    if((OnClient() || Level().IsDemoPlay())&& !IsWorking())
		FireStart();

}

void CWeaponMagazined::switch2_Empty()
{
	OnZoomOut();

	if (m_bAutoreloadEnabled)
	{
		if (!TryReload())
		{
			OnEmptyClick();
		}
		else
		{
			inherited::FireEnd();
		}
	}
	else
	{ 
		OnEmptyClick();
	}
}
void CWeaponMagazined::PlayReloadSound()
{
	if (iAmmoElapsed == 0)
		if (m_sounds.FindSoundItem("sndReloadEmpty", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
			PlaySound("sndReloadEmpty", get_LastFP());
		else
			PlaySound("sndReload", get_LastFP());
	else
		PlaySound("sndReload", get_LastFP());
}

void CWeaponMagazined::switch2_Reload()
{
	CWeapon::FireEnd	();

	PlayReloadSound		();
	PlayAnimReload		();
	SetPending			(TRUE);
}
void CWeaponMagazined::switch2_Hiding()
{
	OnZoomOut();
	CWeapon::FireEnd();
	
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY) && WeaponSoundExist(m_section_id.c_str(), "snd_close"))
		PlaySound("sndClose", get_LastFP());
	else
		PlaySound("sndHide", get_LastFP());

	PlayAnimHide		();
	SetPending			(TRUE);
}

void CWeaponMagazined::switch2_Unmis()
{
	VERIFY(GetState() == eUnMisfire);

	if (m_sounds.FindSoundItem("sndReloadMisfire", false) && psWpnAnimsFlag.test(ANM_MISFIRE))
		PlaySound("sndReloadMisfire", get_LastFP());
	else if (m_sounds.FindSoundItem("sndReloadJammed", false) && isHUDAnimationExist("anm_reload_jammed"))
		PlaySound("sndReloadJammed", get_LastFP());
	else if (m_sounds.FindSoundItem("sndReloadEmpty", false) && psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
		PlaySound("sndReloadEmpty", get_LastFP());
	else
		PlayReloadSound();

	if (psWpnAnimsFlag.test(ANM_MISFIRE) || isHUDAnimationExist("anm_reload_jammed"))
	{
		PlayHUDMotionIfExists({ "anm_reload_misfire", "anm_reload_jammed", "anm_reload" }, true, GetState());
		// Shell Drop
		Fvector vel;
		PHGetLinearVell(vel);
		OnShellDrop(get_LastSP(), vel);
	}
	else if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY))
		PlayHUDMotionIfExists({ "anm_reload_empty", "anm_reload" }, true, GetState());
	else
		PlayAnimReload();
}

void CWeaponMagazined::switch2_Hidden()
{
	CWeapon::FireEnd();

	StopCurrentAnimWithoutCallback();

	signal_HideComplete		();
	RemoveShotEffector		();
	m_nearwall_last_hud_fov = psHUD_FOV_def;
}
void CWeaponMagazined::switch2_Showing()
{
	PlaySound			("sndShow",get_LastFP());

	SetPending			(TRUE);
	PlayAnimShow		();
}

#include "CustomDetector.h"

bool CWeaponMagazined::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;
	
	//���� ������ ���-�� ������, �� ������ �� ������
	if(IsPending()) return false;
	
	switch(cmd) 
	{
	case kWPN_RELOAD:
		{
			if (Actor()->mstate_real & (mcSprint) && !GameConstants::GetReloadIfSprint())
				break;
			else if (flags & CMD_START)
				if (iAmmoElapsed < iMagazineSize || IsMisfire())
				{
					if (GetState() == eUnMisfire) // Rietmon: ��������� �����������, ���� ������ ����� �������������� �������
						return false;

					PIItem Det = Actor()->inventory().ItemFromSlot(DETECTOR_SLOT);
					if (!Det)
						Reload(); // Rietmon: ���� � ����� ���� ���������, �� �� �� ����� ���� �������

					if (Det)
					{
						CCustomDetector* pDet = smart_cast<CCustomDetector*>(Det);
						if (!pDet->IsWorking())
							Reload();
					}
				}
		} 
		return true;
	case kWPN_FIREMODE_PREV:
		{
			if(flags&CMD_START) 
			{
				OnPrevFireMode();
				return true;
			};
		}break;
	case kWPN_FIREMODE_NEXT:
		{
			if(flags&CMD_START) 
			{
				OnNextFireMode();
				return true;
			};
		}break;
	}
	return false;
}

bool CWeaponMagazined::CanAttach(PIItem pIItem)
{
	CScope*				pScope = smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == pIItem->object().cNameSect())
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
					return true;
			}
		}
		return false;
	}
	else if (pSilencer &&
		m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
		(m_sSilencerName == pIItem->object().cNameSect()))
		return true;
	else if (pGrenadeLauncher &&
		m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
		(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
		return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazined::CanDetach(const char* item_section_name)
{
	if (m_eScopeStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope))/* &&
		(m_scopes[cur_scope]->m_sScopeName	== item_section_name))*/
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == item_section_name)
					return true;
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == item_section_name)
					return true;
			}
		}
		return false;
	}
	//	   return true;
	else if (m_eSilencerStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) &&
		(m_sSilencerName == item_section_name))
		return true;
	else if (m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
		(m_sGrenadeLauncherName == item_section_name))
		return true;
	else
		return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazined::Attach(PIItem pIItem, bool b_send_event)
{
	bool result = false;

	CScope*				pScope = smart_cast<CScope*>(pIItem);
	CSilencer*			pSilencer = smart_cast<CSilencer*>(pIItem);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);

	if (pScope &&
		m_eScopeStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope) == 0 /*&&
		(m_scopes[cur_scope]->m_sScopeName == pIItem->object().cNameSect())*/)
	{
		SCOPES_VECTOR_IT it = m_scopes.begin();
		for (; it != m_scopes.end(); it++)
		{
			if (bUseAltScope)
			{
				if (*it == pIItem->object().cNameSect())
					m_cur_scope = u8(it - m_scopes.begin());
			}
			else
			{
				if (pSettings->r_string((*it), "scope_name") == pIItem->object().cNameSect())
					m_cur_scope = u8(it - m_scopes.begin());
			}
		}
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonScope;
		result = true;
	}
	else if (pSilencer &&
		m_eSilencerStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0 &&
		(m_sSilencerName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonSilencer;
		result = true;
	}
	else if (pGrenadeLauncher &&
		m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
		(m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0 &&
		(m_sGrenadeLauncherName == pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;
		result = true;
	}

	if (result)
	{
		if (pScope && bUseAltScope)
		{
			bNVsecondVPstatus = !!pSettings->line_exist(pIItem->object().cNameSect(), "scope_nightvision");
		}

		if (b_send_event && OnServer())
		{
			//���������� �������������� ���� �� ���������
//.			pIItem->Drop					();
			pIItem->object().DestroyObject();
		};

		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return true;
	}
	else
		return inherited::Attach(pIItem, b_send_event);
}

bool CWeaponMagazined::Detach(const char* item_section_name, bool b_spawn_item)
{
	if(		m_eScopeStatus == ALife::eAddonAttachable &&
			DetachScope(item_section_name, b_spawn_item))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonScope) == 0)
		{
			Msg("ERROR: scope addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonScope;
		
		UpdateAltScope();
		UpdateAddonsVisibility();
		InitAddons();

		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if(m_eSilencerStatus == ALife::eAddonAttachable &&
			(m_sSilencerName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonSilencer) == 0)
		{
			Msg("ERROR: silencer addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonSilencer;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else if(m_eGrenadeLauncherStatus == ALife::eAddonAttachable &&
			(m_sGrenadeLauncherName == item_section_name))
	{
		if ((m_flagsAddOnState & CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) == 0)
		{
			Msg("ERROR: grenade launcher addon already detached.");
			return true;
		}
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;

		UpdateAddonsVisibility();
		InitAddons();
		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else
		return inherited::Detach(item_section_name, b_spawn_item);;
}

bool CWeaponMagazined::DetachScope(const char* item_section_name, bool b_spawn_item)
{
	bool detached = false;
	SCOPES_VECTOR_IT it = m_scopes.begin();
	shared_str iter_scope_name = "none";
	for (; it != m_scopes.end(); it++)
	{
		if (bUseAltScope)
		{
			iter_scope_name = (*it);
		}
		else
		{
			iter_scope_name = pSettings->r_string((*it), "scope_name");
		}
		if (!xr_strcmp(iter_scope_name, item_section_name))
		{
			m_cur_scope = NULL;
			m_cur_scope_bone = NULL;
			detached = true;
		}
	}
	return detached;
}

/*
void CWeaponMagazined::LoadAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS( pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f );

}
*/
void CWeaponMagazined::InitAddons()
{
	m_zoom_params.m_fIronSightZoomFactor = READ_IF_EXISTS( pSettings, r_float, cNameSect(), "ironsight_zoom_factor", 50.0f );

	SetAnimFlag(ANM_SHOT_AIM,		"anm_shots_when_aim");
	SetAnimFlag(ANM_SHOT_AIM_GL,	"anm_shots_w_gl_when_aim");

	if (IsScopeAttached())
	{
		if (m_eScopeStatus == ALife::eAddonAttachable)
		{
			LoadCurrentScopeParams(GetScopeName().c_str());

			if (pSettings->line_exist(m_scopes[m_cur_scope], "bones"))
			{
				pcstr ScopeBone = pSettings->r_string(m_scopes[m_cur_scope], "bones");
				m_cur_scope_bone = ScopeBone;
			}
		}
	}
	else
	{
		if (m_UIScope)
			xr_delete(m_UIScope);

		if (bIsSecondVPZoomPresent())
			m_zoom_params.m_fSecondVPFovFactor = 0.0f;

		if (IsZoomEnabled())
			m_zoom_params.m_fIronSightZoomFactor = pSettings->r_float(cNameSect(), "scope_zoom_factor");
	}

	if (IsSilencerAttached() && SilencerAttachable())
	{
		m_sFlameParticlesCurrent = m_sSilencerFlameParticles;
		m_sSmokeParticlesCurrent = m_sSilencerSmokeParticles;

		//��������� �� ��������
		LoadLights(*cNameSect(), "silencer_");
		ApplySilencerKoeffs();
	}
	else
	{
		m_sFlameParticlesCurrent = m_sFlameParticles;
		m_sSmokeParticlesCurrent = m_sSmokeParticles;

		//��������� �� ��������
		LoadLights(*cNameSect(), "");
		ResetSilencerKoeffs();
	}

	inherited::InitAddons();
}

void CWeaponMagazined::LoadSilencerKoeffs()
{
	if ( m_eSilencerStatus == ALife::eAddonAttachable )
	{
		LPCSTR sect = m_sSilencerName.c_str();
		m_silencer_koef.hit_power		= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_hit_power_k", 1.0f );
		m_silencer_koef.hit_impulse		= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_hit_impulse_k", 1.0f );
		m_silencer_koef.bullet_speed	= READ_IF_EXISTS( pSettings, r_float, sect, "bullet_speed_k", 1.0f );
		m_silencer_koef.fire_dispersion	= READ_IF_EXISTS( pSettings, r_float, sect, "fire_dispersion_base_k", 1.0f );
		m_silencer_koef.cam_dispersion	= READ_IF_EXISTS( pSettings, r_float, sect, "cam_dispersion_k", 1.0f );
		m_silencer_koef.cam_disper_inc	= READ_IF_EXISTS( pSettings, r_float, sect, "cam_dispersion_inc_k", 1.0f );
	}

	clamp( m_silencer_koef.hit_power,		0.0f, 1.0f );
	clamp( m_silencer_koef.hit_impulse,		0.0f, 1.0f );
	clamp( m_silencer_koef.bullet_speed,	0.0f, 1.0f );
	clamp( m_silencer_koef.fire_dispersion,	0.0f, 3.0f );
	clamp( m_silencer_koef.cam_dispersion,	0.0f, 1.0f );
	clamp( m_silencer_koef.cam_disper_inc,	0.0f, 1.0f );
}

void CWeaponMagazined::ApplySilencerKoeffs()
{
	cur_silencer_koef = m_silencer_koef;
}

void CWeaponMagazined::ResetSilencerKoeffs()
{
	cur_silencer_koef.Reset();
}

void CWeaponMagazined::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);

	if (iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SHOW_EMPTY))
		PlayHUDMotion("anm_show_empty", FALSE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_show_jammed"))
		PlayHUDMotion("anm_show_jammed", false, this, GetState());
	else
		PlayHUDMotion("anm_show", FALSE, this, GetState());
}

void CWeaponMagazined::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_HIDE_EMPTY))
		PlayHUDMotion("anm_hide_empty", TRUE, this, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_hide_jammed"))
		PlayHUDMotion("anm_hide_jammed", true, this, GetState());
	else
		PlayHUDMotion("anm_hide", TRUE, this, GetState());
}

void CWeaponMagazined::PlayAnimBore()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_BORE_EMPTY))
		PlayHUDMotion("anm_bore_empty", TRUE, this, GetState());
	else
		inherited::PlayAnimBore();
}

void CWeaponMagazined::PlayAnimIdleSprint()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_SPRINT_EMPTY))
		PlayHUDMotion("anm_idle_sprint_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_sprint_jammed"))
		PlayHUDMotion("anm_idle_sprint_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleSprint();
}

void CWeaponMagazined::PlayAnimIdleMoving()
{
	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_MOVING_EMPTY))
		PlayHUDMotion("anm_idle_moving_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_moving_jammed"))
		PlayHUDMotion("anm_idle_moving_jammed", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMoving();
}

void CWeaponMagazined::PlayAnimReload()
{
	VERIFY(GetState()==eReload);

	if (iAmmoElapsed == 0)
		PlayHUDMotionIfExists({ "anm_reload_empty", "anm_reload" }, true, GetState());
	else
		PlayHUDMotion("anm_reload", TRUE, this, GetState());
}

const char* CWeaponMagazined::GetAnimAimName()
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (pActor)
	{
		const u32 state = pActor->get_state();

		if (state && state & mcAnyMove)
		{
			if (IsScopeAttached())
			{
				strcpy_s(guns_aim_anm, "anm_idle_aim_scope_moving");
				return guns_aim_anm;
			}
			else
				return strconcat(sizeof(guns_aim_anm), guns_aim_anm, "anm_idle_aim_moving", (state & mcFwd) ? "_forward" : ((state & mcBack) ? "_back" : ""), (state & mcLStrafe) ? "_left" : ((state & mcRStrafe) ? "_right" : ""));
		}
	}
	return nullptr;
}

void CWeaponMagazined::PlayAnimAim()
{
	if (IsRotatingToZoom())
	{
		if (isHUDAnimationExist("anm_idle_aim_start"))
		{
			PlayHUDMotionNew("anm_idle_aim_start", true, GetState());
			return;
		}
	}

	if (const char* guns_aim_anm = GetAnimAimName())
	{
		if (isHUDAnimationExist(guns_aim_anm))
		{
			PlayHUDMotionNew(guns_aim_anm, true, GetState());
			return;
		}
	}

	if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_AIM_EMPTY))
		PlayHUDMotion("anm_idle_aim_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_aim_jammed"))
		PlayHUDMotion("anm_idle_aim_jammed", true, nullptr, GetState());
	else
		PlayHUDMotion("anm_idle_aim", TRUE, NULL, GetState());
}

void CWeaponMagazined::PlayAnimIdle()
{
	VERIFY(GetState()==eIdle);

	if (TryPlayAnimIdle()) return;

	if(IsZoomed())
		PlayAnimAim();
	else if (iAmmoElapsed == 0 && psWpnAnimsFlag.test(ANM_IDLE_EMPTY))
		PlayHUDMotion("anm_idle_empty", TRUE, NULL, GetState());
	else if (IsMisfire() && isHUDAnimationExist("anm_idle_jammed") && !TryPlayAnimIdle())
		PlayHUDMotion("anm_idle_jammed", true, nullptr, GetState());
	else
	{
		if (IsRotatingFromZoom())
		{
			if (isHUDAnimationExist("anm_idle_aim_end"))
			{
				PlayHUDMotionNew("anm_idle_aim_end", true, GetState());
				return;
			}
		}
		inherited::PlayAnimIdle();
	}
}

void CWeaponMagazined::PlayAnimShoot()
{
	VERIFY(GetState()==eFire);

	string_path guns_shoot_anm{};
	strconcat(sizeof(guns_shoot_anm), guns_shoot_anm, "anm_shoot", (IsZoomed() && !IsRotatingToZoom()) ? (IsScopeAttached() ? "_aim_scope" : "_aim") : "", IsSilencerAttached() ? "_sil" : "");

	if (IsZoomed() && psWpnAnimsFlag.test(ANM_SHOT_AIM) && IsScopeAttached())
		PlayHUDMotion("anm_shots_when_aim", FALSE, this, GetState());
	else if (iAmmoElapsed == 1 && psWpnAnimsFlag.test(ANM_SHOT_EMPTY))
		PlayHUDMotion("anm_shot_l", FALSE, this, GetState());
	else
		PlayHUDMotionIfExists({ "anm_shoot", "anm_shots" }, false, GetState());
}

void CWeaponMagazined::OnZoomIn			()
{
	inherited::OnZoomIn();

	if(GetState() == eIdle)
		PlayAnimIdle();


	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(pActor)
	{
		CEffectorZoomInertion* S = smart_cast<CEffectorZoomInertion*>	(pActor->Cameras().GetCamEffector(eCEZoom));
		if (!S)	
		{
			S = (CEffectorZoomInertion*)pActor->Cameras().AddCamEffector(xr_new<CEffectorZoomInertion> ());
			S->Init(this);
		};
		S->SetRndSeed			(pActor->GetZoomRndSeed());
		R_ASSERT				(S);
	}
}
void CWeaponMagazined::OnZoomOut		()
{
	if(!IsZoomed())	 
		return;

	inherited::OnZoomOut	();

	if(GetState()==eIdle)
		PlayAnimIdle		();

	CActor* pActor			= smart_cast<CActor*>(H_Parent());

	if(pActor)
		pActor->Cameras().RemoveCamEffector	(eCEZoom);

}

//������������ ������� �������� ���������� � ���������
bool CWeaponMagazined::SwitchMode			()
{
	if(eIdle != GetState() || IsPending()) return false;

	if(SingleShotMode())
		m_iQueueSize = WEAPON_ININITE_QUEUE;
	else
		m_iQueueSize = 1;
	
	PlaySound	("sndEmptyClick", get_LastFP());

	return true;
}
 
void	CWeaponMagazined::OnNextFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a"))
		SwitchState(eFiremodeNext);

	if (GetState() != eIdle) return;
	m_iCurFireMode = (m_iCurFireMode+1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());
};

void	CWeaponMagazined::OnPrevFireMode		()
{
	if (!m_bHasDifferentFireModes) return;

	if (isHUDAnimationExist("anm_changefiremode_from_1_to_a"))
		SwitchState(eFiremodePrev);

	if (GetState() != eIdle) return;
	m_iCurFireMode = (m_iCurFireMode-1+m_aFireModes.size()) % m_aFireModes.size();
	SetQueueSize(GetCurrentFireMode());	
};

void	CWeaponMagazined::OnH_A_Chield		()
{
	if (m_bHasDifferentFireModes)
	{
		CActor	*actor = smart_cast<CActor*>(H_Parent());
		if (!actor) SetQueueSize(-1);
		else SetQueueSize(GetCurrentFireMode());
	};	
	inherited::OnH_A_Chield();
};

void	CWeaponMagazined::SetQueueSize			(int size)  
{
	m_iQueueSize = size; 
};

float	CWeaponMagazined::GetWeaponDeterioration	()
{
	if (!m_bHasDifferentFireModes || m_iPrefferedFireMode == -1 || u32(GetCurrentFireMode()) <= u32(m_iPrefferedFireMode)) 
		return inherited::GetWeaponDeterioration();
	return m_iShotNum*conditionDecreasePerShot;
};

void CWeaponMagazined::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(m_iQueueSize, output_packet);
	save_data		(m_iShotNum, output_packet);
	save_data		(m_iCurFireMode, output_packet);
}

void CWeaponMagazined::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(m_iQueueSize, input_packet);SetQueueSize(m_iQueueSize);
	load_data		(m_iShotNum, input_packet);
	load_data		(m_iCurFireMode, input_packet);
}

void CWeaponMagazined::net_Export	(NET_Packet& P)
{
	inherited::net_Export (P);

	P.w_u8(u8(m_iCurFireMode&0x00ff));
}

void CWeaponMagazined::net_Import	(NET_Packet& P)
{
	inherited::net_Import (P);

	m_iCurFireMode = P.r_u8();
	SetQueueSize(GetCurrentFireMode());
}

#include "string_table.h"
void CWeaponMagazined::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count, string16& fire_mode )
{
	int	AE		= GetAmmoElapsed();
	int	AC		= 0;
	if ( IsGameTypeSingle() )
	{
		AC		= GetCurrentTypeAmmoTotal();
	}
	else
	{
		AC		= GetSuitableAmmoTotal();//mp = all type
	}
	
	if(AE==0 || 0==m_magazine.size() )
		icon_sect_name	= *m_ammoTypes[m_ammoType];
	else
		icon_sect_name	= *m_ammoTypes[m_magazine.back().m_LocalAmmoType];


	string256		sItemName;
	strcpy_s			(sItemName, *CStringTable().translate(pSettings->r_string(icon_sect_name.c_str(), "inv_name_short")));

	strcpy_s( fire_mode, sizeof(fire_mode), "" );
	if ( HasFireModes() )
	{
		if (m_iQueueSize == -1)
			strcpy_s(fire_mode, "A");
		else
			sprintf_s(fire_mode, "%d", m_iQueueSize);
	}

	str_name		= sItemName;

	{
		if (!unlimited_ammo())
			sprintf_s			(sItemName, "%d/%d",AE,AC - AE);
		else
			sprintf_s			(sItemName, "%d/--",AE);

		str_count				= sItemName;
	}
}

bool CWeaponMagazined::install_upgrade_impl( LPCSTR section, bool test )
{
	bool result = inherited::install_upgrade_impl( section, test );
	
	LPCSTR str;
	// fire_modes = 1, 2, -1
	bool result2 = process_if_exists_set( section, "fire_modes", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		int ModesCount = _GetItemCount( str );
		m_aFireModes.clear();
		for ( int i = 0; i < ModesCount; ++i )
		{
			string16 sItem;
			_GetItem( str, i, sItem );
			m_aFireModes.push_back( (s8)atoi(sItem) );
		}
		m_iCurFireMode = ModesCount - 1;
	}
	result |= result2;

	result |= process_if_exists( section, "dispersion_start", &CInifile::r_s32, m_iShootEffectorStart, test );

	// sounds (name of the sound, volume (0.0 - 1.0), delay (sec))
	result2 = process_if_exists_set( section, "snd_draw", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_draw"	    , "sndShow"		, m_eSoundShow		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_holster", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_holster"	, "sndHide"		, m_eSoundHide		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_shoot", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_shoot"	, "sndShot"		, m_eSoundShot		);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_empty", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_empty"	, "sndEmptyClick"	, m_eSoundEmptyClick);	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_reload", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_reload"	, "sndReload"		, m_eSoundReload	);	}
	result |= result2;

	result2 = process_if_exists_set(section, "snd_reflect", &CInifile::r_string, str, test);
	if (result2 && !test) { m_sounds.LoadSound(section, "snd_reflect", "sndReflect"			, m_eSoundReflect); }
	result |= result2;

	result2 = process_if_exists_set(section, "snd_shoot_dist", &CInifile::r_string, str, test);
	if (result2 && !test)
	{
		m_sounds.LoadSound(section, "snd_shoot_distant", "sndShotDist", m_eSoundShot);
		m_sounds.LoadSound(section, "snd_shoot_distant_far", "sndShotDistFar", m_eSoundShot);
		m_bHasDistantShotSound = true;
	}
	result |= result2;

	//snd_shoot1     = weapons\ak74u_shot_1 ??
	//snd_shoot2     = weapons\ak74u_shot_2 ??
	//snd_shoot3     = weapons\ak74u_shot_3 ??

	if ( m_eSilencerStatus == ALife::eAddonAttachable )
	{
		result |= process_if_exists_set( section, "silencer_flame_particles", &CInifile::r_string, m_sSilencerFlameParticles, test );
		result |= process_if_exists_set( section, "silencer_smoke_particles", &CInifile::r_string, m_sSilencerSmokeParticles, test );

		result2 = process_if_exists_set( section, "snd_silncer_shot", &CInifile::r_string, str, test );
		if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_silncer_shot"	, "sndSilencerShot", m_eSoundShot	);	}
		result |= result2;
	}

	// fov for zoom mode
	result |= process_if_exists( section, "ironsight_zoom_factor", &CInifile::r_float, m_zoom_params.m_fIronSightZoomFactor, test );

	if( IsScopeAttached() )
	{
		//if ( m_eScopeStatus == ALife::eAddonAttachable )
		{
			result |= process_if_exists( section, "scope_zoom_factor", &CInifile::r_float, m_zoom_params.m_fScopeZoomFactor, test );
		}
	}
	else
	{
		if( IsZoomEnabled() )
		{
			result |= process_if_exists( section, "scope_zoom_factor", &CInifile::r_float, m_zoom_params.m_fIronSightZoomFactor, test );
		}
	}

	return result;
}

// AVO: for custom added sounds check if sound exists
bool CWeaponMagazined::WeaponSoundExist(LPCSTR section, LPCSTR sound_name) const
{
	pcstr str;
	bool sec_exist = process_if_exists_set(section, sound_name, &CInifile::r_string, str, true);
	if (sec_exist)
		return true;
#ifdef DEBUG
	Msg("~ [WARNING] ------ Sound [%s] does not exist in [%s]", sound_name, section);
#endif
	return false;
}

void CWeaponMagazined::CheckMagazine()
{
	if (!ParentIsActor())
	{
		m_bNeedBulletInGun = false;
		return;
	}

	if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed >= 1 && m_bNeedBulletInGun == false)
	{
		m_bNeedBulletInGun = true;
	}
	else if (psWpnAnimsFlag.test(ANM_RELOAD_EMPTY) && iAmmoElapsed == 0 && m_bNeedBulletInGun == true)
	{
		m_bNeedBulletInGun = false;
	}
}