/********************************************************************

    filename:   lua_filesystem.cpp  
    
    Copyright (c) 1:4:2013 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "lua_filesystem.h"
#include <boost/filesystem.hpp>

#define nullptr (std::nullptr_t())


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_modifiedDate(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    time_t t=boost::filesystem::last_write_time(filepath, ec);
    if (!ec) {
        lua_pushnumber(L, (lua_Number)t);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_fileSize(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    size_t t=boost::filesystem::file_size(filepath, ec);
    if (!ec) {
        lua_pushinteger(L, t);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_currentDate(lua_State* L) {
    time_t t;
    time(&t);
    lua_pushnumber(L, (lua_Number)t);
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_exists(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::exists(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isFile(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::is_regular_file(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isDirectory(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::is_directory(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_makeDirectories(lua_State* L) {
    luaL_checkstring(L, -1);
    std::string p=lua_tostring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(p);
    bool r=boost::filesystem::create_directories(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_getCurrentPath(lua_State* L) {
    boost::system::error_code ec;
    boost::filesystem::path cd;
    cd=boost::filesystem::current_path(ec);
    if (ec) {
        lua_pushnil(L);
    } else {
        std::string cdstr=cd.generic_string();
        lua_pushstring(L, cdstr.c_str());
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_absolute(lua_State* L) {
    std::string ret;
    boost::filesystem::path path = luaL_checkstring(L, 1);
    if (lua_isstring(L, 2)) {
        boost::filesystem::path base = luaL_checkstring(L, 2);
        ret = boost::filesystem::absolute(path, base).generic_string();
        lua_pushstring(L, ret.c_str());
    } else {
        ret = boost::filesystem::absolute(path).generic_string();
        lua_pushstring(L, ret.c_str());
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_canonical(lua_State* L) {
    std::string ret;
    boost::system::error_code ec;
    boost::filesystem::path path = luaL_checkstring(L, 1);
    if (lua_isstring(L, 2)) {
        boost::filesystem::path base = luaL_checkstring(L, 2);
        ret = boost::filesystem::canonical(path, base, ec).generic_string();
        if (ec) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, ret.c_str());
        }
    } else {
        ret = boost::filesystem::canonical(path, ec).generic_string();
        if (ec) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, ret.c_str());
        }
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_genericPath(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_nativePath(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_fileWithExt(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.filename().generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_pathWithoutExt(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.replace_extension().generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_pathRoot(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.parent_path().generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_readDir(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    
    boost::system::error_code ec;
    boost::filesystem::directory_iterator itr(path, ec);

    if (ec) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    int tableindex = 1;
    for (boost::filesystem::directory_iterator n; itr != n; itr.increment(ec)) {
        if (ec) {
            break; //just return what we got
        }
        std::string entry = itr->path().generic_string();
        lua_pushstring(L, entry.c_str());
        lua_rawseti(L, -2, tableindex++);
    }

    return 1; // return table 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_readDirRecursive(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);

    boost::system::error_code ec;
    boost::filesystem::recursive_directory_iterator itr(path, boost::filesystem::symlink_option::recurse, ec);

    if (ec) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    int tableindex = 1;
    for (boost::filesystem::recursive_directory_iterator n; itr != n; itr.increment(ec)) {
        if (ec) {
            break; //just return what we got
        }
        std::string entry = itr->path().generic_string();
        lua_pushstring(L, entry.c_str());
        lua_rawseti(L, -2, tableindex++);
    }

    return 1; // return table 
}

enum {
    WC_TRAILINGBACKSLASH = 1,
    WC_UNCLOSEDCLASS,
    WC_INVALIDRANGE
};

/*
 * Error reporting is done by returning various negative values
 * from the wildcard routines. Passing any such value to wc_error
 * will give a human-readable message.
 */
const char *wc_error(int value)
{
    value = abs(value);
    switch (value) {
      case WC_TRAILINGBACKSLASH:
	return "'\' occurred at end of string (expected another character)";
      case WC_UNCLOSEDCLASS:
	return "expected ']' to close character class";
      case WC_INVALIDRANGE:
	return "character range was not terminated (']' just after '-')";
    }
    return "INTERNAL ERROR: unrecognised wildcard error number";
}

