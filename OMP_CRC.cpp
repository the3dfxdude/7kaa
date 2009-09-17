//Filename    : OMP_CRC.CPP
//Description : crc checking for multiplayer debugging
//Owner		  : Alex

#include <CRC.h>
#include <OU_GOD.h>
#include <OU_VEHI.h>
#include <OU_MONS.h>
#include <OU_CART.h>
#include <OU_MARI.h>
#include <OU_CARA.h>
#include <OFIRM.h>
#include <OF_BASE.h>
#include <OF_CAMP.h>
#include <OF_FACT.h>
#include <OF_INN.h>
#include <OF_MARK.h>
#include <OF_MINE.h>
#include <OF_RESE.h>
#include <OF_WAR.h>
#include <OF_HARB.h>
#include <OF_MONS.h>
#include <OTOWN.h>
#include <ONATIONB.h>
#include <OBULLET.h>
#include <OB_PROJ.h>
#include <OB_HOMIN.h>
#include <OB_FLAME.h>
// ###### patch begin Gilbert 20/1 #######//
#include <OREBEL.h>
#include <OSPY.h>
// ###### patch end Gilbert 20/1 #######//
#include <OTALKRES.h>


// ###### patch begin Gilbert 21/1 #####//
#define RTRIM_ARRAY(a,s) { if(s<sizeof(a)/sizeof(*a)) memset(a+s,0,sizeof(a)-s*sizeof(*a)); }
// ###### patch end Gilbert 21/1 #####//

static union
{
	char	sprite[sizeof(Sprite)];
	char	unit[sizeof(Unit)];
	char	unit_god[sizeof(UnitGod)];
	char	unit_vehicle[sizeof(UnitVehicle)];
	char	unit_monster[sizeof(UnitMonster)];
	char	unit_exp_cart[sizeof(UnitExpCart)];
	char	unit_marine[sizeof(UnitMarine)];
	char	unit_caravan[sizeof(UnitCaravan)];
	char	firm[sizeof(Firm)];
	char	firm_base[sizeof(FirmBase)];
	char	firm_camp[sizeof(FirmCamp)];
	char	firm_factory[sizeof(FirmFactory)];
	char	firm_inn[sizeof(FirmInn)];
	char	firm_market[sizeof(FirmMarket)];
	char	firm_mine[sizeof(FirmMine)];
	char	firm_research[sizeof(FirmResearch)];
	char	firm_war[sizeof(FirmWar)];
	char	firm_harbor[sizeof(FirmHarbor)];
	char	firm_monster[sizeof(FirmMonster)];
	char	town[sizeof(Town)];
	char	nation[sizeof(NationBase)];
	char	bullet[sizeof(Bullet)];
	char	projectile[sizeof(Projectile)];
	char	bullet_homing[sizeof(BulletHoming)];
	char	bullet_flame[sizeof(BulletFlame)];
	// ###### patch begin Gilbert 20/1 #######//
	char	rebel[sizeof(Rebel)];
	char	spy[sizeof(Spy)];
	// ###### patch end Gilbert 20/1 #######//
	char	talk_msg[sizeof(TalkMsg)];
} temp_obj;


//----------- End of function Sprite::crc8 -----------//
UCHAR Sprite::crc8()
{
	Sprite &dummySprite = *(Sprite *)temp_obj.sprite;
	memcpy(&dummySprite, this, sizeof(Sprite));

	dummySprite.clear_ptr();
	*((char**) &dummySprite) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummySprite, sizeof(Sprite));
	return c;
}
//----------- End of function Sprite::crc8 -----------//


//----------- End of function Sprite::clear_ptr -----------//
void Sprite::clear_ptr()
{
	sprite_info = NULL;
}
//----------- End of function Sprite::clear_ptr -----------//


