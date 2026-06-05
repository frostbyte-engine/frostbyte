#include "libraries/debuglib.hpp"
#include "Luau/Bytecode.h"
#include "common.hpp"
#include "environment.hpp"

#include "lapi.h"
#include "lcommon.h"
#include "ldebug.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "lua.h"
#include "luaconf.h"
#include "lualib.h"

namespace frostbyte {

// from ldebug.cpp's  lua_getinfo
Closure* levelToClosure(lua_State* L, int level, CallInfo** ci_out = nullptr) {
    Closure* f = nullptr;
    CallInfo* ci = nullptr;

    if (level < 0) {
        // element has to be within stack
        if (-level > L->top - L->base)
            return 0;

        StkId func = L->top + level;

        // and it has to be a function
        if (!ttisfunction(func))
            return 0;

        f = clvalue(func);
    } else if (unsigned(level) < unsigned(L->ci - L->base_ci)) {
        ci = L->ci - level;
        if (ci_out)
            *ci_out = ci;
        LUAU_ASSERT(ttisfunction(ci->func));
        f = clvalue(ci->func);
    }

    return f;
}

int checkStackLevel(lua_State* L, int level) {
    if (level < 1) {
        lua_pushstring(L, "stack level cannot be less than 1");
        lua_error(L);
    } else if (level >= lua_stackdepth(L)) {
        lua_pushstring(L, "stack level is too high");
        lua_error(L);
    }
    return 1;
}

Closure* getClosure(lua_State* L, int index, CallInfo** ci_out = nullptr) {
    Closure* f = nullptr;

    int type = lua_type(L, index);

    switch (type) {
        case LUA_TNUMBER: {
            int level = lua_tointeger(L, 1);
            checkStackLevel(L, level);

            f = levelToClosure(L, level, ci_out);
            break;
        } case LUA_TFUNCTION:
            f = clvalue(luaA_toobject(L, index));
            break;
        default:
            luaL_error(L, "invalid argument #%d to %s (expected number or function, got %s)", index, currfuncname(L), lua_typename(L, type));
            break;
    }

    return f;
}

void checkLClosure(lua_State* L, int narg, Closure* closure) {
    if (closure->isC)
        luaL_error(L, "invalid argument #%d to %s (expected Lua closure, got C closure)", narg, currfuncname(L));
}

lua_State* optionalThread(lua_State* L, int narg) {
    lua_State* thread = L;
    if (!lua_isnone(L, narg)) {
        luaL_checktype(L, narg, LUA_TTHREAD);
        thread = lua_tothread(L, narg);
    }

    return thread;
}

int fr_debug_validlevel(lua_State* L) {
    int level = luaL_checkinteger(L, 1);
    lua_State* thread = optionalThread(L, 2);

    lua_pushboolean(L, level <= lua_stackdepth(thread));
    lua_gettop(L);
    return 1;
}

static int getCurrentpc(lua_State* L, CallInfo* ci) {
    return pcRel(ci->savedpc, ci_func(ci)->l.p);
}

static int getCurrentLine(lua_State* L, CallInfo* ci) {
    return luaG_getline(ci_func(ci)->l.p, getCurrentpc(L, ci));
}
int fr_debug_getcallstack(lua_State* L) {
    lua_State* thread = optionalThread(L, 1);

    CallInfo* ci = thread->base_ci;
    CallInfo* end_ci = thread->ci;

    lua_createtable(L, end_ci - ci, 0);
    int table = lua_absindex(L, -1);

    int index = 0;
    do {
        ci++;

        lua_createtable(L, 2, 0);

        luaA_pushobject(L, ci->func);
        lua_rawsetfield(L, -2, "func");

        auto cl = ci->func->value.gc->cl;

        if (!cl.isC) {
            lua_pushnumber(L, getCurrentLine(thread, ci));
            lua_rawsetfield(L, -2, "currentline");
        }

        lua_rawseti(L, table, ++index);
    } while (ci < end_ci);

    return 1;
}

// ldebug.cpp
static const char* getfuncname(Closure* cl)
{
    if (cl->isC) {
        if (cl->c.debugname)
            return cl->c.debugname;
    } else {
        Proto* p = cl->l.p;

        if (p->debugname)
            return getstr(p->debugname);
    }
    return nullptr;
}
int fr_debug_getinfo(lua_State* L) {
    CallInfo* callinfo = nullptr;
    Closure* closure = getClosure(L, 1, &callinfo);
    const char* what = luaL_optstring(L, 2, "sluanf");

    std::optional<const char*> source;
    std::optional<const char*> _what;
    std::optional<int> linedefined;
    std::optional<const char*> short_src;
    std::optional<int> currentline;
    std::optional<int> nups;
    std::optional<int> is_vararg;
    std::optional<int> numparams;
    std::optional<const char*> name;
    bool func = false;

    char shortsrc_buf[LUA_IDSIZE];

    for (; *what; what++) {
        switch (*what) {
            case 's':
                if (closure->isC) {
                    source = "=[C]";
                    _what = "C";
                    linedefined = -1;
                    short_src = "[C]";
                } else {
                    TString* clsource = closure->l.p->source;
                    source = getstr(clsource);
                    _what = "Lua";
                    linedefined = closure->l.p->linedefined;
                    short_src = luaO_chunkid(shortsrc_buf, sizeof(shortsrc_buf), getstr(clsource), clsource->len);
                }
                break;
            case 'l':
                if (closure->isC)
                    currentline = -1;
                else if (callinfo)
                    currentline = getCurrentLine(L, callinfo);
                else
                    currentline = closure->l.p->linedefined;
                break;
            case 'u':
                nups = closure->nupvalues;
                break;
            case 'a':
                if (closure->isC) {
                    is_vararg = 1;
                    numparams = 0;
                } else {
                    is_vararg = closure->l.p->is_vararg;
                    numparams = closure->l.p->numparams;
                }
                break;
            case 'n':
                // TODO: this is copied from auxgetinfo.. why does it use callinfo??
                name = callinfo ? getfuncname(ci_func(callinfo)) :  getfuncname(closure);
                break;
            case 'f':
                func = true;
                break;
        }
    }

    lua_createtable(L, 0, 9);
    int table = lua_absindex(L, -1);

    if (source) {
        lua_pushstring(L, source.value());
        lua_setfield(L, table, "source");
    }
    if (_what) {
        lua_pushstring(L, _what.value());
        lua_setfield(L, table, "what");
    }
    if (linedefined) {
        lua_pushnumber(L, linedefined.value());
        lua_setfield(L, table, "linedefined");
    }
    if (short_src) {
        lua_pushstring(L, short_src.value());
        lua_setfield(L, table, "short_src");
    }

    if (currentline) {
        lua_pushnumber(L, currentline.value());
        lua_setfield(L, table, "currentline");
    }

    if (nups) {
        lua_pushnumber(L, nups.value());
        lua_setfield(L, table, "nups");
    }

    if (is_vararg) {
        lua_pushnumber(L, is_vararg.value());
        lua_setfield(L, table, "is_vararg");
    }
    if (numparams) {
        lua_pushnumber(L, numparams.value());
        lua_setfield(L, table, "numparams");
    }

    if (name) {
        lua_pushstring(L, name.value());
        lua_setfield(L, table, "name");
    }

    if (func) {
        lua_pushnil(L);
        setclvalue(L, L->top - 1, closure);
        lua_setfield(L, table, "func");
    }

    return 1;
}

Proto* cloneProto(lua_State* L, Proto* original) {
    Proto* cloned = luaF_newproto(L);

    cloned->nups = original->nups;
    cloned->numparams = original->numparams;
    cloned->is_vararg = original->is_vararg;
    cloned->maxstacksize = original->maxstacksize;
    cloned->flags = original->flags;

    // FIXME: should we clone k as well?
    // cloned->k = original->k;

    if (original->sizelineinfo) {
        cloned->sizelineinfo = original->sizelineinfo;
        cloned->lineinfo = luaM_newarray(L, original->sizelineinfo, uint8_t, cloned->memcat);
        memcpy(cloned->lineinfo, original->lineinfo, original->sizelineinfo * sizeof(uint8_t));

        cloned->abslineinfo = (int*)(cloned->lineinfo + ((original->sizecode + 3) & ~3));
    }

    if (original->sizelocvars) {
        cloned->sizelocvars = original->sizelocvars;
        cloned->locvars = luaM_newarray(L, original->sizelocvars, LocVar, cloned->memcat);

        cloned->sizeupvalues = original->sizeupvalues;
        cloned->upvalues = luaM_newarray(L, original->sizeupvalues, TString*, cloned->memcat);

        for (int i = 0; i < original->sizeupvalues; i++)
            cloned->upvalues[i] = luaS_newlstr(L, getstr(original->upvalues[i]), original->upvalues[i]->len);
    }

    cloned->source = original->source;

    cloned->debugname = original->debugname;

    cloned->linegaplog2 = original->linegaplog2;
    cloned->linedefined = original->linedefined;
    cloned->bytecodeid = original->bytecodeid;
    // cloned->sizetypeinfo = original->sizetypeinfo;

    cloned->sizecode = 1;
    cloned->code = luaM_newarray(L, 1, Instruction, cloned->memcat);
    cloned->code[0] = Instruction(LOP_RETURN) | (1 << 16); // RETURN B1
    cloned->codeentry = cloned->code;

    return cloned;
}
int fr_debug_getprotos(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure);
    Proto* p = closure->l.p;

