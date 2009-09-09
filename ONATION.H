//Filename    : ONATION.H
//Description : Header of class Nation
//Owner		  : Alex

#ifndef __ONATION_H
#define __ONATION_H

#ifndef __ONATIONB_H
#include <ONATIONB.H>
#endif

#ifndef __OUNIT_H
#include <OUNIT.H>
#endif

#ifndef __OTOWN_H
#include <OTOWN.H>
#endif

//--------- define parameters ----------//

#define  MAX_AI_REGION				20

#define	AI_TOWN_INIT_SIZE    	60
#define	AI_BASE_TOWN_INIT_SIZE  60
#define	AI_BASE_INIT_SIZE			20
#define  AI_MINE_INIT_SIZE			10
#define	AI_FACTORY_INIT_SIZE	 	50
#define	AI_MARKET_INIT_SIZE		50
#define	AI_INN_INIT_SIZE			10
#define	AI_CAMP_INIT_SIZE			60
#define  AI_RESEARCH_INIT_SIZE	30
#define  AI_WAR_INIT_SIZE			30
#define  AI_HARBOR_INIT_SIZE		10
#define  AI_GENERAL_INIT_SIZE	 	50
#define  AI_CARAVAN_INIT_SIZE		100
#define  AI_SHIP_INIT_SIZE			50

#define	AI_TOWN_INC_SIZE    		30
#define	AI_BASE_TOWN_INC_SIZE   30
#define	AI_BASE_INC_SIZE			10
#define  AI_MINE_INC_SIZE			10
#define	AI_FACTORY_INC_SIZE	 	10
#define	AI_MARKET_INC_SIZE		10
#define	AI_INN_INC_SIZE			10
#define	AI_CAMP_INC_SIZE			10
#define  AI_RESEARCH_INC_SIZE		10
#define  AI_WAR_INC_SIZE			10
#define  AI_HARBOR_INC_SIZE		10
#define  AI_GENERAL_INC_SIZE	 	20
#define  AI_CARAVAN_INC_SIZE		30
#define  AI_SHIP_INC_SIZE			20

//--------------------------------------//

enum	{	STD_ACTION_RETRY_COUNT = 4,		// retry this number of times before giving up
			MAX_SCORE = 600,

			MIGRATE_DEMAND_SUPPLY_DIFF=25, // suppy > demand + MIGRATE_DEMAND_SUPPLY_DIFF
			MIGRATE_STOCK_QTY=150,

			TRADE_STOCK_QTY=125,
			MAX_TRADE_MARKET = 4,

			MAX_BASE_TOWN = 10,
		};

enum  {  ACTION_DYNAMIC,     		// for ActionNode::action_type
			ACTION_FIXED		};

enum	{	ACTION_AI_BUILD_FIRM=1,		// define ActionNode action_type
			ACTION_AI_ASSIGN_OVERSEER,
			ACTION_AI_ASSIGN_CONSTRUCTION_WORKER,
			ACTION_AI_ASSIGN_WORKER,
			ACTION_AI_ASSIGN_SPY,
         ACTION_AI_SCOUT, 
			ACTION_AI_SETTLE_TO_OTHER_TOWN,
			ACTION_AI_PROCESS_TALK_MSG,
			ACTION_AI_SEA_TRAVEL,
			ACTION_AI_SEA_TRAVEL2,
			ACTION_AI_SEA_TRAVEL3,
		};

enum  {  SEA_ACTION_SETTLE=1, 			// for AI marine actions
			SEA_ACTION_BUILD_CAMP,
			SEA_ACTION_ASSIGN_TO_FIRM,
			SEA_ACTION_MOVE,
         SEA_ACTION_NONE,			// just transport them to the specific region and disemark and wait for their own actions 
		};

//--------- define AIRegion ---------//

struct AIRegion
{
	BYTE	region_id;
	char  town_count;
	char  base_town_count;
};

//-------- define ActionNode structure -------//

struct ActionNode
{
	enum { MAX_ACTION_GROUP_UNIT = 9 };

