/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WOWSERVER_QUEST_H
#define WOWSERVER_QUEST_H

//safe way to handle arrays in debug mode
#include <array>

using namespace std;
/*
2.3.0 research
not available because low level = 1
available but quest low level = 2
question mark = 3 (not got objectives)
blue question mark = 4
blue exclamation mark = 5
yellow exclamation mark = 6
yellow question mark = 7
finished = 8
132 error = 9
*/

enum QUEST_STATUS
{
	QMGR_QUEST_NOT_AVAILABLE		= 0x00,	// There aren't quests avaiable.				| "No Mark"
	QMGR_QUEST_AVAILABLELOW_LEVEL	= 0x01,	// Quest avaiable, and your level isnt enough.	| "Gray Quotation Mark !"
	QMGR_QUEST_CHAT					= 0x02,	// Quest avaiable it shows a talk baloon.		| "No Mark"
	QMGR_QUEST_NOT_FINISHED			= 0x05,	// Quest isnt finished yet.						| "Gray Question ? Mark"
	QMGR_QUEST_REPEATABLE_FINISHED	= 0x06,
	QMGR_QUEST_REPEATABLE			= 0x07,	// Quest repeatable								| "Blue Question ? Mark" 
	QMGR_QUEST_AVAILABLE			= 0x08,	// Quest avaiable, and your level is enough		| "Yellow Quotation ! Mark" 
	QMGR_QUEST_FINISHED_2			= 0x09,  // Quest has been finished                      | "No icon on the minimap"
	QMGR_QUEST_FINISHED				= 0x0A,	// Quest has been finished.						| "Yellow Question  ? Mark" (7 has no minimap icon)
	//QUEST_ITEM_UPDATE				= 0x06	 // Yellow Question "?" Mark. //Unknown
};

enum QuestStatus		// dupe for scriptdev2
{
    QUEST_STATUS_NONE           = 0,
    QUEST_STATUS_COMPLETE       = 1,
    QUEST_STATUS_UNAVAILABLE    = 2,
    QUEST_STATUS_INCOMPLETE     = 3,
    QUEST_STATUS_AVAILABLE      = 4,
    MAX_QUEST_STATUS
};



enum QUESTGIVER_QUEST_TYPE
{
	QUESTGIVER_QUEST_START  = 0x01,
	QUESTGIVER_QUEST_END	= 0x02,
};

enum QUEST_TYPE
{
	QUEST_GATHER	= 0x01,
	QUEST_SLAY	  = 0x02,
};

enum QUEST_FLAG
{
	QUEST_FLAG_NONE					= 0x00000000,
	QUEST_FLAG_DELIVER				= 0x00000001,   
	QUEST_FLAG_KILL					= 0x00000002,   
	QUEST_FLAG_SPEAKTO				= 0x00000004,
	QUEST_FLAG_REPEATABLE			= 0x00000008,
	QUEST_FLAG_EXPLORATION			= 0x00000010,
	QUEST_FLAG_TIMED				= 0x00000020,
	QUEST_FLAG_REPUTATION			= 0x00000080,
    QUEST_FLAGS_FLAGS_PVP			= 0x00002000,				// activates PvP on accept
    QUEST_FLAGS_UNK4				= 0x00004000,                // ? Membership Card Renewal
    QUEST_FLAGS_WEEKLY				= 0x00008000,                // Not used currently: Weekly quests
    QUEST_FLAGS_AUTOCOMPLETE		= 0x00010000,                // auto complete
    QUEST_FLAGS_UNK5				= 0x00020000,                // has something to do with ReqItemId and SrcItemId
    QUEST_FLAGS_UNK6				= 0x00040000,                // use Objective text as Complete text
    QUEST_FLAGS_AUTO_ACCEPT			= 0x00080000,                // quests in starting areas

	// Custom flags
	QUEST_FLAG_ONLY_ONE_REQUIRED	= 0x00010000,
};