    lua_createtable(L, p->sizep, 0);
    int table = lua_absindex(L, -1);

    for (int i = 0; i < p->sizep; i++) {
        Proto* original = p->p[i];

        Closure* lclosure = luaF_newLclosure(L, original->nups, closure->env, cloneProto(L, original));
        lua_pushnil(L);
        setclvalue(L, L->top - 1, lclosure);

        lua_rawseti(L, table, i + 1);
    }

    return 1;
}
int fr_debug_getproto(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure);
    Proto* p = closure->l.p;

    int index = luaL_checknumberrange(L, 2, 1, p->sizep, "index");

    if (lua_isboolean(L, 3))
        luaL_error(L, "active is not yet implemented");

    Proto* original = p->p[index - 1];

    Closure* lclosure = luaF_newLclosure(L, original->nups, closure->env, cloneProto(L, original));
    lua_pushnil(L);
    setclvalue(L, L->top - 1, lclosure);

    return 1;
}
// apparently unsafe, should look into this
// int fr_debug_setproto(lua_State* L) {
//     Closure* closure = getClosure(L, 1);
//     checkLClosure(L, 1, closure);
//     Proto* p = closure->l.p;

//     int index = luaL_checknumberrange(L, 2, 1, p->sizep, "index");

//     Closure* value = getClosure(L, 3);
//     checkLClosure(L, 3, value);