	char 	action_mode;		// eg build firm, attack, etc
	char	action_type;
	short action_para;		// parameter for the action. e.g. firmId for AI_BUILD_FIRM
	short action_para2;		// parameter for the action. e.g. firm race id. for building FirmBase
	WORD	action_id;			// an unique id. for identifying this node

	long 	add_date;			// when this action is added
	short	unit_recno;

	short	action_x_loc;		// can be firm loc, or target loc, etc
	short	action_y_loc;
	short	ref_x_loc;			// reference x loc, eg the raw material location
	short	ref_y_loc;

	char	retry_count;	 				// number of term to wait before this action is removed from the array if it cannot be processed
	char	instance_count; 				// no. of times this action needs to be carried out

	short group_unit_array[MAX_ACTION_GROUP_UNIT]; 		// for group unit actions, the no. of units in the array is stored in instance_count

	char  processing_instance_count;
	char  processed_instance_count;

	long  next_retry_date; 				// continue processing this action after this date, this is used when training a unit for construction
};

//------- Define struct AttackCamp --------//

#define MAX_SUITABLE_ATTACK_CAMP    30    // total no. of useful camps

struct AttackCamp
{
	short firm_recno;
	short combat_level;
	short distance;
	int   patrol_date;
};

//--------- Define class Nation ---------//

class  Firm;
class  Town;
class  Spy;
struct TalkMsg;

class Nation : public NationBase
{
public:
	DynArray		action_array;
	WORD			last_action_id; 	// a 16-bit id. for identifying ActionNode

public:
	Nation();
	~Nation();

	//------------------------------------------------------//
	// array used to store the the waiting and procesing actions
	//------------------------------------------------------//

	int			action_count() 			{ return action_array.size(); }
	ActionNode* get_action(int recNo)   { return (ActionNode*) action_array.get(recNo); }
	ActionNode* get_action_based_on_id(int actionId);

	//------------------------------------------------------//
	// array used to store the info. of the firms
	//------------------------------------------------------//
	short*		ai_town_array;
	short* 		ai_base_array;
	short* 		ai_mine_array;
	short*		ai_factory_array;
	short* 		ai_camp_array;
	short*		ai_research_array;
	short*		ai_war_array;
	short*		ai_harbor_array;
	short*      ai_market_array;
	short*		ai_inn_array;
	short*    	ai_general_array;
	short*		ai_caravan_array;
	short*		ai_ship_array;

	//--------------------------------------------------------//
	// parameters used to make decisions
	//--------------------------------------------------------//
	short			ai_town_size;
	short			ai_base_size;
	short			ai_mine_size;
	short			ai_factory_size;
	short			ai_camp_size;
	short			ai_research_size;
	short			ai_war_size;
	short			ai_harbor_size;
	short			ai_market_size;
	short			ai_inn_size;
	short			ai_general_size;
	short			ai_caravan_size;
	short			ai_ship_size;

	short			ai_town_count;
	short			ai_base_count;
	short			ai_mine_count;
	short			ai_factory_count;
	short			ai_camp_count;
	short			ai_research_count;
	short			ai_war_count;
	short			ai_harbor_count;
	short			ai_market_count;
	short			ai_inn_count;
	short			ai_general_count;
	short			ai_caravan_count;
	short			ai_ship_count;

	short			ai_base_town_count;

	short			firm_should_close_array[MAX_FIRM_TYPE];

	//------------------------------------------------------//
	// parameters about the nation itself
	//------------------------------------------------------//

	AIRegion		ai_region_array[MAX_AI_REGION];
	char			ai_region_count;

	//------------------------------------------------------//
	// AI personalties
	//------------------------------------------------------//

