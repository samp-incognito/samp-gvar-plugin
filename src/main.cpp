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

#include "main.h"

#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include <sdk/plugin.h>

#include <algorithm>
#include <queue>
#include <string>

IndexMap indexMap;
MainMap mainMap;

logprintf_t logprintf;

std::string getString(AMX *amx, cell param, bool toLower)
{
	std::string name;
	char *string = NULL;
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
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("\n\n*** GVar Plugin v%s by Incognito loaded ***\n", PLUGIN_VERSION);
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	logprintf("\n\n*** GVar Plugin v%s by Incognito unloaded ***\n", PLUGIN_VERSION);
}

static cell AMX_NATIVE_CALL n_SetGVarInt(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "SetGVarInt");
	std::string name = getString(amx, params[1], true);
	int value = static_cast<int>(params[2]), id = static_cast<int>(params[3]), index = 0;
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

static cell AMX_NATIVE_CALL n_GetGVarInt(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "GetGVarInt");
	std::string name = getString(amx, params[1], true);
	int id = static_cast<int>(params[2]);
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

static cell AMX_NATIVE_CALL n_SetGVarString(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "SetGVarString");
	std::string name = getString(amx, params[1], true), value = getString(amx, params[2], false);
	int id = static_cast<int>(params[3]), index = 0;
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

static cell AMX_NATIVE_CALL n_GetGVarString(AMX *amx, cell *params)
{
	CHECK_PARAMS(4, "GetGVarString");
	std::string name = getString(amx, params[1], true);
	int size = static_cast<int>(params[3]), id = static_cast<int>(params[4]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(std::string))
			{
				cell *dest = NULL;
				amx_GetAddr(amx, params[2], &dest);
				amx_SetString(dest, boost::get<std::string>(j->second.get<1>()).c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_SetGVarFloat(AMX *amx, cell *params)
{
	CHECK_PARAMS(3, "SetGVarFloat");
	std::string name = getString(amx, params[1], true);
	float value = amx_ctof(params[2]);
	int id = static_cast<int>(params[3]), index = 0;
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

static cell AMX_NATIVE_CALL n_GetGVarFloat(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "GetGVarFloat");
	std::string name = getString(amx, params[1], true);
	int id = static_cast<int>(params[2]);
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

static cell AMX_NATIVE_CALL n_DeleteGVar(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "DeleteGVar");
	std::string name = getString(amx, params[1], true);
	int id = static_cast<int>(params[2]);
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

static cell AMX_NATIVE_CALL n_GetGVarsUpperIndex(AMX *amx, cell *params)
{
	CHECK_PARAMS(1, "GetGVarsUpperIndex");
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

static cell AMX_NATIVE_CALL n_GetGVarNameAtIndex(AMX *amx, cell *params)
{
	CHECK_PARAMS(4, "GetGVarNameAtIndex");
	int index = static_cast<int>(params[1]), size = static_cast<int>(params[3]), id = static_cast<int>(params[4]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		for (DataMap::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if (j->second.get<0>() == index)
			{
				cell *dest = NULL;
				amx_GetAddr(amx, params[2], &dest);
				amx_SetString(dest, j->first.c_str(), 0, 0, size);
				return 1;
			}
		}
	}
	return 0;
}

static cell AMX_NATIVE_CALL n_GetGVarType(AMX *amx, cell *params)
{
	CHECK_PARAMS(2, "GetGVarType");
	std::string name = getString(amx, params[1], true);
	int id = static_cast<int>(params[2]);
	MainMap::iterator i = mainMap.find(id);
	if (i != mainMap.end())
	{
		DataMap::iterator j = i->second.find(name);
		if (j != i->second.end())
		{
			if (j->second.get<1>().type() == typeid(int))
			{
				return static_cast<cell>(GLOBAL_VARTYPE_INT);
			}
			if (j->second.get<1>().type() == typeid(std::string))
			{
				return static_cast<cell>(GLOBAL_VARTYPE_STRING);
			}
			if (j->second.get<1>().type() == typeid(float))
			{
				return static_cast<cell>(GLOBAL_VARTYPE_FLOAT);
			}
		}
	}
	return static_cast<cell>(GLOBAL_VARTYPE_NONE);
}

AMX_NATIVE_INFO natives[] =
{
	{ "SetGVarInt", n_SetGVarInt },
	{ "GetGVarInt", n_GetGVarInt },
	{ "SetGVarString", n_SetGVarString },
	{ "GetGVarString", n_GetGVarString },
	{ "SetGVarFloat", n_SetGVarFloat },
	{ "GetGVarFloat", n_GetGVarFloat },
	{ "DeleteGVar", n_DeleteGVar },
	{ "GetGVarsUpperIndex", n_GetGVarsUpperIndex },
	{ "GetGVarNameAtIndex", n_GetGVarNameAtIndex },
	{ "GetGVarType", n_GetGVarType },
	{ "GetGVarIterator", n_GetGVarIterator },
	{ "ResetGVarIterator", n_ResetGVarIterator },
	{ "AdvanceGVarIterator", n_AdvanceGVarIterator },
	
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	return AMX_ERR_NONE;
}
