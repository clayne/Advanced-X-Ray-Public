#include "pch_script.h"

#include "uiiteminfo.h"
#include "uistatic.h"
#include "UIXmlInit.h"

#include "UIProgressBar.h"
#include "UIScrollView.h"
#include "UIFrameWindow.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "../string_table.h"
#include "../Inventory_Item.h"
#include "UIInventoryUtilities.h"
#include "../PhysicsShellHolder.h"
#include "UIWpnParams.h"
#include "UIArtefactParams.h"
#include "UIInvUpgradeProperty.h"
#include "UIOutfitInfo.h"
#include "UIBoosterInfo.h"
#include "UIInventoryItemParams.h"
#include "../Weapon.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../eatable_item.h"
#include "UICellItem.h"
#include "../AdvancedXrayGameConstants.h"
#include "../Torch.h"
#include "../CustomDetector.h"
#include "../AnomalyDetector.h"
#include "../ArtefactContainer.h"
#include "../CustomBackpack.h"

extern const LPCSTR g_inventory_upgrade_xml;

#define INV_GRID_WIDTH2(HQ_ICONS) ((HQ_ICONS) ? (80.0f) : (40.0f))
#define INV_GRID_HEIGHT2(HQ_ICONS) ((HQ_ICONS) ? (80.0f) : (40.0f))

CUIItemInfo::CUIItemInfo()
{
	UIItemImageSize.set			(0.0f,0.0f);
	
	UICost						= NULL;
	UITradeTip					= NULL;
	UIWeight					= NULL;
	UIItemImage					= NULL;
	UIDesc						= NULL;
	//UIConditionWnd				= NULL;
	UIWpnParams					= NULL;
	UIProperties				= NULL;
	UIOutfitInfo				= NULL;
	UIBoosterInfo				= NULL;
	UIArtefactParams			= NULL;
	UIInventoryItem				= NULL;
	UIName						= NULL;
	UIBackground				= NULL;
	m_pInvItem					= NULL;
	UIItemConditionParams		= NULL;
	m_b_FitToHeight				= false;
	m_complex_desc				= false;
}

CUIItemInfo::~CUIItemInfo()
{
//	xr_delete	(UIConditionWnd);
	xr_delete	(UIWpnParams);
	xr_delete	(UIArtefactParams);
	xr_delete	(UIProperties);
	xr_delete	(UIOutfitInfo);
	xr_delete	(UIBoosterInfo);
	xr_delete	(UIInventoryItem);
	xr_delete	(UIItemConditionParams);
}