	char 			pref_force_projection;
	char			pref_military_development;		// pref_military_development + pref_economic_development = 100
	char			pref_economic_development;
	char			pref_inc_pop_by_capture;		// pref_inc_pop_by_capture + pref_inc_pop_by_growth = 100
	char			pref_inc_pop_by_growth;
	char			pref_peacefulness;
	char			pref_military_courage;
	char			pref_territorial_cohesiveness;
	char			pref_trading_tendency;
	char			pref_allying_tendency;
	char			pref_honesty;
	char			pref_town_harmony;
	char			pref_loyalty_concern;
	char			pref_forgiveness;
	char			pref_collect_tax;
	char			pref_hire_unit;
	char			pref_use_weapon;
	char			pref_keep_general;          	// whether to keep currently non-useful the general, or demote them.
	char			pref_keep_skilled_unit; 		// whether to keep currently non-useful skilled units, or assign them to towns.
	char			pref_diplomacy_retry;			// tedency to retry diplomatic actions after previous ones have been rejected.
	char			pref_attack_monster;
	char			pref_spy;
	char			pref_counter_spy;
	char			pref_food_reserve;
	char			pref_cash_reserve;
	char			pref_use_marine;
	char			pref_unit_chase_distance;
	char			pref_repair_concern;
	char			pref_scout;

	//------- AI action vars --------//

	short			ai_capture_enemy_town_recno;
	int  			ai_capture_enemy_town_plan_date;
	int  			ai_capture_enemy_town_start_attack_date;
	char			ai_capture_enemy_town_use_all_camp;

	int			ai_last_defend_action_date;

	short			ai_attack_target_x_loc;
	short			ai_attack_target_y_loc;
	short			ai_attack_target_nation_recno;		//	nation recno of the target

	AttackCamp  attack_camp_array[MAX_SUITABLE_ATTACK_CAMP];
	short			attack_camp_count;
	short			lead_attack_camp_recno;		// the firm recno of the lead attacking firm

public:
	//--------------------------------------------------------------//
	// functions to init. parameters and process ai actions
	//--------------------------------------------------------------//
	void			init(int nationType, int raceId, int colorSchemeId, DWORD playerId); // init local parameters
	void			deinit();

	void 			init_all_ai_info();
	void 			init_ai_info(short** aiInfoArray, short& aiInfoCount, short& aiInfoSize, int arrayInitSize );

	void 			deinit_all_ai_info();

	void			init_personalty();

	void			process_ai();							// entry point to start ai
	void			process_ai_main();
	void 			process_on_going_action();

	//---------------------------------------------------------------//
	// main AI thinking functions
	//---------------------------------------------------------------//
	void			think_trading();
	void			think_explore();
	int 			think_capture();

	void			ai_improve_relation();

	//---------------------------------------------------------------//
	// functions for processing waiting actions
	//---------------------------------------------------------------//
	int			ai_build_firm(ActionNode*);
	int			ai_assign_overseer(ActionNode*);
	int 			ai_assign_construction_worker(ActionNode*);
	int			ai_assign_worker(ActionNode*);
	int			ai_scout(ActionNode*);
	int			ai_settle_to_other_town(ActionNode*);
	int			ai_assign_spy(ActionNode*);

	//-----------------------------------------------------------//
	// functions used to update internal parameters
	//-----------------------------------------------------------//
	short*		update_ai_firm_array(int firmId, int actionType, int actionRecno, int& arrayCount);

	short*		update_ai_array(short& aiInfoCount, short& aiInfoSize,
						short** aiInfoArray, int arrayIncSize, int actionType, int actionRecno);

	void			add_town_info(short townRecno);	// add town information
	void			del_town_info(short townRecno);	// remove town information
	void			add_firm_info(char firmId, short firmRecno);// add firm information
	void			del_firm_info(char firmId, short firmRecno);// remove useless information as firm is removed
	void 			add_general_info(short unitRecno);
	void 			del_general_info(short unitRecno);
	void 			add_caravan_info(short unitRecno);
	void 			del_caravan_info(short unitRecno);
	void 			add_ship_info(short unitRecno);
	void 			del_ship_info(short unitRecno);
	void			assign_firm_overseer(char firm_id, short firm_recno, short overseerUnitRecno);	// add overseer information
	void			remove_firm_overseer(char firm_id, short firm_recno);	// remove overseer information
	int 			is_caravan_exist(int firstMarket, int secondMarket, int setStopInternal=0);
	void 			update_ai_region();