/*
 * This is the routine that tests a target string to see if an
 * initial substring of it matches a fragment. If successful, it
 * returns 1, and advances both `fragment' and `target' past the
 * fragment and matching substring respectively. If unsuccessful it
 * returns zero. If the wildcard fragment suffers a syntax error,
 * it returns <0 and the precise value indexes into wc_error.
 */
static int wc_match_fragment(const char **fragment, const char **target)
{
    const char *f, *t;

    f = *fragment;
    t = *target;
    /*
     * The fragment terminates at either the end of the string, or
     * the first (unescaped) *.
     */
    while (*f && *f != '*' && *t) {
	/*
	 * Extract one character from t, and one character's worth
	 * of pattern from f, and step along both. Return 0 if they
	 * fail to match.
	 */
	if (*f == '\\') {
	    /*
	     * Backslash, which means f[1] is to be treated as a
	     * literal character no matter what it is. It may not
	     * be the end of the string.
	     */
	    if (!f[1])
		return -WC_TRAILINGBACKSLASH;   /* error */
	    if (f[1] != *t)
		return 0;	       /* failed to match */
	    f += 2;
	} else if (*f == '?') {
	    /*
	     * Question mark matches anything.
	     */
	    f++;
	} else if (*f == '[') {
	    int invert = 0;
	    int matched = 0;
	    /*
	     * Open bracket introduces a character class.
	     */
	    f++;
	    if (*f == '^') {
		invert = 1;
		f++;
	    }
	    while (*f != ']') {
		if (*f == '\\')
		    f++;	       /* backslashes still work */
		if (!*f)
		    return -WC_UNCLOSEDCLASS;   /* error again */
		if (f[1] == '-') {
		    int lower, upper, ourchr;
		    lower = (unsigned char) *f++;
		    f++;	       /* eat the minus */
		    if (*f == ']')
			return -WC_INVALIDRANGE;   /* different error! */
		    if (*f == '\\')
			f++;	       /* backslashes _still_ work */
		    if (!*f)
			return -WC_UNCLOSEDCLASS;   /* error again */
		    upper = (unsigned char) *f++;
		    ourchr = (unsigned char) *t;
		    if (lower > upper) {
			int t = lower; lower = upper; upper = t;
		    }
		    if (ourchr >= lower && ourchr <= upper)
			matched = 1;
		} else {
		    matched |= (*t == *f++);
		}
	    }
	    if (invert == matched)
		return 0;	       /* failed to match character class */
	    f++;		       /* eat the ] */
	} else {
	    /*
	     * Non-special character matches itself.
	     */
	    if (*f != *t)
		return 0;
	    f++;
	}
	/*
	 * Now we've done that, increment t past the character we
	 * matched.
	 */
	t++;
    }
    if (!*f || *f == '*') {
	/*
	 * We have reached the end of f without finding a mismatch;
	 * so we're done. Update the caller pointers and return 1.
	 */
	*fragment = f;
	*target = t;
	return 1;
    }
    /*
     * Otherwise, we must have reached the end of t before we
     * reached the end of f; so we've failed. Return 0. 
     */
    return 0;
}

/*
 * This is the real wildcard matching routine. It returns 1 for a
 * successful match, 0 for an unsuccessful match, and <0 for a
 * syntax error in the wildcard.
 */