void CUIItemInfo::InitItemInfo(LPCSTR xml_name)
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, xml_name);
	CUIXmlInit					xml_init;

	if(uiXml.NavigateToNode("main_frame",0))
	{
		Frect wnd_rect;
		wnd_rect.x1		= uiXml.ReadAttribFlt("main_frame", 0, "x", 0);
		wnd_rect.y1		= uiXml.ReadAttribFlt("main_frame", 0, "y", 0);

		wnd_rect.x2		= uiXml.ReadAttribFlt("main_frame", 0, "width", 0);
		wnd_rect.y2		= uiXml.ReadAttribFlt("main_frame", 0, "height", 0);
		wnd_rect.x2		+= wnd_rect.x1;
		wnd_rect.y2		+= wnd_rect.y1;
		inherited::SetWndRect(wnd_rect);
		
		delay			= uiXml.ReadAttribInt("main_frame", 0, "delay", 500);
	}
	if(uiXml.NavigateToNode("background_frame",0))
	{
		UIBackground				= xr_new<CUIFrameWindow>();
		UIBackground->SetAutoDelete	(true);
		AttachChild					(UIBackground);
		xml_init.InitFrameWindow	(uiXml, "background_frame", 0,	UIBackground);
	}
	m_complex_desc = false;
	if(uiXml.NavigateToNode("static_name",0))
	{
		UIName						= xr_new<CUITextWnd>();	 
		AttachChild					(UIName);		
		UIName->SetAutoDelete		(true);
		xml_init.InitTextWnd		(uiXml, "static_name", 0,	UIName);
		m_complex_desc				= ( uiXml.ReadAttribInt("static_name", 0, "complex_desc", 0) == 1 );
	}
	if(uiXml.NavigateToNode("static_weight",0))
	{
		UIWeight				= xr_new<CUITextWnd>();	 
		AttachChild				(UIWeight);		
		UIWeight->SetAutoDelete(true);
		xml_init.InitTextWnd		(uiXml, "static_weight", 0,			UIWeight);
	}

	if(uiXml.NavigateToNode("static_cost",0))
	{
		UICost					= xr_new<CUITextWnd>();	 
		AttachChild				(UICost);
		UICost->SetAutoDelete	(true);
		xml_init.InitTextWnd		(uiXml, "static_cost", 0,			UICost);
	}

	if(uiXml.NavigateToNode("static_no_trade",0))
	{
		UITradeTip					= xr_new<CUITextWnd>();	 
		AttachChild					(UITradeTip);
		UITradeTip->SetAutoDelete	(true);
		xml_init.InitTextWnd		(uiXml, "static_no_trade", 0,		UITradeTip);
	}

	if(uiXml.NavigateToNode("descr_list",0))
	{
//		UIConditionWnd					= xr_new<CUIConditionParams>();
//		UIConditionWnd->InitFromXml		(uiXml);

		UIItemConditionParams = xr_new<CUIItemConditionParams>();
		UIItemConditionParams->InitFromXml(uiXml);

		UIWpnParams						= xr_new<CUIWpnParams>();
		UIWpnParams->InitFromXml		(uiXml);

		UIArtefactParams				= xr_new<CUIArtefactParams>();
		UIArtefactParams->InitFromXml	(uiXml);

		UIBoosterInfo					= xr_new<CUIBoosterInfo>();
		UIBoosterInfo->InitFromXml		(uiXml);

		//UIDesc_line						= xr_new<CUIStatic>();
		//AttachChild						(UIDesc_line);	
		//UIDesc_line->SetAutoDelete		(true);
		//xml_init.InitStatic				(uiXml, "description_line", 0, UIDesc_line);

		if ( ai().get_alife() ) // (-designer)
		{
			UIProperties					= xr_new<UIInvUpgPropertiesWnd>();
			UIProperties->init_from_xml		("actor_menu_item.xml");
		}

		UIDesc							= xr_new<CUIScrollView>(); 
		AttachChild						(UIDesc);		
		UIDesc->SetAutoDelete			(true);
		m_desc_info.bShowDescrText		= !!uiXml.ReadAttribInt("descr_list",0,"only_text_info", 1);
		m_b_FitToHeight					= !!uiXml.ReadAttribInt("descr_list",0,"fit_to_height", 0);
		xml_init.InitScrollView			(uiXml, "descr_list", 0, UIDesc);
		xml_init.InitFont				(uiXml, "descr_list:font", 0, m_desc_info.uDescClr, m_desc_info.pDescFont);
	}	

	if (uiXml.NavigateToNode("image_static", 0))
	{	
		UIItemImage					= xr_new<CUIStatic>();	 
		AttachChild					(UIItemImage);	
		UIItemImage->SetAutoDelete	(true);
		xml_init.InitStatic			(uiXml, "image_static", 0, UIItemImage);
		UIItemImage->TextureOn		();

		UIItemImage->TextureOff			();
		UIItemImageSize.set				(UIItemImage->GetWidth(),UIItemImage->GetHeight());
	}
	if ( uiXml.NavigateToNode( "outfit_info", 0 ) )
	{
		UIOutfitItem				= xr_new<CUIOutfitItem>();
		UIOutfitItem->InitFromXml	(uiXml);

		UIOutfitInfo = xr_new<CUIOutfitItem>();
		UIOutfitInfo->InitFromXml(uiXml);
	}

	if (uiXml.NavigateToNode("inventory_items_info", 0))
	{
		UIInventoryItem = xr_new<CUIInventoryItem>();
		UIInventoryItem->InitFromXml(uiXml);
	}

	xml_init.InitAutoStaticGroup	(uiXml, "auto", 0, this);
}

void CUIItemInfo::InitItemInfo(Fvector2 pos, Fvector2 size, LPCSTR xml_name)
{
	inherited::SetWndPos	(pos);
	inherited::SetWndSize	(size);
    InitItemInfo			(xml_name);
}

bool	IsGameTypeSingle();

