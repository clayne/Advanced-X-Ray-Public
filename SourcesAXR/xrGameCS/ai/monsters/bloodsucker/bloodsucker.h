#pragma once

#include "../BaseMonster/base_monster.h"
#include "../ai_monster_bones.h"
#include "../controlled_entity.h"
#include "../controlled_actor.h"
#include "../anim_triple.h"
#include "../../../../XrServerEntitiesCS/script_export_space.h"
#include "bloodsucker_alien.h"


class CAI_Bloodsucker : public CBaseMonster, 
						public CControlledActor {

	typedef		CBaseMonster	inherited;
	
public:
							CAI_Bloodsucker	();
	virtual					~CAI_Bloodsucker();	

	virtual void			reinit					();
	virtual	void			reload					(LPCSTR section);

	virtual void			UpdateCL				();
	virtual void			shedule_Update			(u32 dt);
	virtual void			Die						(CObject* who);
	virtual BOOL			net_Spawn				(CSE_Abstract* DC);
	virtual	void			Load					(LPCSTR section);
	virtual	void			Hit						(SHit* pHDS);

	virtual	void			CheckSpecParams			(u32 spec_params);
	virtual bool			ability_invisibility	() {return true;}
	virtual bool			ability_pitch_correction() {return false;}
	virtual bool			ability_can_drag		() {return true;}

	virtual	void			post_fsm_update			();

	
	virtual bool			use_center_to_aim		() const {return true;}
	virtual bool			check_start_conditions	(ControlCom::EControlType);
	virtual void			HitEntity				(const CEntity *pEntity, float fDamage, float impulse, Fvector &dir, ALife::EHitType hit_type, bool draw_hit_marks);
	
	//--------------------------------------------------------------------
	// Utils
	//--------------------------------------------------------------------
			void			move_actor_cam			(float angle);
			bool			is_collision_off		();
			void			set_collision_off		(bool b_collision);

	//--------------------------------------------------------------------
	// Bones
	//--------------------------------------------------------------------
private:
	static	void	_BCL		BoneCallback			(CBoneInstance *B);
			void				vfAssignBones			();
			void				LookDirection			(Fvector to_dir, float bone_turn_speed);


	bonesManipulation		Bones;


	CBoneInstance			*bone_spine;
	CBoneInstance			*bone_head;
	bool					collision_hit_off;
	bool					collision_off;

	//--------------------------------------------------------------------
	// Invisibility
	//--------------------------------------------------------------------
private:
	SMotionVel				invisible_vel;
	LPCSTR					invisible_particle_name;

public:
			void			start_invisible_predator ();
			void			stop_invisible_predator	 ();

			virtual bool    in_solid_state           ();

	//--------------------------------------------------------------------
	// Vampire
	//--------------------------------------------------------------------
public:

	u32						m_vampire_min_delay;
	SAnimationTripleData	anim_triple_vampire;

	SPPInfo					pp_vampire_effector;

			
			void			ActivateVampireEffector	();
	IC		bool			WantVampire				() {return (fsimilar(m_vampire_want_value,1.f) == TRUE);}
	IC		void			SatisfyVampire			() {m_vampire_want_value = 0.f;}

			u32             get_last_critical_hit_tick () { return m_last_critical_hit_tick; }
			void            clear_last_critical_hit_tick () { m_last_critical_hit_tick = 0; }
private:
	TTime                   m_last_critical_hit_tick;
	float                   m_critical_hit_chance; //0..1
	float                   m_critical_hit_camera_effector_angle;

	float					m_vampire_want_value;
	float					m_vampire_want_speed;		// load from ltx
	float					m_vampire_wound;
	float					m_vis_state;
	bool					m_drag_anim_jump;
	bool					m_animated;
	static void				animation_end_jump		(CBlend* B);


			void			LoadVampirePPEffector	(LPCSTR section);	

	//--------------------------------------------------------------------
	// Alien
	//--------------------------------------------------------------------
public:
	CBloodsuckerAlien		m_alien_control;
	u32						m_time_lunge;

			void			set_alien_control		(bool val);


	//--------------------------------------------------------------------
	// Predator
	//--------------------------------------------------------------------
public:
	shared_str				m_visual_default;
	LPCSTR					m_visual_predator;
	bool					m_predator;
	bool					m_bHitIfInvisible;
	CEntityAlive			*m_cob;
	LPCSTR					m_str_cel;
	Fvector					j_position;
	float					j_factor;
			
			void			predator_start			();
			void			predator_stop			();
			void			predator_freeze			();
			void			predator_unfreeze		();
			void			set_vis					();
			void			set_invis				();
			void			jump					(const Fvector &position, float factor);
			void			start_drag				();
			void			set_drag_jump			(CEntityAlive* e, LPCSTR s, const Fvector &position, float factor);
			bool			is_drag_anim_jump		();
			bool			is_animated				();
	//--------------------------------------------------------------------
	// Sounds
	//--------------------------------------------------------------------
public:

	enum EBloodsuckerSounds {
		eAdditionalSounds		= MonsterSound::eMonsterSoundCustom,

		eVampireGrasp			= eAdditionalSounds | 0,
		eVampireSucking			= eAdditionalSounds | 1,
		eVampireHit				= eAdditionalSounds | 2,
		eVampireStartHunt		= eAdditionalSounds | 3,

		eGrowl					= eAdditionalSounds | 5,

		eChangeVisibility		= eAdditionalSounds | 6,
		
		eAlien					= eAdditionalSounds | 7,
	};

	//--------------------------------------------------------------------

public:
			void	set_manual_control	(bool value) {}
			void	manual_activate		();
			void	manual_deactivate	();

			u32     get_invisibility_activate_delay () { return m_invisibility_activate_delay; }


#ifdef DEBUG
	virtual CBaseMonster::SDebugInfo show_debug_info();

	// Lain: added 
	virtual void add_debug_info  (debug::text_tree& root_s);

#ifdef _DEBUG
			void debug_on_key						(int key);
#endif

#endif

	void    apply_actor_camera_effector ();

	float   m_camera_effector_move_angular_speed;
	TTime   m_camera_effector_start_move_tick;
	TTime   m_camera_effector_last_move_tick;
	TTime   m_camera_effector_move_time_milis;
	float   m_camera_effector_hor_angle;
	float   m_camera_effector_ver_angle;

	u32     m_invisibility_activate_delay;

	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CAI_Bloodsucker)
#undef script_type_list
#define script_type_list save_type_list(CAI_Bloodsucker)