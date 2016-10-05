
#include <iostream>
#include "artconf.hpp"
#include <lua.hpp>

#if LUA_VERSION_NUM != 503
#warning Only Lua 5.3 is supported
#endif

//Expects the table on top of stack
//Returns false if access failed, and pops the table
//Otherwise leaves the accessed element and the table on the stack
bool ArtConfAccessArray(ArtConf& t, const char* name, int index)
{
	if (lua_isnil(t.lua,-1))
	{
		lua_pop(t.lua,1);
		return false;
	}
	lua_pushstring(t.lua,name);
	lua_gettable(t.lua,-2);

	if (lua_isnil(t.lua,-1))
	{
		lua_pop(t.lua,2);
		return false;
	}

	lua_pushinteger(t.lua,index);
	lua_gettable(t.lua,-2);

	lua_remove(t.lua,-2);
	return true;
}



double ArtConfGetGlobalDouble(ArtConf& t, const char* name)
{
	lua_getglobal(t.lua, name);
	int isnum = 0;
	double val = -1;
	double ret = lua_tonumberx(t.lua,-1,&isnum);
	if (isnum)
	{
		val = ret;
	}

	lua_pop(t.lua,1);
	return val;
}


int ArtConfGetGlobalInt(ArtConf& t, const char* name)
{
	lua_getglobal(t.lua, name);
	int isnum = 0;
	int val = -1;
	int ret = lua_tointegerx(t.lua,-1,&isnum);
	if (isnum)
	{
		val = ret;
	}

	lua_pop(t.lua,1);
	return val;
}

std::string ArtConfGetGlobalString(ArtConf& t, const char* name)
{
	lua_getglobal(t.lua, name);
	const char* ret = lua_tostring(t.lua,-1);
	std::string val = "";
	if (ret)
	{
		val = std::string(ret);
	}

	lua_pop(t.lua,1);
	return val;
}

std::string ArtConfGetArrayString(ArtConf& t, const char* array, int index)
{
	lua_pushglobaltable(t.lua);
	if (!ArtConfAccessArray(t,array,index))
	{
		return "";
	}

	const char* ret = lua_tostring(t.lua,-1);
	std::string val = "";
	if (ret)
	{
		val = std::string(ret);
	}

	lua_pop(t.lua,2);
	return val;
}


int ArtConfGetArrayInt(ArtConf& t, const char* array, int index)
{
	lua_pushglobaltable(t.lua);
	if (!ArtConfAccessArray(t,array,index))
		return -1;

	int isnum;
	int val = -1;
	int ret = lua_tointegerx(t.lua,-1,&isnum);
	if (isnum)
	{
		val = ret;
	}

	lua_pop(t.lua,2);
	return val;
}

Variant ArtConfToVariant(ArtConf& t)
{
	Variant v;
	v.isVariant = false;
	v.value = -1;
    v.funref = LUA_NOREF;

	if (lua_isnumber(t.lua, -1))
	{
		double ret = lua_tonumber(t.lua,-1);
		lua_pop(t.lua,1);
		v.value = ret;
		return v;
	}
	if (!lua_isfunction(t.lua,-1))
	{
		lua_pop(t.lua,1);
		return v;
	}

	int funref = luaL_ref(t.lua, LUA_REGISTRYINDEX);
	v.isVariant = true;
	v.funref = funref;
	return v;
}

Variant ArtConfGetArrayVariant(ArtConf& t, const char* array, int index)
{
	Variant v;
	v.isVariant = false;
	v.value = -1;
	v.funref = LUA_NOREF;

	lua_pushglobaltable(t.lua);
	if (!ArtConfAccessArray(t,array,index))
		return v;
	v = ArtConfToVariant(t);
	lua_pop(t.lua, 1);
	return v;
}

double ArtConfVariant(ArtConf& t, Variant v, const std::vector<std::pair<double, std::string>>& input)
{
	if (v.isVariant == false)
		return v.value;

	lua_rawgeti(t.lua, LUA_REGISTRYINDEX, v.funref);
	lua_newtable(t.lua);

	for (auto p : input)
	{
		lua_pushstring(t.lua, p.second.c_str());
		lua_pushnumber(t.lua, p.first);
		lua_settable(t.lua,-3);
	}
	int error = lua_pcall(t.lua,1,1,0);
	if (error)
	{
		std::string e;
		e = std::string(lua_tostring(t.lua, -1));
		std::cout << e << std::endl;
		lua_pop(t.lua, 1);

		return -1;
	}
	double ret = lua_tonumber(t.lua,-1);
	lua_pop(t.lua,1);
	return ret;
}

std::vector<std::pair<std::string,Variant>> ArtConfGetVariantTable(ArtConf t, const char* name)
{
	std::vector<std::pair<std::string,Variant>> ret;

	lua_getglobal(t.lua, name);
	if (!lua_istable(t.lua,-1))
	{
		lua_pop(t.lua,1);
		return ret;
	}
	lua_pushnil(t.lua);
	while (lua_next(t.lua, -2))
    {
		std::string vm = std::string(lua_tostring(t.lua,-2));

		Variant v = ArtConfToVariant(t);
		ret.push_back({vm,v});
    }
	return ret;
}

void LoadConfig(ArtConf t, std::string file)
{
	int error = luaL_loadfile(t.lua, file.c_str());
	if (!error)
		error = lua_pcall(t.lua, 0, 0, 0);
	if (error)
	{
		auto e = lua_tostring(t.lua, -1);
		throw e;
	}
}

ArtConf ArtConfOpen(std::string conf)
{
	ArtConf t;
	t.lua = luaL_newstate();

	luaL_openlibs(t.lua);

	lua_newtable(t.lua);
	lua_setglobal(t.lua, "pass_shader");

	lua_newtable(t.lua);
	lua_setglobal(t.lua, "pass_wrap");

	lua_newtable(t.lua);
	lua_setglobal(t.lua, "uniforms");

	lua_pushinteger(t.lua, 0);
	lua_setglobal(t.lua, "Wrap_Clamp");

	lua_pushinteger(t.lua, 1);
	lua_setglobal(t.lua, "Wrap_Repeat");

	lua_pushinteger(t.lua, 2);
	lua_setglobal(t.lua, "Wrap_Mirror");

	LoadConfig(t,conf);

    t.isOpen = true;
	return t;
}


void ArtConfClose(ArtConf& t)
{
    if (t.isOpen == false)
        return;

	lua_close(t.lua);
}