void CUIItemInfo::InitItem(CUICellItem* pCellItem, CInventoryItem* pCompareItem, u32 item_price, LPCSTR trade_tip)
{
	if(!pCellItem)
	{
		m_pInvItem			= NULL;
		Enable				(false);
		return;
	}

	PIItem pInvItem			= (PIItem)pCellItem->m_pData;
	m_pInvItem				= pInvItem;
	Enable					(NULL != m_pInvItem);
	if(!m_pInvItem)			return;

	Fvector2				pos;	pos.set( 0.0f, 0.0f );
	string256				str;
	if ( UIName )
	{
		UIName->SetText		(pInvItem->NameItem());
		UIName->AdjustHeightToText();
		pos.y = UIName->GetWndPos().y + UIName->GetHeight() + 4.0f;
	}
	if ( UIWeight )
	{
		LPCSTR  kg_str = CStringTable().translate( "st_kg" ).c_str();
		float	weight = pInvItem->Weight();
		
		if ( !weight )
		{
			if ( CWeaponAmmo* ammo = dynamic_cast<CWeaponAmmo*>(pInvItem) )
			{
				// its helper item, m_boxCur is zero, so recalculate via CInventoryItem::Weight()
				weight = pInvItem->CInventoryItem::Weight();
				for( u32 j = 0; j < pCellItem->ChildsCount(); ++j )
				{
					PIItem jitem	= (PIItem)pCellItem->Child(j)->m_pData;
					weight			+= jitem->CInventoryItem::Weight();
				}

			}
		}

		xr_sprintf				(str, "%3.2f %s", weight, kg_str );
		UIWeight->SetText	(str);
		
		pos.x = UIWeight->GetWndPos().x;
		if ( m_complex_desc )
		{
			UIWeight->SetWndPos	(pos);
		}
	}
	if ( UICost && IsGameTypeSingle() && item_price!=u32(-1) )
	{
		xr_sprintf				(str, "%d RU", item_price);// will be owerwritten in multiplayer
		UICost->SetText		(str);
		pos.x = UICost->GetWndPos().x;
		if ( m_complex_desc )
		{
			UICost->SetWndPos	(pos);
		}
		UICost->Show(true);
	}
	else
		UICost->Show(false);
	
//	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
//	if ( g_pGameLevel && Level().game && actor )
//	{
//		game_cl_Deathmatch* gs_mp = smart_cast<game_cl_Deathmatch*>( Game() );
//		IBuyWnd* buy_menu = gs_mp->pCurBuyMenu->GetItemPrice();
//		GetItemPrice();
//	}
	if ( UITradeTip && IsGameTypeSingle())
	{
		pos.y = UITradeTip->GetWndPos().y;
		if ( UIWeight && m_complex_desc )
		{
			pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;
		}

		if(trade_tip==NULL)
			UITradeTip->Show(false);
		else
		{
			UITradeTip->SetText(CStringTable().translate(trade_tip).c_str());
			UITradeTip->AdjustHeightToText();
			UITradeTip->SetWndPos(pos);
			UITradeTip->Show(true);
		}
	}
	
	if ( UIDesc )
	{
		pos = UIDesc->GetWndPos();
		if ( UIWeight )
			pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;

		if(UITradeTip && trade_tip!=NULL)
			pos.y = UITradeTip->GetWndPos().y + UITradeTip->GetHeight() + 4.0f;

		UIDesc->SetWndPos		(pos);
		UIDesc->Clear			();
		VERIFY					(0==UIDesc->GetSize());
		if(m_desc_info.bShowDescrText)
		{
			CUITextWnd* pItem					= xr_new<CUITextWnd>();
			pItem->SetTextColor					(m_desc_info.uDescClr);
			pItem->SetFont						(m_desc_info.pDescFont);
			pItem->SetWidth						(UIDesc->GetDesiredChildWidth());
			pItem->SetTextComplexMode			(true);
			pItem->SetText						(*pInvItem->ItemDescription());
			pItem->AdjustHeightToText			();
			UIDesc->AddWindow					(pItem, true);
		}
		TryAddConditionInfo					(*pInvItem, pCompareItem);
		TryAddWpnInfo						(*pInvItem, pCompareItem);
		TryAddArtefactInfo					(*pInvItem);
		TryAddOutfitInfo					(*pInvItem, pCompareItem);
		TryAddUpgradeInfo					(*pInvItem);
		TryAddBoosterInfo					(*pInvItem);
		TryAddItemInfo						(*pInvItem);

		if(m_b_FitToHeight)
		{
			UIDesc->SetWndSize				(Fvector2().set(UIDesc->GetWndSize().x, UIDesc->GetPadSize().y) );
			Fvector2 new_size;
			new_size.x						= GetWndSize().x;
			new_size.y						= UIDesc->GetWndPos().y+UIDesc->GetWndSize().y+20.0f;
			new_size.x						= _max(105.0f, new_size.x);
			new_size.y						= _max(105.0f, new_size.y);
			
			SetWndSize						(new_size);
			if(UIBackground)
				UIBackground->SetWndSize	(new_size);
		}

		UIDesc->ScrollToBegin				();
	}
	if(UIItemImage)
	{
		// Загружаем картинку
		UIItemImage->SetShader				(InventoryUtilities::GetEquipmentIconsShader());

		Irect item_grid_rect				= pInvItem->GetInvGridRect();
		Frect texture_rect;
		texture_rect.lt.set					(item_grid_rect.x1*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()),	item_grid_rect.y1*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
		texture_rect.rb.set					(item_grid_rect.x2*INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons()),	item_grid_rect.y2*INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons()));
		texture_rect.rb.add					(texture_rect.lt);
		UIItemImage->GetUIStaticItem().SetTextureRect(texture_rect);
		UIItemImage->TextureOn				();
		UIItemImage->SetStretchTexture		(true);

		Fvector2 v_r{};

		if (GameConstants::GetUseHQ_Icons())
		{
			v_r	= { item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons()) / 2,
				item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons()) / 2 };
		}
		else
		{
			v_r	= { item_grid_rect.x2 * INV_GRID_WIDTH2(GameConstants::GetUseHQ_Icons()),
				item_grid_rect.y2 * INV_GRID_HEIGHT2(GameConstants::GetUseHQ_Icons()) };
		}
		
		v_r.x								*= UI().get_current_kx();

		UIItemImage->GetUIStaticItem().SetSize	(v_r);
		UIItemImage->SetWidth					(v_r.x);
		UIItemImage->SetHeight					(v_r.y);
	}
}