int wildcardMatch(const char *wildcard, const char *target)
{
    int ret;

    /*
     * Every time we see a '*' _followed_ by a fragment, we just
     * search along the string for a location at which the fragment
     * matches. The only special case is when we see a fragment
     * right at the start, in which case we just call the matching
     * routine once and give up if it fails.
     */
    if (*wildcard != '*') {
	ret = wc_match_fragment(&wildcard, &target);
	if (ret <= 0)
	    return ret;		       /* pass back failure or error alike */
    }

    while (*wildcard) {
	while (*wildcard == '*')
	    wildcard++;

	/*
	 * It's possible we've just hit the end of the wildcard
	 * after seeing a *, in which case there's no need to
	 * bother searching any more because we've won.
	 */
	if (!*wildcard)
	    return 1;

	/*
	 * Now `wildcard' points at the next fragment. So we
	 * attempt to match it against `target', and if that fails
	 * we increment `target' and try again, and so on. When we
	 * find we're about to try matching against the empty
	 * string, we give up and return 0.
	 */
	ret = 0;
	while (*target) {
	    const char *save_w = wildcard, *save_t = target;

	    ret = wc_match_fragment(&wildcard, &target);

	    if (ret < 0)
		return ret;	       /* syntax error */

	    if (ret > 0 && !*wildcard && *target) {
		/*
		 * Final special case - literally.
		 * 
		 * This situation arises when we are matching a
		 * _terminal_ fragment of the wildcard (that is,
		 * there is nothing after it, e.g. "*a"), and it
		 * has matched _too early_. For example, matching
		 * "*a" against "parka" will match the "a" fragment
		 * against the _first_ a, and then (if it weren't
		 * for this special case) matching would fail
		 * because we're at the end of the wildcard but not
		 * at the end of the target string.
		 * 
		 * In this case what we must do is measure the
		 * length of the fragment in the target (which is
		 * why we saved `target'), jump straight to that
		 * distance from the end of the string using
		 * strlen, and match the same fragment again there
		 * (which is why we saved `wildcard'). Then we
		 * return whatever that operation returns.
		 */
		target = save_t + strlen(save_t) - (target - save_t);
		wildcard = save_w;
		return wc_match_fragment(&wildcard, &target);
	    }

	    if (ret > 0)
		break;
	    target++;
	}
	if (ret > 0)
	    continue;
	return 0;
    }

    /*
     * If we reach here, it must be because we successfully matched
     * a fragment and then found ourselves right at the end of the
     * wildcard. Hence, we return 1 if and only if we are also
     * right at the end of the target.
     */
    return (*target ? 0 : 1);
}

/*
 * Another utility routine that translates a non-wildcard string
 * into its raw equivalent by removing any escaping backslashes.
 * Expects a target string buffer of anything up to the length of
 * the original wildcard. You can also pass NULL as the output
 * buffer if you're only interested in the return value.
 * 
 * Returns 1 on success, or 0 if a wildcard character was
 * encountered. In the latter case the output string MAY not be
 * zero-terminated and you should not use it for anything!
 */
int wc_unescape(char *output, const char *wildcard)
{
    while (*wildcard) {
	if (*wildcard == '\\') {
	    wildcard++;
	    /* We are lenient about trailing backslashes in non-wildcards. */
	    if (*wildcard) {
		if (output)
		    *output++ = *wildcard;
		wildcard++;
	    }
	} else if (*wildcard == '*' || *wildcard == '?' ||
		   *wildcard == '[' || *wildcard == ']') {
	    return 0;		       /* it's a wildcard! */
	} else {
	    if (output)
		*output++ = *wildcard;
	    wildcard++;
	}
    }
    *output = '\0';
    return 1;			       /* it's clean */
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_wildcardPathMatch(lua_State* L) {
    int ret = wildcardMatch(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    if (ret == 1) {
        lua_pushboolean(L, 1);
    } else {
        lua_pushboolean(L, 0);
    }
    return 1;
}

extern "C" {
//Lua entry point calls
luaFILESYSTEM_EXPORT int luaFILESYSTEM_API luaopen_filesystem(lua_State *L) {
    static const luaL_Reg filesystemlib[] = {
        {"modifieddate",fs_modifiedDate},
        {"filesize",fs_fileSize},
        {"currentfiledate", fs_currentDate},
        {"exists",fs_exists},
        {"isfile",fs_isFile},
        {"isdirectory",fs_isDirectory},
        {"makedirectories",fs_makeDirectories},
        {"getcurrentpath",fs_getCurrentPath},
        {"absolute",fs_absolute},
        {"canonical",fs_canonical},
        {"genericpath",fs_genericPath},
        {"nativepath",fs_nativePath},
        {"filewithext", fs_fileWithExt},
        {"pathwithoutext", fs_pathWithoutExt},
        {"parentpath", fs_pathRoot},
        {"readdir", fs_readDir},
        {"readdirrecursive", fs_readDirRecursive},
        {"wildcardpathmatch", fs_wildcardPathMatch},
        {NULL, NULL}
    };

    luaL_newlib(L, filesystemlib);
    //lua_setglobal(L, "filesystem");
    return 1;
}
};