//----------- End of function Unit::crc8 -----------//
UCHAR Unit::crc8()
{
	Unit &dummyUnit = *(Unit *)temp_obj.unit;
	memcpy(&dummyUnit, this, sizeof(Unit));

	dummyUnit.clear_ptr();
	*((char**) &dummyUnit) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnit, sizeof(Unit));
	return c;
}
//----------- End of function Unit::crc8 -----------//


//----------- End of function Unit::clear_ptr -----------//
void Unit::clear_ptr()
{
	Sprite::clear_ptr();

	selected_flag = 0;
	group_select_id = 0;
	
	attack_info_array = NULL;
   result_node_array = NULL;
	way_point_array	= NULL;
   team_info = NULL;

	if( !original_action_mode )
	{
		original_action_para = 0;
		original_action_x_loc = 0;
		original_action_y_loc = 0;
		original_target_x_loc = 0;
		original_target_y_loc = 0;

		ai_original_target_x_loc = 0;
		ai_original_target_y_loc = 0;
	}
}
//----------- End of function Unit::clear_ptr -----------//


//----------- End of function UnitGod::crc8 -----------//
UCHAR UnitGod::crc8()
{
	UnitGod &dummyUnitGod = *(UnitGod *)temp_obj.unit_god;
	memcpy(&dummyUnitGod, this, sizeof(UnitGod));

	dummyUnitGod.clear_ptr();
	*((char**) &dummyUnitGod) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitGod, sizeof(UnitGod));
	return c;
}
//----------- End of function UnitGod::crc8 -----------//


//----------- End of function UnitGod::clear_ptr -----------//
void UnitGod::clear_ptr()
{
	Unit::clear_ptr();

	if( !cast_power_type )
	{
		cast_origin_x = 0;
		cast_origin_y = 0;
		cast_target_x = 0;
		cast_target_y = 0;
	}

}
//----------- End of function UnitGod::clear_ptr -----------//


//----------- End of function UnitVehicle::crc8 -----------//
UCHAR UnitVehicle::crc8()
{
	UnitVehicle &dummyUnitVehicle = *(UnitVehicle *)temp_obj.unit_vehicle;
	memcpy(&dummyUnitVehicle, this, sizeof(UnitVehicle));

	dummyUnitVehicle.clear_ptr();
	*((char**) &dummyUnitVehicle) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitVehicle, sizeof(UnitVehicle));
	return c;
}
//----------- End of function UnitVehicle::crc8 -----------//


//----------- End of function UnitVehicle::clear_ptr -----------//
void UnitVehicle::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitVehicle::clear_ptr -----------//


//----------- End of function UnitMonster::crc8 -----------//
UCHAR UnitMonster::crc8()
{
	UnitMonster &dummyUnitMonster = *(UnitMonster *)temp_obj.unit_monster;
	memcpy(&dummyUnitMonster, this, sizeof(UnitMonster));

	dummyUnitMonster.clear_ptr();
	*((char**) &dummyUnitMonster) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitMonster, sizeof(UnitMonster));
	return c;
}
//----------- End of function UnitMonster::crc8 -----------//


//----------- End of function UnitMonster::clear_ptr -----------//
void UnitMonster::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitMonster::clear_ptr -----------//


//----------- End of function UnitExpCart::crc8 -----------//
UCHAR UnitExpCart::crc8()
{
	UnitExpCart &dummyUnitExpCart = *(UnitExpCart *)temp_obj.unit_exp_cart;
	memcpy(&dummyUnitExpCart, this, sizeof(UnitExpCart));

	dummyUnitExpCart.clear_ptr();
	*((char**) &dummyUnitExpCart) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitExpCart, sizeof(UnitExpCart));
	return c;
}
//----------- End of function UnitExpCart::crc8 -----------//


//----------- End of function UnitExpCart::clear_ptr -----------//
void UnitExpCart::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitExpCart::clear_ptr -----------//