void CUIItemInfo::TryAddConditionInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	CTorch* torch = smart_cast<CTorch*>(&pInvItem);
	CCustomDetector* artefact_detector = smart_cast<CCustomDetector*>(&pInvItem);
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(&pInvItem);

	bool ShowCharge = GameConstants::GetTorchHasBattery() || GameConstants::GetArtDetectorUseBattery() || GameConstants::GetAnoDetectorUseBattery();

	if ( pInvItem.IsUsingCondition() )
	{
		//UIConditionWnd->SetInfo( pCompareItem, pInvItem );
		//UIDesc->AddWindow( UIConditionWnd, false );
	}

	if ((torch || artefact_detector || anomaly_detector) && ShowCharge)
	{
		UIItemConditionParams->SetInfo( pCompareItem, pInvItem );
		UIDesc->AddWindow(UIItemConditionParams, false );
	}
}

void CUIItemInfo::TryAddWpnInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	if (UIWpnParams->Check(pInvItem) && GameConstants::GetShowWpnInfo())
	{
		UIWpnParams->SetInfo( pCompareItem, pInvItem );
		UIDesc->AddWindow( UIWpnParams, false );

		// Lex Addon (correct by Suhar_) 7.08.2018		(begin)
		// Необходимо расширить окно для отображения дополнительных иконок патронов
		// Получаем кол-во типов патронов, используемых оружием
		/*CWeapon* weapon = smart_cast<CWeapon*>(&pInvItem);
		xr_vector<shared_str> ammo_types;
		ammo_types = weapon->m_ammoTypes;
		int ammo_types_size = ammo_types.size();
		// Проверяем переменную высоты окна свойств оружия
		if (WpnWndSiseY == NULL)
			// Если переменная пуста, то необходимо считать её из xml и запомнить
			WpnWndSiseY = UIWpnParams->GetWndSize().y;
		// Вектор-переменная размера окна
		Fvector2 new_size;
		// Параметр ширины вектор-переменной остаётся неизменным
		new_size.x = UIWpnParams->GetWndSize().x;
		// Параметр высоты вектор-переменной меняется в зависимости от ammo_types_size
		new_size.y = WpnWndSiseY;
		// Если ammo_types_size привысил 2, то необходим дополнительный ряд
		if (ammo_types_size >= 3)
			// Увеличиваем высоту окна на 50 пикселей
			new_size.y += 50.0f;
		// Если ammo_types_size привысил 4, то необходим дополнительный ряд
		if (ammo_types_size >= 5)
			// Увеличиваем высоту окна на 50 пикселей
			new_size.y += 50.0f;
		// Устанавливаем окну свойств оружия новые размеры
		UIWpnParams->SetWndSize(new_size);
		// Корректируем размер фонового изображения
		if (UIBackground)
			UIBackground->SetWndSize(new_size);*/
		// Lex Addon (correct by Suhar_) 7.08.2018		(end)
	}
}

