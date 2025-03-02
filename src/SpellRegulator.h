#pragma once
#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "ScriptMgr.h"

class SpellRegulator
{
public:
	static SpellRegulator* instance()
	{
		static SpellRegulator instance;
		return &instance;
	}

	void Regulate(uint32& damage, uint32 spellId)
	{
		if (RegulatorContainer.find(spellId) == RegulatorContainer.end())
			return;

		float val = RegulatorContainer[spellId];

		if (!val || val == 100.0f)
			return;

		damage = (damage / 100.0f) * val;
	}

	void LoadFromDB()
	{
		RegulatorContainer.clear();
		uint32 msTime = getMSTime();
		QueryResult result = WorldDatabase.Query("SELECT * FROM spellregulator");

		if (!result)
			return;

		uint32 count = 0;
		do
        {
			Field* fields = result->Fetch();
			RegulatorContainer[fields[0].Get<uint32>()] = fields[1].Get<float>();
			++count;
		} while (result->NextRow());
		LOG_INFO("server.loading", "Loaded {} regulated spells in {} ms", count, GetMSTimeDiffToNow(msTime));
	}

private:
	std::unordered_map<uint32, float> RegulatorContainer; // spellid, percentage
};

#define sSpellRegulator SpellRegulator::instance()

class RegulatorLoader : public WorldScript
{
public:
	RegulatorLoader() : WorldScript("SpellRegulatorLoader", {
        WORLDHOOK_ON_STARTUP
    }) {}

	void OnStartup() override
	{
		sSpellRegulator->LoadFromDB();
	}
};