//----------- End of function UnitMarine::crc8 -----------//
UCHAR UnitMarine::crc8()
{
	UnitMarine &dummyUnitMarine = *(UnitMarine *)temp_obj.unit_marine;
	memcpy(&dummyUnitMarine, this, sizeof(UnitMarine));

	dummyUnitMarine.clear_ptr();
	*((char**) &dummyUnitMarine) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitMarine, sizeof(UnitMarine));
	return c;
}
//----------- End of function UnitMarine::crc8 -----------//


//----------- End of function UnitMarine::clear_ptr -----------//
void UnitMarine::clear_ptr()
{
	memset(&splash, 0, sizeof(splash));
	
	//### begin alex 23/10 ###//
	selected_unit_id = 0;
	menu_mode = 0;
	//#### end alex 23/10 ####//

	// ###### patch begin Gilbert 21/1 ######//
	// must do this step before clear_ptr(), attack_info_array is reset there
	if( !attack_info_array )
		memset(&ship_attack_info, 0, sizeof(ship_attack_info));
	// ###### patch end Gilbert 21/1 ######//

	Unit::clear_ptr();

	RTRIM_ARRAY(unit_recno_array, unit_count);
	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( !stop_array[i].firm_recno )
		{
			memset(&stop_array[i], 0, sizeof(*stop_array));
		}
	}
}
//----------- End of function UnitMarine::clear_ptr -----------//


//----------- End of function UnitCaravan::crc8 -----------//
UCHAR UnitCaravan::crc8()
{
	UnitCaravan &dummyUnitCaravan = *(UnitCaravan *)temp_obj.unit_caravan;
	memcpy(&dummyUnitCaravan, this, sizeof(UnitCaravan));

	dummyUnitCaravan.clear_ptr();
	*((char**) &dummyUnitCaravan) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyUnitCaravan, sizeof(UnitCaravan));
	return c;
}
//----------- End of function UnitCaravan::crc8 -----------//


//----------- End of function UnitCaravan::clear_ptr -----------//
void UnitCaravan::clear_ptr()
{
	Unit::clear_ptr();

	caravan_id = 0;	// caravan_id is no longer valid

	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( !stop_array[i].firm_recno )
		{
			memset(&stop_array[i], 0, sizeof(*stop_array));
		}
	}
}
//----------- End of function UnitCaravan::clear_ptr -----------//


//----------- End of function Firm::crc8 -----------//
UCHAR Firm::crc8()
{
	Firm &dummyFirm = *(Firm *)temp_obj.firm;
	memcpy(&dummyFirm, this, sizeof(Firm));

	dummyFirm.clear_ptr();
	*((char**) &dummyFirm) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirm, sizeof(Firm));
	return c;
}
//----------- End of function Firm::crc8 -----------//


//----------- End of function Firm::clear_ptr -----------//
void Firm::clear_ptr()
{
	worker_array = NULL;
	selected_worker_id = 0;
	player_spy_count = 0;

	// clear unused element in linked_firm_array, linked_firm_enable_array
	RTRIM_ARRAY(linked_firm_array, linked_firm_count);
	RTRIM_ARRAY(linked_firm_enable_array, linked_firm_count);

	// clear unused element in linked_town_array, linked_town_enable_array
	RTRIM_ARRAY(linked_town_array, linked_town_count);
	RTRIM_ARRAY(linked_town_enable_array, linked_town_count);
}
//----------- End of function Firm::clear_ptr -----------//


//----------- End of function FirmBase::crc8 -----------//
UCHAR FirmBase::crc8()
{
	FirmBase &dummyFirmBase = *(FirmBase *)temp_obj.firm_base;
	memcpy(&dummyFirmBase, this, sizeof(FirmBase));

	dummyFirmBase.clear_ptr();
	*((char**) &dummyFirmBase) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmBase, sizeof(FirmBase));
	return c;
}
//----------- End of function FirmBase::crc8 -----------//


//----------- End of function FirmBase::clear_ptr -----------//
void FirmBase::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmBase::clear_ptr -----------//