	AIRegion* 	get_ai_region(int regionId);
	int 			has_base_town_in_region(int regionId);

	//-----------------------------------------------------------//
	// functions for building firms
	//-----------------------------------------------------------//
	void			think_build_firm();
	int 			think_build_mine();

	int  			think_destroy_raw_site_guard();
	int 			ai_supported_inn_count();
	int 			ai_has_should_close_camp(int regionId);
	int			ai_should_build_mine();

	//-----------------------------------------------------------//
	// functions used to locate position to build firms
	//-----------------------------------------------------------//
	int			seek_mine(short& xLoc, short& yLoc, short& refXLoc, short& refYLoc);
	void			seek_best_build_mine_location(short& xLoc, short& yLoc, short mapXLoc, short mapYLoc);
	void			cal_location_score(short x1, short y1, short width, short height, int& weight);
	int 			find_best_firm_loc(short firmId, short refXLoc, short refYLoc, short& resultXLoc, short& resultYLoc);

	//------------------------------------------------------------//
	// functions for dealing with the AI action array
	//------------------------------------------------------------//
	int 			add_action(short xLoc, short yLoc, short refXLoc, short refYLoc, int actionType, int actionPara, int instanceCount=1, int unitRecno=0, int actionPara2=0, short* groupUnitArray=NULL);
	int 			add_action(ActionNode* actionNode, int immediateProcess=0);
	void			del_action(int actionRecno);
	int			is_action_exist(short actionXLoc, short actionYLoc, short refXLoc, short refYLoc, int actionType, int actionPara, int unitRecno=0, int checkMode=0);
	int			is_action_exist(int actionType, int actionPara, int regionId=0);
	int			is_build_action_exist(int firmId, int xLoc, int yLoc);

	int 			process_action(int priorityActionRecno=0, int processActionMode=0);	// waiting --> processing
	int 			process_action_id(int actionId);

	void			action_finished(WORD aiActionId, short unitRecno=0, int actionFailure=0);
	void			action_failure(WORD aiActionId, short unitRecno=0);
	void 			auto_next_action(ActionNode* actionNode);

	void 			stop_unit_action(short unitRecno);
	int			check_firm_ready(short xLoc, short yLoc, int firmId=0); // check whether firm exists and belongs to our nation
	int			check_town_ready(short xLoc, short yLoc); // check whether town exists and belongs to our nation

	//------------------------------------------------------------//
	// functions used to find skilled units
	//------------------------------------------------------------//
	Unit*			get_skilled_unit(int skillId, int raceId, ActionNode* actionNode);
	Unit*			find_skilled_unit(int skillId, int raceId, short destX, short destY, char& resultFlag, int actionId=0);
	int   		hire_unit(int skillId, int raceId, short destX, short destY);
	int			train_unit(int skillId, int raceId, short destX, short destY, int& trainTownRecno, int actionId=0);
	int   		recruit_jobless_worker(Firm* firmPtr, int preferedRaceId=0);
	int   		recruit_on_job_worker(Firm* firmPtr, int preferedRaceId=0);
	int 			ai_should_hire_unit(int considerProfit);

	//------------------------------------------------------------//
	// other functions
	//------------------------------------------------------------//

	void			settle_to_other_town();
	int 			can_ai_build(int firmId);
	int 			think_succeed_king();
	int 			closest_enemy_firm_distance(int firmId, int xLoc, int yLoc);

	//------------------------------------------------------------//
	// military related functions
	//------------------------------------------------------------//

	void			think_military();

	int			think_secret_attack();
	int 			think_attack_town();
	int			think_close_camp();

	int 			ai_attack_target(int targetXLoc, int targetYLoc, int targetCombatLevel, int defenseMode=0, int justMoveToFlag=0, int attackerMinCombatLevel=0, int attackerCampRecno=0, int useAllCamp=0);
	void 			ai_attack_target_sync();
	void 			ai_attack_target_execute(int directAttack);
	int 			ai_attack_order_nearby_mobile(int targetXLoc, int targetYLoc, int targetCombatLevel);

