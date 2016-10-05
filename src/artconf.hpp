#pragma once
#include <string>
#include <vector>
#include <utility>


//forward declaring fun
extern "C"
{
struct lua_State;
}

//simple "variant"
//it's either a static double, or a reference to Lua function
struct Variant
{
	bool isVariant = false;
	double value = 0.0;
	int funref = 0;
};

class ArtConf
{
public:
    bool isOpen = false;
	lua_State* lua;
};

bool ArtConfAccessArray(ArtConf& t, const char* name, int index);
double ArtConfGetGlobalDouble(ArtConf& t, const char* name);
int ArtConfGetGlobalInt(ArtConf& t, const char* name);

std::string ArtConfGetGlobalString(ArtConf& t, const char* name);
std::string ArtConfGetArrayString(ArtConf& t, const char* array, int index);

int ArtConfGetArrayInt(ArtConf& t, const char* array, int index);
Variant ArtConfToVariant(ArtConf& t);

Variant ArtConfGetArrayVariant(ArtConf& t, const char* array, int index);
double ArtConfVariant(ArtConf& t, Variant v, const std::vector<std::pair<double, std::string>>& input);
std::vector<std::pair<std::string,Variant>> ArtConfGetVariantTable(ArtConf t, const char* name);
void LoadConfig(ArtConf t, std::string file);
ArtConf ArtConfOpen(std::string conf);
void ArtConfClose(ArtConf& t);