//----------- End of function FirmCamp::crc8 -----------//
UCHAR FirmCamp::crc8()
{
	FirmCamp &dummyFirmCamp = *(FirmCamp *)temp_obj.firm_camp;
	memcpy(&dummyFirmCamp, this, sizeof(FirmCamp));

	dummyFirmCamp.clear_ptr();
	*((char**) &dummyFirmCamp) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmCamp, sizeof(FirmCamp));
	return c;
}
//----------- End of function FirmCamp::crc8 -----------//


//----------- End of function FirmCamp::clear_ptr -----------//
void FirmCamp::clear_ptr()
{
	Firm::clear_ptr();

	// clear unused element in defense_array
	for( int i = 0; i < sizeof(defense_array)/sizeof(*defense_array); ++i )
	{
		if( !defense_array[i].unit_recno )
		{
			memset(&defense_array[i], 0, sizeof(*defense_array));
		}
	}

	// clear unused element in patrol_unit_array
	RTRIM_ARRAY(patrol_unit_array, patrol_unit_count);
	
	// clear unused element in coming_unit_array
	RTRIM_ARRAY(coming_unit_array, coming_unit_count);
}
//----------- End of function FirmCamp::clear_ptr -----------//


//----------- End of function FirmFactory::crc8 -----------//
UCHAR FirmFactory::crc8()
{
	FirmFactory &dummyFirmFactory = *(FirmFactory *)temp_obj.firm_factory;
	memcpy(&dummyFirmFactory, this, sizeof(FirmFactory));

	dummyFirmFactory.clear_ptr();
	*((char**) &dummyFirmFactory) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmFactory, sizeof(FirmFactory));
	return c;
}
//----------- End of function FirmFactory::crc8 -----------//


//----------- End of function FirmFactory::clear_ptr -----------//
void FirmFactory::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmFactory::clear_ptr -----------//


//----------- End of function FirmInn::crc8 -----------//
UCHAR FirmInn::crc8()
{
	FirmInn &dummyFirmInn = *(FirmInn *)temp_obj.firm_inn;
	memcpy(&dummyFirmInn, this, sizeof(FirmInn));

	dummyFirmInn.clear_ptr();
	*((char**) &dummyFirmInn) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmInn, sizeof(FirmInn));
	return c;
}
//----------- End of function FirmInn::crc8 -----------//


//----------- End of function FirmInn::clear_ptr -----------//
void FirmInn::clear_ptr()
{
	Firm::clear_ptr();

	// clear unused element in inn_unit_array
	RTRIM_ARRAY(inn_unit_array, inn_unit_count);
}
//----------- End of function FirmInn::clear_ptr -----------//


//----------- End of function FirmMarket::crc8 -----------//
UCHAR FirmMarket::crc8()
{
	FirmMarket &dummyFirmMarket = *(FirmMarket *)temp_obj.firm_market;
	memcpy(&dummyFirmMarket, this, sizeof(FirmMarket));

	dummyFirmMarket.clear_ptr();
	*((char**) &dummyFirmMarket) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmMarket, sizeof(FirmMarket));
	return c;
}
//----------- End of function FirmMarket::crc8 -----------//


//----------- End of function FirmMarket::clear_ptr -----------//
void FirmMarket::clear_ptr()
{
	int i;
	for(i=0; i<MAX_RAW; ++i)
		market_raw_array[i] = NULL;

	for(i=0; i<MAX_PRODUCT; ++i)
		market_product_array[i] = NULL;

	// clear unused element in defense_array
	for( i = 0; i < sizeof(market_goods_array)/sizeof(*market_goods_array); ++i )
	{
		if( !market_goods_array[i].raw_id && !market_goods_array[i].product_raw_id )
		{
			memset(&market_goods_array[i], 0, sizeof(*market_goods_array));
		}
	}

	Firm::clear_ptr();
}
//----------- End of function FirmMarket::clear_ptr -----------//