	int 			ai_sea_attack_target(int targetXLoc, int targetYLoc);

	void 			ai_attack_unit_in_area(int xLoc1, int yLoc1, int xLoc2, int yLoc2);
	int 			ai_defend(int attackerUnitRecno);
	int 			ai_request_military_aid();

	void			reset_ai_attack_target();

	int 			think_attack_monster();
	int 			think_monster_target(int& targetCombatLevel);

	int			ai_should_expand_military();
	int 			ai_is_troop_need_new_camp();
	int 			ai_has_too_many_camp();

	int 			ai_should_attack_friendly(int friendlyNationRecno, int attackTemptation);

	void			enable_should_attack_on_target(int targetXLoc, int targetYLoc);

	//------------------------------------------------------------//
	// economic related functions
	//------------------------------------------------------------//

	void			think_reduce_expense();
	int 			surplus_supply_rating();
	int 			ai_trade_with_rating(int withNationRecno);
	int 			ai_should_spend(int importanceRating, float spendAmt=0);
	int 			ai_should_spend_war(int enemyMilitaryRating, int considerCeaseFire=0);
	int 			ai_has_enough_food();

	//------------------------------------------------------------//
	// town related functions
	//------------------------------------------------------------//

	void			think_town();
	void			optimize_town_race();
	void 			optimize_town_race_region(int regionId);

	//--------------------------------------------------------------//
	// functions for capturing independent and enemy towns
	//--------------------------------------------------------------//

	int 			think_capture_independent();
	int 			capture_expected_resistance(int townRecno);
	int 			start_capture(int townRecno);
	int 			capture_build_camp(int townRecno, int raceId);
	int 			find_best_capturer(int townRecno, int raceId, int& bestResistanceReduce);
	int 			hire_best_capturer(int townRecno, int raceId);
	int			mobilize_capturer(int unitRecno);

	int 			think_capture_new_enemy_town(Town* capturerTown, int useAllCamp=0);
	void 			think_capturing_enemy_town();

	int 			attack_enemy_town_defense(Town* targetTown, int useAllCamp=0);
	Town* 		think_capture_enemy_town_target(Town* capturerTown);
	int 			enemy_town_combat_level(Town* targetTown, int returnIfWar, int hasWar);
	int 			enemy_firm_combat_level(Firm* targetFirm, int returnIfWar, int hasWar);
	int 			mobile_defense_combat_level(int targetXLoc, int targetYLoc, int targetNationRecno, int returnIfWar, int& hasWar);

	int 			should_use_cash_to_capture();

	//--------------------------------------------------------------//
	// marine functions
	//--------------------------------------------------------------//

	void 			think_marine();

	int			think_build_harbor_network();

	int 			think_move_between_region();
	int 			think_move_troop_between_region();
	int 			think_move_people_between_region();
	int 			think_sea_attack_enemy();

	int 			think_move_to_region_with_mine();
	int 			ai_build_camp_town_next_to(int xLoc1, int yLoc1, int xLoc2, int yLoc2);
	int 			ai_settle_to_region(int destXLoc, int destYLoc, int seaActionId);
	int 			ai_patrol_to_region(int destXLoc, int destYLoc, int seaActionId);

	int 			ai_should_sail_to_rating(int regionStatId);
	int 			ai_build_harbor(int landRegionId, int seaRegionId);

	int 			ai_sea_travel(ActionNode* actionNode);
	int 			ai_sea_travel2(ActionNode* actionNode);
	int 			ai_sea_travel3(ActionNode* actionNode);

	int 			ai_find_transport_ship(int seaRegionId, int unitXLoc, int unitYLoc, int findBest=1);
	int 			ai_build_ship(int seaRegionId, int preferXLoc, int preferYLoc, int needTransportUnit);

	int 			has_trade_ship(int firmRecno1, int firmRecno2);

