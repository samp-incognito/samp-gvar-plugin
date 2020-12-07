/*
 * Copyright (C) 2014 Incognito
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <algorithm>
#include <queue>
#include <string>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include"sampgdk.h" 

#define PLUGIN_VERSION "1.4"

#define CHECK_PARAMS(m, n) \
	if (params[0] != (m * 4)) \
	{ \
		sampgdk::logprintf("*** %s: Expecting %d parameter(s), but found %d", n, m, params[0] / 4); \
		return 0; \
	}

typedef boost::unordered_map<std::string, boost::tuple<int, boost::variant<int, std::string, float> > > DataMap;
typedef boost::unordered_map<int, boost::tuple<int, std::queue<int> > > IndexMap;
typedef boost::unordered_map<int, DataMap> MainMap;

MainMap mainMap;
DataMap dataMap;
IndexMap indexMap;

typedef void (*logprintf_t)(const char*, ...);

logprintf_t logprintf;

extern void* pAMXFunctions;

int gIndex = 0;

std::string getString(AMX* amx, cell param, bool toLower)
{
	std::string name;
	char* string = NULL;
	amx_StrParam(amx, param, string);
	if (string)
	{
		name = string;
		if (toLower)
		{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		}
	}
	return name;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return sampgdk::Supports() | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("\n\n*** GVar Plugin v%s by Incognito loaded ***\n", PLUGIN_VERSION);
	return sampgdk::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	logprintf("\n\n*** GVar Plugin v%s by Incognito unloaded ***\n", PLUGIN_VERSION);
	sampgdk::Unload();
}

static cell AMX_NATIVE_CALL n_GVarSetInt(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "GVarSetInt");
	std::string name = getString(amx, params[1], true);
	int value = static_cast<int>(params[2]);
	DataMap::iterator i = dataMap.find(name);
	if (i != dataMap.end())
	{
		i->second = boost::make_tuple(i->second.get<0>(), value);
		return 1;
	}
	gIndex++;
	dataMap[name] = boost::make_tuple(gIndex, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_GVarGetInt(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "GVarGetInt");
	std::string name = getString(amx, params[1], true);
	DataMap::iterator j = dataMap.find(name);
	if (j != dataMap.end())
	{
		if (j->second.get<1>().type() == typeid(int))
		{
			int value = boost::get<int>(j->second.get<1>());
			return static_cast<cell>(value);
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GVarSetString(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "GVarSetString");
	std::string name = getString(amx, params[1], true), value = getString(amx, params[2], false);
	DataMap::iterator i = dataMap.find(name);
	if (i != dataMap.end())
	{
		i->second = boost::make_tuple(i->second.get<0>(), value);
		return 1;
	}
	gIndex++;
	dataMap[name] = boost::make_tuple(gIndex, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_GVarGetString(AMX* amx, cell* params)
{
	CHECK_PARAMS(3, "GVarGetString");
	std::string name = getString(amx, params[1], true);
	int size = static_cast<int>(params[3]);
	DataMap::iterator j = dataMap.find(name);
	if (j != dataMap.end())
	{
		if (j->second.get<1>().type() == typeid(std::string))
		{
			cell* dest = NULL;
			amx_GetAddr(amx, params[2], &dest);
			amx_SetString(dest, boost::get<std::string>(j->second.get<1>()).c_str(), 0, 0, size);
			return 1;
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GVarSetFloat(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "GVarSetFloat");
	std::string name = getString(amx, params[1], true);
	float value = amx_ctof(params[2]);
	DataMap::iterator i = dataMap.find(name);
	if (i != dataMap.end())
	{
		i->second = boost::make_tuple(i->second.get<0>(), value);
		return 1;
	}
	gIndex++;
	dataMap[name] = boost::make_tuple(gIndex, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_GVarGetFloat(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "GVarGetFloat");
	std::string name = getString(amx, params[1], true);
	int value = static_cast<int>(params[2]);
	DataMap::iterator j = dataMap.find(name);
	if (j != dataMap.end())
	{
		if (j->second.get<1>().type() == typeid(float))
		{
			float value = boost::get<float>(j->second.get<1>());
			return amx_ftoc(value);
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GVarDelete(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "GVarDelete");
	std::string name = getString(amx, params[1], true);
	DataMap::iterator j = dataMap.find(name);
	if (j != dataMap.end())
	{
		dataMap.erase(j);
		return 1;
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GVarsGetUpperIndex(AMX* amx, cell* params)
{
	CHECK_PARAMS(0, "GVarsGetUpperIndex");
	return static_cast<cell>(gIndex);
}

static cell AMX_NATIVE_CALL n_GVarGetNameAtIndex(AMX* amx, cell* params)
{
	CHECK_PARAMS(3, "GVarGetNameAtIndex");
	int index = static_cast<int>(params[1]), size = static_cast<int>(params[3]);
	DataMap::iterator j;
	for (j = dataMap.begin(); j != dataMap.end(); ++j)
	{
		if (j->second.get<0>() == index)
		{
			cell* dest = NULL;
			amx_GetAddr(amx, params[2], &dest);
			amx_SetString(dest, j->first.c_str(), 0, 0, size);
			return 1;
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GVarGetType(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "GVarGetType");
	std::string name = getString(amx, params[1], true);
	DataMap::iterator j = dataMap.find(name);
	if (j != dataMap.end())
	{
		if (j->second.get<1>().type() == typeid(int))
		{
			return static_cast<cell>(SERVER_VARTYPE_INT);
		}
		if (j->second.get<1>().type() == typeid(std::string))
		{
			return static_cast<cell>(SERVER_VARTYPE_STRING);
		}
		if (j->second.get<1>().type() == typeid(float))
		{
			return static_cast<cell>(SERVER_VARTYPE_FLOAT);
		}
	}
	return static_cast<cell>(SERVER_VARTYPE_NONE);
}

//=====================================PVars====================================

static cell AMX_NATIVE_CALL n_PVarSetInt(AMX* amx, cell* params)
{
	CHECK_PARAMS(3, "PVarSetInt");
	std::string name = getString(amx, params[2], true);
	int value = static_cast<int>(params[3]), index = 0;
	int id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(id);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[id] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[id][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetInt(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "PVarGetInt");
	std::string name = getString(amx, params[2], true);
	int id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(int))
			{
				int value = boost::get<int>(j->second.get<1>());
				return static_cast<cell>(value);
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarSetString(AMX* amx, cell* params)
{
	CHECK_PARAMS(3, "PVarSetString");
	std::string name = getString(amx, params[2], true), value = getString(amx, params[3], false);
	int id = static_cast<int>(params[1]), index = 0;
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(id);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[id] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[id][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetString(AMX* amx, cell* params)
{
	CHECK_PARAMS(4, "PVarGetString");
	std::string name = getString(amx, params[2], true);
	int size = static_cast<int>(params[4]), id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(std::string))
			{
				cell* dest = NULL;
				amx_GetAddr(amx, params[3], &dest);
				amx_SetString(dest, boost::get<std::string>(j->second.get<1>()).c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarSetFloat(AMX* amx, cell* params)
{
	CHECK_PARAMS(3, "PVarSetFloat");
	std::string name = getString(amx, params[2], true);
	float value = amx_ctof(params[3]);
	int id = static_cast<int>(params[1]), index = 0;
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			j->second = boost::make_tuple(j->second.get<0>(), value);
			return 1;
		}
	}
	IndexMap::iterator k = indexMap.find(id);
	if (k != indexMap.end())
	{
		if (!k->second.get<1>().empty())
		{
			index = k->second.get<1>().front();
			k->second.get<1>().pop();
		}
		else
		{
			index = ++k->second.get<0>();
		}
	}
	else
	{
		indexMap[id] = boost::make_tuple(0, std::queue<int>());
	}
	mainMap[id][name] = boost::make_tuple(index, value);
	return 1;
}

static cell AMX_NATIVE_CALL n_PVarGetFloat(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "PVarGetFloat");
	std::string name = getString(amx, params[2], true);
	int id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(float))
			{
				float value = boost::get<float>(j->second.get<1>());
				return amx_ftoc(value);
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarDelete(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "PVarDelete");
	std::string name = getString(amx, params[2], true);
	int id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			IndexMap::iterator k = indexMap.find(id);
			if (k != indexMap.end())
			{
				k->second.get<1>().push(j->second.get<0>());
			}
			i->second.erase(j);
			if (i->second.empty())
			{
				indexMap.erase(k);
				mainMap.erase(i);
			}
			return 1;
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarsGetUpperIndex(AMX* amx, cell* params)
{
	CHECK_PARAMS(1, "PVarsGetUpperIndex");
	int id = static_cast<int>(params[1]), index = 0;
	IndexMap::iterator k = indexMap.find(id);
	if (k != indexMap.end())
	{
		if (k->second.get<1>().empty())
		{
			index = k->second.get<0>();
			return static_cast<cell>(index + 1);
		}
	}
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		for (DataMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->second.get<0>() > index)
			{
				index = j->second.get<0>();
			}
		}
		return static_cast<cell>(index + 1);
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarGetNameAtIndex(AMX* amx, cell* params)
{
	CHECK_PARAMS(4, "PVarGetNameAtIndex");
	int index = static_cast<int>(params[2]), size = static_cast<int>(params[4]), id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		for (DataMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->second.get<0>() == index)
			{
				cell* dest = NULL;
				amx_GetAddr(amx, params[3], &dest);
				amx_SetString(dest, j->first.c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_PVarGetType(AMX* amx, cell* params)
{
	CHECK_PARAMS(2, "PVarGetType");
	std::string name = getString(amx, params[2], true);
	int id = static_cast<int>(params[1]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(int))
			{
				return static_cast<cell>(PLAYER_VARTYPE_INT);
			}
			if (j->second.get<1>().type() == typeid(std::string))
			{
				return static_cast<cell>(PLAYER_VARTYPE_STRING);
			}
			if (j->second.get<1>().type() == typeid(float))
			{
				return static_cast<cell>(PLAYER_VARTYPE_FLOAT);
			}
		}
	}
	return static_cast<cell>(PLAYER_VARTYPE_NONE);
}

AMX_NATIVE_INFO natives[] =
{
	{ "GVarSetInt", n_GVarSetInt },
	{ "GVarGetInt", n_GVarGetInt },
	{ "GVarSetString", n_GVarSetString },
	{ "GVarGetString", n_GVarGetString },
	{ "GVarSetFloat", n_GVarSetFloat },
	{ "GVarGetFloat", n_GVarGetFloat },
	{ "GVarDelete", n_GVarDelete },
	{ "GVarsGetUpperIndex", n_GVarsGetUpperIndex },
	{ "GVarGetNameAtIndex", n_GVarGetNameAtIndex },
	{ "GVarGetType", n_GVarGetType },
	{ "PVarSetInt", n_PVarSetInt },
	{ "PVarGetInt", n_PVarGetInt },
	{ "PVarSetString", n_PVarSetString },
	{ "PVarGetString", n_PVarGetString },
	{ "PVarSetFloat", n_PVarSetFloat },
	{ "PVarGetFloat", n_PVarGetFloat },
	{ "PVarDelete", n_PVarDelete },
	{ "PVarsGetUpperIndex", n_PVarsGetUpperIndex },
	{ "PVarGetNameAtIndex", n_PVarGetNameAtIndex },
	{ "PVarGetType", n_PVarGetType },
	//{ "GetGVarIterator", n_GetGVarIterator },
	//{ "ResetGVarIterator", n_ResetGVarIterator },
	//{ "AdvanceGVarIterator", n_AdvanceGVarIterator },

	{ 0, 0 }
};

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerDisconnect(int playerid, int reason)
{
	MainMap::iterator i = mainMap.find(playerid);
	if (i != mainMap.end())
	{
		IndexMap::iterator k = indexMap.find(playerid);
		if (k != indexMap.end())
		{
			indexMap.erase(k);
			mainMap.erase(i);
		}
	}
	return 1;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	return AMX_ERR_NONE;
}