//----------- End of function FirmMine::crc8 -----------//
UCHAR FirmMine::crc8()
{
	FirmMine &dummyFirmMine = *(FirmMine *)temp_obj.firm_mine;
	memcpy(&dummyFirmMine, this, sizeof(FirmMine));

	dummyFirmMine.clear_ptr();
	*((char**) &dummyFirmMine) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmMine, sizeof(FirmMine));
	return c;
}
//----------- End of function FirmMine::crc8 -----------//


//----------- End of function FirmMine::clear_ptr -----------//
void FirmMine::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmMine::clear_ptr -----------//


//----------- End of function FirmResearch::crc8 -----------//
UCHAR FirmResearch::crc8()
{
	FirmResearch &dummyFirmResearch = *(FirmResearch *)temp_obj.firm_research;
	memcpy(&dummyFirmResearch, this, sizeof(FirmResearch));

	dummyFirmResearch.clear_ptr();
	*((char**) &dummyFirmResearch) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmResearch, sizeof(FirmResearch));
	return c;
}
//----------- End of function FirmResearch::crc8 -----------//


//----------- End of function FirmResearch::clear_ptr -----------//
void FirmResearch::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmResearch::clear_ptr -----------//


//----------- End of function FirmWar::crc8 -----------//
UCHAR FirmWar::crc8()
{
	FirmWar &dummyFirmWar = *(FirmWar *)temp_obj.firm_war;
	memcpy(&dummyFirmWar, this, sizeof(FirmWar));

	dummyFirmWar.clear_ptr();
	*((char**) &dummyFirmWar) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmWar, sizeof(FirmWar));
	return c;
}
//----------- End of function FirmWar::crc8 -----------//


//----------- End of function FirmWar::clear_ptr -----------//
void FirmWar::clear_ptr()
{
	Firm::clear_ptr();

	// if nothing building clear some variables
	if( !build_unit_id )
	{
		last_process_build_frame_no = 0;
		build_progress_days = (float)0.0;
	}

	// clear unused build_queue_array
	RTRIM_ARRAY(build_queue_array, build_queue_count);
}
//----------- End of function FirmWar::clear_ptr -----------//


//----------- End of function FirmHarbor::crc8 -----------//
UCHAR FirmHarbor::crc8()
{
	FirmHarbor &dummyFirmHarbor = *(FirmHarbor *)temp_obj.firm_harbor;
	memcpy(&dummyFirmHarbor, this, sizeof(FirmHarbor));

	dummyFirmHarbor.clear_ptr();
	*((char**) &dummyFirmHarbor) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmHarbor, sizeof(FirmHarbor));
	return c;
}
//----------- End of function FirmHarbor::crc8 -----------//


//----------- End of function FirmHarbor::clear_ptr -----------//
void FirmHarbor::clear_ptr()
{
	Firm::clear_ptr();

	if(!build_unit_id)
		start_build_frame_no = 0;

	RTRIM_ARRAY(ship_recno_array, ship_count);
	RTRIM_ARRAY(build_queue_array, build_queue_count);
	RTRIM_ARRAY(linked_mine_array, linked_mine_num);
	RTRIM_ARRAY(linked_factory_array, linked_factory_num);
	RTRIM_ARRAY(linked_market_array, linked_market_num);
}
//----------- End of function FirmHarbor::clear_ptr -----------//


//----------- End of function FirmMonster::crc8 -----------//
UCHAR FirmMonster::crc8()
{
	FirmMonster &dummyFirmMonster = *(FirmMonster *)temp_obj.firm_monster;
	memcpy(&dummyFirmMonster, this, sizeof(FirmMonster));

	dummyFirmMonster.clear_ptr();
	*((char**) &dummyFirmMonster) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyFirmMonster, sizeof(FirmMonster));
	return c;
}
//----------- End of function FirmMonster::crc8 -----------//