enum FAILED_REASON
{
	FAILED_REASON_FAILED			= 0,
	FAILED_REASON_INV_FULL			= 4,
    FAILED_REASON_DUPE_ITEM_FOUND   = 17,
};

enum INVALID_REASON
{
	INVALID_REASON_DONT_HAVE_REQ			= 0,
	INVALID_REASON_DONT_HAVE_LEVEL			= 1,
	INVALID_REASON_DONT_HAVE_RACE			= 6,
	INVALID_REASON_COMPLETED_QUEST			= 7,
	INVALID_REASON_HAVE_TIMED_QUEST			= 12,
	INVALID_REASON_HAVE_QUEST				= 13,
//	INVALID_REASON_DONT_HAVE_REQ_ITEMS	  = 0x13,
//	INVALID_REASON_DONT_HAVE_REQ_MONEY	  = 0x15,
	INVALID_REASON_DONT_HAVE_EXP_ACCOUNT	= 16,
	INVALID_REASON_DONT_HAVE_REQ_ITEMS		= 21, //changed for 2.1.3
	INVALID_REASON_DONT_HAVE_REQ_MONEY		= 23,
	INVALID_REASON_REACHED_DAILY_LIMIT		= 26, //"you have completed xx daily quests today" confirmd :)
	INVALID_REASON_UNKNOW27					= 27,//"You cannot completed quests once you have reached tired time"
};

enum QUEST_SHARE
{
	QUEST_SHARE_MSG_SHARING_QUEST   = 0,
	QUEST_SHARE_MSG_CANT_TAKE_QUEST = 1,
	QUEST_SHARE_MSG_ACCEPT_QUEST	= 2,
	QUEST_SHARE_MSG_REFUSE_QUEST	= 3,
	QUEST_SHARE_MSG_TOO_FAR		 = 4,
	QUEST_SHARE_MSG_BUSY			= 5,
	QUEST_SHARE_MSG_LOG_FULL		= 6,
	QUEST_SHARE_MSG_HAVE_QUEST	  = 7,
	QUEST_SHARE_MSG_FINISH_QUEST	= 8,
	QUEST_SHARE_MSG_QUEST_TIMER_FINISHED	= 9,
	QUEST_SHARE_MSG_NOT_IN_PARTY			= 10,
};

#define arcemu_QUEST_REPEATABLE 1
#define arcemu_QUEST_REPEATABLE_DAILY 2
#define arcemu_QUEST_REPEATABLE_WEEKLY 3

class QuestScript;
#pragma pack(push,1)
struct Quest
{
	int32 id;
	int32 IsActiveFlags;	// 0, 1 or 2. If value = 0 then Quest is autocompleted (skip objectives/details). If value = 1 then Quest is disabled (not yet implemented in the core) If value = 2 them Quest is enabled and not autocomplete
	int32 max_level;
	int32 min_level;
	int32 zone_id;
	int32 type;
	int32 suggestedplayers;
	int32 objective_rep_faction;
	int32 objective_rep_value;
	int32 objective_rep_faction2;
	int32 objective_rep_value2;
	int32 next_quest_id;
	int32 rew_xp_rank; // XP Reward level
	int32 reward_money;
	int32 rew_money_at_max_level;
	int32 reward_spell;
	int32 effect_on_player;
	int32 bonushonor;
	float unk_33_2;
	int32 srcitem;
	int32 quest_flags;
	int32 rewardtitleid;
	int32 req_playerkillcount;
	int32 rewardtalents;
	int32 unk_33_3;
	int32 unk_33_4;
	int32 reward_item[4];
	int32 reward_itemcount[4];
	int32 reward_choiceitem[6];
	int32 reward_choiceitemcount[6];
	int32 rew_faction[5];			
	int32 rew_faction_repA[5];	
	int32 rew_faction_repB[5];
	int32 point_mapid;
	float point_x;
	float point_y;
	int32 point_opt;
	char * title;
	char * details;
	char * objectives;
	char * endtext;
	char * unk_33_6;
	int32 required_mob[4];
	int32 required_mobcount[4];
	int32 linkItemId[4];
	int32 linkItemIdCount[4];
	int32 required_item[6];
	int32 required_itemcount[6];
	char * objectivetexts[4];