//     *p->p[index - 1] = *value->l.p;

//     return 0;
// }

int fr_debug_getconstant(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure);
    Proto* p = closure->l.p;

    int index = luaL_checknumberrange(L, 2, 1, p->sizek, "index");

    luaA_pushobject(L, &p->k[index - 1]);

    return 1;
}
int fr_debug_getconstants(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure);
    Proto* p = closure->l.p;

    lua_createtable(L, p->sizek, 0);
    int table = lua_absindex(L, -1);

    for (int i = 0; i < p->sizek; i++) {
        const TValue* obj = &p->k[i];
        if (obj->tt == LUA_TFUNCTION)
            lua_newuserdata(L, 0);
        else
            luaA_pushobject(L, obj);
        lua_rawseti(L, table, i + 1);
    }

    return 1;
}
int fr_debug_setconstant(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure);
    Proto* p = closure->l.p;

    int index = luaL_checknumberrange(L, 2, 1, p->sizek, "index");
    luaL_checkany(L, 3);

    int valuet = lua_type(L, 3);
    if (valuet == LUA_TNIL || valuet == LUA_TBOOLEAN || valuet == LUA_TNUMBER ||
        valuet == LUA_TVECTOR || valuet == LUA_TSTRING || valuet == LUA_TTABLE)
    {
        TValue* original = &p->k[index - 1];
        const TValue* replacement = luaA_toobject(L, 3);

        if (replacement->tt != original->tt)
            luaL_typeerror(L, 3, lua_typename(L, original->tt));

        setobj(L, original, replacement);
        luaC_barrier(L, closure, replacement)
    } else
        luaL_error(L, "invalid argument #3 to getconstants (expected nil, boolean, number, vector, string, table, or function, got %s", lua_typename(L, valuet));

    return 0;
}