//----------- End of function FirmMonster::clear_ptr -----------//
void FirmMonster::clear_ptr()
{
	Firm::clear_ptr();

	if( !monster_king.monster_id )
	{
		monster_king.unit_id = 0;
		monster_king.mobile_unit_recno = 0;
		monster_king.combat_level = 0;
		monster_king.hit_points = 0;
		monster_king.max_hit_points = 0;
	}

	RTRIM_ARRAY(monster_general_array, monster_general_count);
	RTRIM_ARRAY(waiting_soldier_array, waiting_soldier_count);
	RTRIM_ARRAY(patrol_unit_array, patrol_unit_count);
}
//----------- End of function FirmMonster::clear_ptr -----------//


//----------- End of function Town::crc8 -----------//
UCHAR Town::crc8()
{
	Town &dummyTown = *(Town *)temp_obj.town;
	memcpy(&dummyTown, this, sizeof(Town));

	dummyTown.clear_ptr();
	// to clear virtual table pointer, possibly in the future
	if( (void *)&dummyTown != (void *)&dummyTown.town_recno )
		*((char**) &dummyTown) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyTown, sizeof(Town));
	return c;
}
//----------- End of function Town::crc8 -----------//


//----------- End of function Town::clear_ptr -----------//
void Town::clear_ptr()
{
	int layoutCount = town_res.get_layout(layout_id)->slot_count;
	RTRIM_ARRAY(slot_object_id_array, layoutCount);

	RTRIM_ARRAY(train_queue_skill_array, train_queue_count);
	RTRIM_ARRAY(train_queue_race_array, train_queue_count);

	RTRIM_ARRAY(linked_firm_array, linked_firm_count);
	RTRIM_ARRAY(linked_firm_enable_array, linked_firm_count);

	RTRIM_ARRAY(linked_town_array, linked_town_count);
	RTRIM_ARRAY(linked_town_enable_array, linked_town_count);
}
//----------- End of function Town::clear_ptr -----------//


//----------- End of function NationBase::crc8 -----------//
UCHAR NationBase::crc8()
{
	NationBase &dummyNationBase = *(NationBase *)temp_obj.nation;
	memcpy(&dummyNationBase, this, sizeof(NationBase));

	dummyNationBase.clear_ptr();
	*((char**) &dummyNationBase) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyNationBase, sizeof(NationBase));
	return c;
}
//----------- End of function NationBase::crc8 -----------//


//----------- End of function NationBase::clear_ptr -----------//
void NationBase::clear_ptr()
{
	nation_type = 0;
	memset(nation_name_str, 0, sizeof(nation_name_str) );    // garbage may exist after the '\0'
	next_frame_ready = 0;
	is_allied_with_player = 0;

	// ignore contact_msg_flag in relation_array
	for(short nationRecno=1; nationRecno <= MAX_NATION; ++nationRecno)
	{
		get_relation(nationRecno)->contact_msg_flag = 0;
	}
}
//----------- End of function NationBase::clear_ptr -----------//


//----------- End of function Bullet::crc8 -----------//
UCHAR Bullet::crc8()
{
	Bullet &dummyBullet = *(Bullet *)temp_obj.bullet;
	memcpy(&dummyBullet, this, sizeof(Bullet));

	dummyBullet.clear_ptr();
	*((char**) &dummyBullet) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyBullet, sizeof(Bullet));
	return c;
}
//----------- End of function Bullet::crc8 -----------//


//----------- End of function Bullet::clear_ptr -----------//
void Bullet::clear_ptr()
{
	Sprite::clear_ptr();
}
//----------- End of function Bullet::clear_ptr -----------//


//----------- End of function Projectile::crc8 -----------//
UCHAR Projectile::crc8()
{
	Projectile &dummyProjectile = *(Projectile *)temp_obj.projectile;
	memcpy(&dummyProjectile, this, sizeof(Projectile));

	dummyProjectile.clear_ptr();
	*((char**) &dummyProjectile) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyProjectile, sizeof(Projectile));
	return c;
}
//----------- End of function Projectile::crc8 -----------//