	int 			ai_is_sea_travel_safe();
	int 			max_human_battle_ship_count();

	//--------------------------------------------------------------//
	// spy functions
	//--------------------------------------------------------------//

	void 			think_spy();

	int 			ai_assign_spy(int targetXLoc, int targetYLoc, int spyRaceId, int mobileOnly);
	int 			ai_assign_spy_to_town(int townRecno, int raceId=0);
	int 			ai_assign_spy_to_firm(int firmRecno);

	Spy* 			ai_find_spy(int targetXLoc, int targetYLoc, int targetRaceId, int mobileOnly);

	int 			think_assign_spy_target_camp(int raceId, int regionId);
	int 			think_assign_spy_target_town(int raceId, int regionId);
	int 			think_assign_spy_own_town(int raceId, int regionId);

	//--------------------------------------------------------------//
	// strategic grand planning functions
	//--------------------------------------------------------------//

	void 			think_grand_plan();
	int 			total_alliance_military();
	int 			total_enemy_military();
	int 			total_enemy_count();

	int 	 		think_against_mine_monopoly();
	int 	 		think_ally_against_big_enemy();
	int 	 		think_unite_against_big_enemy();

	void 			think_deal_with_all_enemy();
	void 			think_deal_with_one_enemy(int enemyNationRecno);

	int 			think_eliminate_enemy_town(int enemyNationRecno);
	int 			think_eliminate_enemy_firm(int enemyNationRecno);
	int 			think_eliminate_enemy_unit(int enemyNationRecno);

	int 			think_attack_enemy_firm(int enemyNationRecno, int firmId);
	int 			think_surrender();

	int 			ai_surrender_to_rating(int nationRecno);

	//--------------------------------------------------------------//
	// functions for responsing to diplomatic messages
	//--------------------------------------------------------------//

	void			think_diplomacy();
	int 			think_trade_treaty();
	int			think_propose_friendly_treaty();
	int			think_propose_alliance_treaty();
	int 			think_end_treaty();
	int 			think_request_cease_war();
	int 			think_request_buy_food();
	int			think_declare_war();
	int			think_give_tech();
	int			think_demand_tech();
	int			think_demand_tribute_aid();
	int 			think_give_tribute_aid(TalkMsg* rejectedMsg);
	int			think_request_surrender();

	int			ai_process_talk_msg(ActionNode* actionNode);
	void 			ai_notify_reply(int talkMsgRecno);
	int 			should_diplomacy_retry(int talkId, int nationRecno);
	void			ai_end_treaty(int nationRecno);

	int			consider_talk_msg(TalkMsg* talkMsg);
	void			notify_talk_msg(TalkMsg* talkMsg);

	int 			consider_trade_treaty(int withNationRecno);
	int 			consider_friendly_treaty(int withNationRecno);
	int 			consider_alliance_treaty(int withNationRecno);
	int 			consider_military_aid(TalkMsg* talkMsg);
	int 			consider_trade_embargo(TalkMsg* talkMsg);
	int 			consider_cease_war(int withNationRecno);
	int 			consider_declare_war(TalkMsg* talkMsg);
	int 			consider_sell_food(TalkMsg* talkMsg);
	int 			consider_take_tribute(TalkMsg* talkMsg);
	int 			consider_give_tribute(TalkMsg* talkMsg);
	int 			consider_take_aid(TalkMsg* talkMsg);
	int 			consider_give_aid(TalkMsg* talkMsg);
	int 			consider_take_tech(TalkMsg* talkMsg);
	int 			consider_give_tech(TalkMsg* talkMsg);
	int			consider_accept_surrender_request(TalkMsg* talkMsg);

	int			consider_alliance_rating(int nationRecno);
	int 			should_consider_friendly(int withNationRecno);
	int 			ai_overall_relation_rating(int withNationRecno);

	//--------- file functions -----------//

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr);
};

#ifndef __ONATIONA_H
#include <ONATIONA.H>
#endif

//-------------------------------------//

#endif