	// Custom fields start here
	uint32 previous_quest_id;
	uint32 required_races;
	uint32 required_class;
	uint32 required_tradeskill;
	uint32 required_tradeskill_value;

	uint32 required_min_rep_faction;
	uint32 required_min_rep_value;
	uint32 required_max_rep_faction;
	uint32 required_max_rep_value;

	uint32 required_quests[4];
	uint32 receive_items[4];
	uint32 receive_itemcount[4];
	uint32 time;
	int32 required_spell[4];
	uint32 required_triggers[4];
	uint32 special_flags;
	char * completiontext;
	char * incompletetext;
	int32 is_repeatable;
	
	int32 count_required_mob;
	int32 count_requiredquests;
	int32 count_requiredtriggers;
	int32 count_receiveitems;
	int32 count_reward_choiceitem;
	int32 count_required_item;
	int32 required_mobtype[4];
	int32 count_reward_item;
	QuestScript* pQuestScript;
};
#pragma pack(pop)

enum QUEST_MOB_TYPES
{
	QUEST_MOB_TYPE_CREATURE = 1,
	QUEST_MOB_TYPE_GAMEOBJECT = 2,
};

#define MAX_QUEST_LOG_SIZE 25
#define QUEST_LOG_SIZE (PLAYER_QUEST_LOG_2_1-PLAYER_QUEST_LOG_1_1)

class QuestScript;
#define CALL_QUESTSCRIPT_EVENT(obj, func) if(static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript != NULL) static_cast<QuestLogEntry*>(obj)->GetQuest()->pQuestScript->func

class SERVER_DECL QuestLogEntry : public EventableObject
{
	friend class QuestMgr;

public:

	QuestLogEntry();
	~QuestLogEntry();

	ARCEMU_INLINE Quest* GetQuest() { return m_quest; };
	void Init(Quest* quest, Player* plr, uint32 slot);

	bool CanBeFinished();
	void HackComplete();		//just so completed state could be loaded from DB 
	void SubtractTime(uint32 value);
	void SaveToDB(QueryBuffer * buf);
	bool LoadFromDB(Field *fields);
	void UpdatePlayerFields();

	void SetTrigger(uint32 i);
	void SetMobCount(uint32 i, uint32 count);

	bool IsUnitAffected(Unit* target);
	ARCEMU_INLINE bool IsCastQuest() { return iscastquest;}
	void AddAffectedUnit(Unit* target);
	void ClearAffectedUnits();

	void SetSlot(int32 i);
	ARCEMU_INLINE int32 GetSlot() { return m_slot; }
	void Finish();

	void SendQuestComplete();
	void SendUpdateAddKill(uint32 i);
	ARCEMU_INLINE uint32 GetMobCount(uint32 i) { return m_mobcount[i]; }
	ARCEMU_INLINE uint32 GetExploredAreas(uint32 i) { return m_explored_areas[i]; }

	ARCEMU_INLINE uint32 GetBaseField(uint32 slot)
	{
		return PLAYER_QUEST_LOG_1_1 + (slot * QUEST_LOG_SIZE);
	}

private:
	uint32 completed;

	bool mInitialized;
	bool mDirty;

	Quest *m_quest;
	Player *m_plr;
	
	//uint32 m_mobcount[4];
	tr1::array< uint32, 4> m_mobcount;
	//uint32 m_explored_areas[4];
	tr1::array< uint32, 4> m_explored_areas;

	std::set<uint64> m_affected_units;
	bool iscastquest;

	uint32 m_time_left;
	int32 m_slot;
};

#endif