//----------- End of function Projectile::clear_ptr -----------//
void Projectile::clear_ptr()
{
	memset(&act_bullet, 0, sizeof(act_bullet));
	memset(&bullet_shadow, 0, sizeof(bullet_shadow));
	
	Bullet::clear_ptr();
}
//----------- End of function Projectile::clear_ptr -----------//


//----------- End of function BulletHoming::crc8 -----------//
UCHAR BulletHoming::crc8()
{
	BulletHoming &dummyBulletHoming = *(BulletHoming *)temp_obj.bullet_homing;
	memcpy(&dummyBulletHoming, this, sizeof(BulletHoming));

	dummyBulletHoming.clear_ptr();
	*((char**) &dummyBulletHoming) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyBulletHoming, sizeof(BulletHoming));
	return c;
}
//----------- End of function BulletHoming::crc8 -----------//


//----------- End of function BulletHoming::clear_ptr -----------//
void BulletHoming::clear_ptr()
{
	Bullet::clear_ptr();
}
//----------- End of function BulletHoming::clear_ptr -----------//


//----------- End of function BulletFlame::crc8 -----------//
UCHAR BulletFlame::crc8()
{
	BulletFlame &dummyBulletFlame = *(BulletFlame *)temp_obj.bullet_flame;
	memcpy(&dummyBulletFlame, this, sizeof(BulletFlame));

	dummyBulletFlame.clear_ptr();
	*((char**) &dummyBulletFlame) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyBulletFlame, sizeof(BulletFlame));
	return c;
}
//----------- End of function BulletFlame::crc8 -----------//


//----------- Begin of function BulletFlame::clear_ptr -----------//
void BulletFlame::clear_ptr()
{
	Bullet::clear_ptr();
}
//----------- End of function BulletFlame::clear_ptr -----------//


// ###### patch begin Gilbert 20/1 #######//
//----------- Begin of function Rebel::crc8 -----------//
UCHAR Rebel::crc8()
{
	Rebel &dummyRebel = *(Rebel *)temp_obj.rebel;
	memcpy(&dummyRebel, this, sizeof(Rebel));

	dummyRebel.clear_ptr();

	UCHAR c = ::crc8((UCHAR *)&dummyRebel, sizeof(Rebel));
	return c;
}
//----------- End of function Rebel::crc8 -----------//


//----------- Begin of function Rebel::clear_ptr -----------//
void Rebel::clear_ptr()
{
}
//----------- End of function Rebel::clear_ptr -----------//


//----------- Begin of function Spy::crc8 -----------//
UCHAR Spy::crc8()
{
	Spy &dummySpy = *(Spy *)temp_obj.spy;
	memcpy(&dummySpy, this, sizeof(Spy));

	dummySpy.clear_ptr();

	UCHAR c = ::crc8((UCHAR *)&dummySpy, sizeof(Spy));
	return c;
}
//----------- End of function Spy::crc8 -----------//


//----------- Begin of function Spy::clear_ptr -----------//
void Spy::clear_ptr()
{
}
//----------- End of function Spy::clear_ptr -----------//
// ###### patch end Gilbert 20/1 #######//

//----------- Begin of function TalkMsg::crc8 -----------//
UCHAR TalkMsg::crc8()
{
	TalkMsg &dummyTalkMsg = *(TalkMsg *)temp_obj.talk_msg;
	memcpy(&dummyTalkMsg, this, sizeof(TalkMsg));

	dummyTalkMsg.clear_ptr();
	// *((char**) &dummyTalkMsg) = NULL;

	UCHAR c = ::crc8((UCHAR*)&dummyTalkMsg, sizeof(TalkMsg));
	return c;
}
//----------- End of function TalkMsg::crc8 -----------//

//----------- Begin of function TalkMsg::clear_ptr -----------//
void TalkMsg::clear_ptr()
{
}
//----------- End of function TalkMsg::clear_ptr -----------//
