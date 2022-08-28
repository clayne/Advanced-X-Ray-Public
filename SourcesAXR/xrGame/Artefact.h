#pragma once

#include "hud_item_object.h"
#include "hit_immunity.h"
#include "../xrphysics/PHUpdateObject.h"
#include "script_export_space.h"
#include "patrol_path.h"

class SArtefactActivation;
class CInventoryItem;
struct SArtefactDetectorsSupport;

class CArtefact :	public CHudItemObject, 
					public CPHUpdateObject 
{
	typedef			CHudItemObject	inherited;
public:
									CArtefact						();
	virtual							~CArtefact						();

	virtual void					Load							(LPCSTR section);
	virtual BOOL					net_Spawn						(CSE_Abstract* DC);
	virtual void					net_Destroy						();

	virtual void					OnH_A_Chield					();
	virtual void					OnH_B_Independent				(bool just_before_destroy);
	virtual void					save							(NET_Packet &output_packet);
	virtual void					load							(IReader &input_packet);
	virtual void					OnActiveItem					();
	virtual void					OnHiddenItem					();
	
	virtual void					UpdateCL						();
	virtual void					shedule_Update					(u32 dt);	
			void					UpdateWorkload					(u32 dt);

	
	virtual bool					CanTake							() const;

	virtual BOOL					renderable_ShadowGenerate		()		{ return FALSE;	}
	virtual BOOL					renderable_ShadowReceive		()		{ return TRUE;	}
	virtual void					create_physic_shell				();

	virtual CArtefact*				cast_artefact					()		{return this;}
	virtual	u32						Cost							() const;

protected:
	virtual void					UpdateCLChild					()		{};
	virtual void					CreateArtefactActivation			();

	SArtefactActivation*			m_activationObj;
	SArtefactDetectorsSupport*		m_detectorObj;

	u16								m_CarringBoneID;
	shared_str						m_sParticlesName;

	ref_light						m_pTrailLight;
	Fcolor							m_TrailLightColor;
	float							m_fTrailLightRange;
	u8								m_af_rank;
	bool							m_bLightsEnabled;

	virtual void					UpdateLights					();
public:
	IC u8							GetAfRank						() const		{return m_af_rank;}
	IC bool							CanBeActivated					()				{return m_bCanSpawnZone;};
	void							ActivateArtefact				();
	void							FollowByPath					(LPCSTR path_name, int start_idx, Fvector magic_force);
	bool							CanBeInvisible					();
	void							SwitchVisibility				(bool);

	void							SwitchAfParticles				(bool bOn);
	virtual void					StartLights();
	virtual void					StopLights();

	virtual void					PhDataUpdate					(float step);
	virtual void					PhTune							(float step)	{};

	float							m_additional_weight;
	float							AdditionalInventoryWeight		() const {return m_additional_weight;}
	bool							m_bCanSpawnZone;
	float							m_fHealthRestoreSpeed;
	float 							m_fRadiationRestoreSpeed;
	float 							m_fSatietyRestoreSpeed;
	float							m_fPowerRestoreSpeed;
	float							m_fBleedingRestoreSpeed;
	float 							m_fThirstRestoreSpeed;
	float 							m_fIntoxicationRestoreSpeed;
	float 							m_fSleepenessRestoreSpeed;
	float 							m_fAlcoholismRestoreSpeed;
	float 							m_fNarcotismRestoreSpeed;
	float							m_fJumpSpeed;
	float							m_fWalkAccel;
	CHitImmunity 					m_ArtefactHitImmunities;
	HitImmunity::HitTypeSVec		m_HitTypeProtection;
	HitImmunity::HitTypeSVec		m_ConstHitTypeProtection;

	//For Degradation
	float							m_fConstHealthRestoreSpeed;
	float							m_fConstRadiationRestoreSpeed;
	float							m_fConstSatietyRestoreSpeed;
	float							m_fConstPowerRestoreSpeed;
	float							m_fConstBleedingRestoreSpeed;
	float							m_fConstThirstRestoreSpeed;
	float							m_fConstIntoxicationRestoreSpeed;
	float							m_fConstSleepenessRestoreSpeed;
	float							m_fConstAlcoholismRestoreSpeed;
	float							m_fConstNarcotismRestoreSpeed;
	float							m_fConstAdditionalWeight;
	float							m_fConstTrailLightRange;
	float							m_fConstVolumetricDistance;
	float							m_fConstVolumetricIntensity;
	float							m_fConstJumpSpeed;
	float							m_fConstWalkAccel;
	float							m_fChargeLevel;
	float							m_fCurrentChargeLevel;
	float							m_fDegradationSpeed;
	float							GetCurrentChargeLevel(void) const;
	int								GetCurrentAfRank(void) const;

	bool							m_bVolumetricLights;
	float							m_fVolumetricQuality;
	float							m_fVolumetricDistance;
	float							m_fVolumetricIntensity;

	int								m_iAfRank;
public:
	enum EAFHudStates {
		eActivating = eLastBaseState+1,
	};
	virtual void					Interpolate			();

	virtual	void					PlayAnimIdle		();
	virtual void					MoveTo(Fvector const & position);
	virtual void					StopActivation		();

	virtual void					ForceTransform		(const Fmatrix& m);

	virtual void					Hide				();
	virtual void					Show				();
	virtual	void					UpdateXForm			();
	virtual bool					Action				(u16 cmd, u32 flags);
	virtual void					OnStateSwitch		(u32 S);
	virtual void					OnAnimationEnd		(u32 state);
	virtual bool					IsHidden			()	const	{return GetState()==eHidden;}

			void					UpdateDegradation	(void);

	// optimization FAST/SLOW mode
	u32						o_render_frame				;
	BOOL					o_fastmode					;
	IC void					o_switch_2_fast				()	{
		if (o_fastmode)		return	;
		o_fastmode			= TRUE	;
		//processing_activate		();
	}
	IC void					o_switch_2_slow				()	{
		if (!o_fastmode)	return	;
		o_fastmode			= FALSE	;
		//processing_deactivate		();
	}

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

struct SArtefactDetectorsSupport
{
	CArtefact*						m_parent;
	ref_sound						m_sound;

	Fvector							m_path_moving_force;
	u32								m_switchVisTime;
	const CPatrolPath*				m_currPatrolPath;
	const CPatrolPath::CVertex*		m_currPatrolVertex;
	Fvector							m_destPoint;

			SArtefactDetectorsSupport		(CArtefact* A);
			~SArtefactDetectorsSupport		();
	void	SetVisible						(bool);
	void	FollowByPath					(LPCSTR path_name, int start_idx, Fvector force);
	void	UpdateOnFrame					();
	void	Blink							();
};

add_to_type_list(CArtefact)
#undef script_type_list
#define script_type_list save_type_list(CArtefact)

