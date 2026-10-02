#include "engine/datatypes/random.hpp"

#include "common.hpp"
#include "engine/datatypes/vector3.hpp"
#include "lualib.h"
#include "userdata.hpp"

#include "lua.h"
#include <cmath>
#include <random>

namespace frostbyte {

std::random_device seed_random;

static constexpr uint64_t PCG_MULT = 6364136223846793005ull;
static constexpr uint64_t PCG_INC  = 105ull;

uint32_t Random::nextUint() {
    uint64_t old = state;
    state = old * PCG_MULT + PCG_INC;
    uint32_t xorshifted = static_cast<uint32_t>(((old >> 18u) ^ old) >> 27u);
    uint32_t rot = static_cast<uint32_t>(old >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-static_cast<int32_t>(rot)) & 31));
}
int Random::nextInteger(int min, int max) {
    uint32_t ul = static_cast<uint32_t>(max) - static_cast<uint32_t>(min);
    uint64_t x = static_cast<uint64_t>(ul + 1) * nextUint();
    return static_cast<int>(min + (x >> 32));
}
double Random::nextDouble() {
    uint32_t rl = nextUint();
    uint32_t rh = nextUint();
    return std::ldexp(static_cast<double>(rl | (static_cast<uint64_t>(rh) << 32)), -64);
}

int pushRandom(lua_State* L, Random* old) {
    Random* random = static_cast<Random*>(lua_newuserdatatagged(L, sizeof(Random), userdata::Random));
    new (random) Random(*old);

    userdata::getClassMetatable(L, userdata::Random);
    lua_setmetatable(L, -2);

    return 1;
}
int pushRandom(lua_State* L, uint64_t seed) {
    Random random(seed);

    random.state = 0;
    random.nextUint();
    random.state += seed;
    random.nextUint();

    return pushRandom(L, &random);
}

Random* lua_checkrandom(lua_State* L, int narg) {
    return static_cast<Random*>(userdata::check(L, narg, userdata::Random));
}

static int Random_new(lua_State *L) {
    int64_t seed = 0;
    if (lua_isnumber(L, 1))
        seed = lua_tointeger(L, 1);
    else if (lua_isnone(L, 1))
        seed = seed_random();
    else
        luaL_typeerror(L, 1, "number or nil");

    return pushRandom(L, seed);
}

namespace Random_methods {
    static int nextInteger(lua_State* L) {
        Random* random = lua_checkrandom(L, 1);

        int min = luaL_checkinteger(L, 2);
        int max = luaL_checkinteger(L, 3);

        lua_pushinteger(L, random->nextInteger(min, max));

        return 1;
    }
    static int nextNumber(lua_State* L) {
        Random* random = lua_checkrandom(L, 1);

        const int narg = lua_gettop(L);
        if (narg == 1) {
            lua_pushnumber(L, random->nextDouble());

            return 1;
        } else if (narg == 3) {
            double min = luaL_checknumber(L, 2);
            double max = luaL_checknumber(L, 3);

            lua_pushnumber(L, min + (max - min) * random->nextDouble());

            return 1;
        }

        luaL_error(L, "invalid number of args to NextNumber (expected 0 or 2, got %d)", narg - 1);
    }
    static int nextUnitVector(lua_State* L) {
        Random* random = lua_checkrandom(L, 1);

        std::normal_distribution<float> dist(0.f, 1.f);

        float x = static_cast<float>(random->nextDouble());
        float y = static_cast<float>(random->nextDouble());
        float z = static_cast<float>(random->nextDouble());

        float len = std::sqrt(x*x + y*y + z*z);

        if (len < 1e-12)
            return nextUnitVector(L);

        pushVector3(L, x / len, y / len, z / len);
        return 1;
    }
    static int shuffle(lua_State* L) {
        luaL_error(L, "INTERNAL ERROR: implement Random.shuffle");
    }
    static int clone(lua_State* L) {
        Random* random = lua_checkrandom(L, 1);

        return pushRandom(L, random);
    }
};

lua_CFunction getRandomMethod(const char* key) {
    if (strequal(key, "NextInteger"))
        return Random_methods::nextInteger;
    else if (strequal(key, "NextNumber"))
        return Random_methods::nextNumber;
    else if (strequal(key, "NextUnitVector"))
        return Random_methods::nextUnitVector;
    else if (strequal(key, "Shuffle"))
        return Random_methods::shuffle;
    else if (strequal(key, "Clone"))
        return Random_methods::clone;

    return nullptr;
}

static int Random__index(lua_State* L) {
    lua_checkrandom(L, 1);
    const char* key = luaL_checkstring(L, 2);

    lua_CFunction func = getRandomMethod(key);
    if (func)
        return pushFunctionFromLookup(L, func, key);

    luaL_error(L, "%s is not a valid member of Random", key);
}

static int Random__namecall(lua_State* L) {
    lua_checkrandom(L, 1);
    const char* namecall = lua_namecallatom(L, nullptr);
    if (!namecall)
        luaL_error(L, "no namecall method!");

    lua_CFunction func = getRandomMethod(namecall);
    if (!func)
        luaL_error(L, "%s is not a valid member of Random", namecall);

    return func(L);
}

void open_randomlib(lua_State* L) {
    // Random
    lua_newtable(L);

    setfunctionfield(L, Random_new, "new", true);

    lua_setglobal(L, "Random");

    // metatable
    userdata::newClassMetatable(L, userdata::Random);
    setfunctionfield(L, Random__index, "__index");
    setfunctionfield(L, Random__namecall, "__namecall");

    lua_pop(L, 1);

    lua_setuserdatadtor(L, userdata::Random, [] (lua_State* L, void* ud) {
        Random* random = static_cast<Random*>(ud);
        random->~Random();
    });
}

};