void CUIItemInfo::TryAddArtefactInfo(CInventoryItem& pInvItem)
{
	if (UIArtefactParams->Check(pInvItem.object().cNameSect()) && UIArtefactParams->CheckDescrInfoPortions(pInvItem.object().cNameSect()))
	{
		UIArtefactParams->SetInfo(pInvItem);
		UIDesc->AddWindow( UIArtefactParams, false );
	}
}

void CUIItemInfo::TryAddOutfitInfo( CInventoryItem& pInvItem, CInventoryItem* pCompareItem )
{
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);
	CHelmet* helmet = smart_cast<CHelmet*>(&pInvItem);
	if ( outfit && UIOutfitInfo )
	{
		CCustomOutfit* comp_outfit = smart_cast<CCustomOutfit*>(pCompareItem);
		UIOutfitItem->SetInfo( outfit, comp_outfit );
		UIDesc->AddWindow( UIOutfitInfo, false );
	}
	if ( helmet && UIOutfitInfo )
	{
		CHelmet* comp_helmet = smart_cast<CHelmet*>(pCompareItem);
		UIOutfitItem->SetInfo( helmet, comp_helmet );
		UIDesc->AddWindow( UIOutfitInfo, false );
	}

	if ((outfit || helmet) && UIOutfitItem)
	{
		UIOutfitItem->SetInfo(pInvItem);
		UIDesc->AddWindow(UIOutfitItem, false);
	}
}

void CUIItemInfo::TryAddUpgradeInfo( CInventoryItem& pInvItem )
{
	if ( pInvItem.upgardes().size() && UIProperties )
	{
		UIProperties->set_item_info( pInvItem );
		UIDesc->AddWindow( UIProperties, false );
	}
}

void CUIItemInfo::TryAddBoosterInfo(CInventoryItem& pInvItem)
{
	CEatableItem* food = smart_cast<CEatableItem*>(&pInvItem);
	if (food && UIBoosterInfo)
	{
		UIBoosterInfo->SetInfo(pInvItem);
		UIDesc->AddWindow( UIBoosterInfo, false );
	}
}

void CUIItemInfo::ResetInventoryItem()
{
	if (UIInventoryItem)
		UIInventoryItem->SetHeight(0);
}

void CUIItemInfo::TryAddItemInfo(CInventoryItem& pInvItem)
{
	CTorch* torch = smart_cast<CTorch*>(&pInvItem);
	CCustomDetector* artefact_detector = smart_cast<CCustomDetector*>(&pInvItem);
	CDetectorAnomaly* anomaly_detector = smart_cast<CDetectorAnomaly*>(&pInvItem);
	CArtefactContainer* af_container = smart_cast<CArtefactContainer*>(&pInvItem);
	CCustomBackpack* backpack = smart_cast<CCustomBackpack*>(&pInvItem);

	if ((torch || artefact_detector || anomaly_detector || af_container || backpack) && UIInventoryItem)
	{
		UIInventoryItem->SetInfo(pInvItem);
		UIDesc->AddWindow(UIInventoryItem, false);
	}
}

void CUIItemInfo::Draw()
{
	if(m_pInvItem)
		inherited::Draw();
}