int fr_debug_getupvalue(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure); // TODO: toggle allowing C closures

    int index = luaL_checknumberrange(L, 2, 1, closure->nupvalues, "index");

    lua_getupvalue(L, 1, index);

    return 1;
}
int fr_debug_getupvalues(lua_State* L) {
    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure); // TODO: toggle allowing C closures

    lua_createtable(L, closure->nupvalues, 0);
    int table = lua_absindex(L, -1);

    for (int i = 0; i < closure->nupvalues; i++) {
        TValue* r = &closure->l.uprefs[i];
        r = ttisupval(r) ? upvalue(r)->v : r;
        luaA_pushobject(L, r);
        lua_rawseti(L, table, i + 1);
    }

    return 1;
}
int fr_debug_setupvalue(lua_State* L) {
    // setupvalue expects value to be at top
    if (lua_gettop(L) > 3)
        luaL_error(L, "too many arguments to setupvalue! expected 3");

    Closure* closure = getClosure(L, 1);
    checkLClosure(L, 1, closure); // TODO: toggle allowing C closures

    int index = luaL_checknumberrange(L, 2, 1, closure->nupvalues, "index");

    luaL_checkany(L, 3);

    lua_setupvalue(L, 1, index);

    return 0;
}
int fr_debug_upvaluejoin(lua_State* L) {
    Closure* closure_1 = getClosure(L, 1);
    Closure* closure_2 = getClosure(L, 3);
    checkLClosure(L, 1, closure_1); // TODO: toggle allowing C closures
    checkLClosure(L, 3, closure_2); // TODO: toggle allowing C closures

    int n_1 = luaL_checknumberrange(L, 2, 1, closure_1->nupvalues, "n1");
    int n_2 = luaL_checknumberrange(L, 4, 1, closure_2->nupvalues, "n2");

    TValue* o_1 = &closure_1->l.uprefs[n_1 - 1];
    TValue* o_2 = &closure_2->l.uprefs[n_2 - 1];

    // FIXME: I'm not sure the idea of setobj here is correct
    setobj(L, o_1, o_2);
    luaC_barrier(L, closure_1, o_2);

    return 0;
}

int fr_debug_getstack(lua_State* L) {
    int level = luaL_checkinteger(L, 1);
    CallInfo* ci = L->ci - level;
    LUAU_ASSERT(ttisfunction(ci->func));

    int top = ci->top - ci->base;

    if (lua_isnone(L, 2)) {
        lua_createtable(L, top, 0);
        int table = lua_absindex(L, -1);

        int index = 0;
        for (StkId val = ci->base; val < ci->top; ++val) {
            luaA_pushobject(L, val);
            lua_rawseti(L, table, ++index);
        }

        return 1;
    }

    int index = luaL_checknumberrange(L, 2, 1, top, "index");
    luaA_pushobject(L, ci->base + index - 1);

    return 1;
}
int fr_debug_setstack(lua_State* L) {
    int level = luaL_checkinteger(L, 1);
    CallInfo* ci = L->ci - level;
    LUAU_ASSERT(ttisfunction(ci->func));

    int top = ci->top - ci->base;

    int index = luaL_checknumberrange(L, 2, 1, top, "index");

    luaL_checkany(L, 3);


    TValue* original = ci->base + index - 1;
    const TValue* replacement = luaA_toobject(L, 3);

    // TODO: an error that is more specific to this function
    if (replacement->tt != original->tt)
        luaL_typeerror(L, 3, lua_typename(L, original->tt));

    setobj(L, original, replacement);
    luaC_barrier(L, ci->func, replacement);

    return 0;
}

int fr_debug_setname(lua_State* L) {
    Closure* closure = getClosure(L, 1);

    size_t l;
    const char* name = luaL_checklstring(L, 2, &l);

    if (closure->isC)
        closure->c.debugname = name;
    else
        closure->l.p->debugname = luaS_newlstr(L, name, l);

    return 0;
}

void open_debuglib(lua_State* L) {
    lua_getglobal(L, "debug");

    setfunctionfield(L, fr_debug_validlevel, "validlevel");
    setfunctionfield(L, fr_debug_getcallstack, "getcallstack");

    setfunctionfield(L, fr_debug_getinfo, "getinfo");

    setfunctionfield(L, fr_debug_getprotos, "getprotos");
    setfunctionfield(L, fr_debug_getproto, "getproto");
    // setfunctionfield(L, fr_debug_setproto, "setproto");

    setfunctionfield(L, fr_debug_getconstant, "getconstant");
    setfunctionfield(L, fr_debug_getconstants, "getconstants");
    setfunctionfield(L, fr_debug_setconstant, "setconstant");

    setfunctionfield(L, fr_debug_getupvalue, "getupvalue");
    setfunctionfield(L, fr_debug_getupvalues, "getupvalues");
    setfunctionfield(L, fr_debug_setupvalue, "setupvalue");
    setfunctionfield(L, fr_debug_upvaluejoin, "upvaluejoin");

    setfunctionfield(L, fr_debug_getstack, "getstack");
    setfunctionfield(L, fr_debug_setstack, "setstack");

    setfunctionfield(L, fr_getrawmetatable, "getmetatable");
    setfunctionfield(L, fr_setrawmetatable, "setmetatable");
    setfunctionfield(L, fr_getreg, "getregistry");

    setfunctionfield(L, fr_debug_setname, "setname");

    lua_pop(L, 1);
}

}
