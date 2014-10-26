/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lua_filesystem.h"
#include "minfs.h"
#include "cryptoMD5.h"
#include <string.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_modifiedDate(lua_State* L) {
    const char* filepath = luaL_checkstring(L, -1);
    if (!minfs_path_exist(filepath)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushnumber(L, (lua_Number)minfs_get_file_mdate(filepath));
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_fileSize(lua_State* L) {
    const char* filepath = luaL_checkstring(L, -1);
    lua_pushnumber(L, (lua_Number)minfs_get_file_size(filepath));
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_currentDate(lua_State* L) {
    lua_pushnumber(L, (lua_Number)minfs_get_current_file_time());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_exists(lua_State* L) {
    const char* filepath = luaL_checkstring(L, -1);
    lua_pushboolean(L, minfs_path_exist(filepath));
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isFile(lua_State* L) {
    lua_pushboolean(L, minfs_is_file(luaL_checkstring(L, -1)));
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isDirectory(lua_State* L) {
    lua_pushboolean(L, minfs_is_directory(luaL_checkstring(L, -1)));
    return 1;
}

static int fs_canonical(lua_State* L) {
    const char* filepath = luaL_checkstring(L, -1);
    luaL_Buffer canonpath;
    size_t len = 1024;
    
    luaL_buffinitsize(L, &canonpath, len);
    len = minfs_canonical_path(filepath, canonpath.b, len);
    luaL_pushresultsize(&canonpath, len);
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isSymLink(lua_State* L) {
    lua_pushboolean(L, minfs_is_sym_link(luaL_checkstring(L, -1)));
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_makeDirectories(lua_State* L) {
    minfs_create_directories(luaL_checkstring(L, -1));
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_getCurrentPath(lua_State* L) {
    luaL_Buffer currentpath;
    size_t len = minfs_current_working_directory_len();
    
    luaL_buffinitsize(L, &currentpath, len);
    len = minfs_current_working_directory(currentpath.b, len);
    luaL_pushresultsize(&currentpath, len);
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_fileWithExt(lua_State* L) {
    size_t len, newlen;
    const char* filepath = luaL_checklstring(L, 1, &len);
    luaL_Buffer buffer;
    luaL_buffinitsize(L, &buffer, len);

    if (!minfs_is_file(filepath)) {
        lua_pushnil(L);
        return 1;
    }

    newlen = minfs_path_leaf(filepath, buffer.b, len);
    if (FS_FAILED(newlen)) {
        lua_pushnil(L);
    } else {
        luaL_pushresultsize(&buffer, newlen);
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_pathWithoutExt(lua_State* L) {
    size_t len, newlen;
    const char* filepath = luaL_checklstring(L, 1, &len);
    luaL_Buffer buffer;
    luaL_buffinitsize(L, &buffer, len);

    if (!minfs_is_file(filepath)) {
        lua_pushnil(L);
        return 1;
    }

    newlen = minfs_path_without_ext(filepath, buffer.b, len);
    if (FS_FAILED(newlen)) {
        lua_pushnil(L);
    } else {
        luaL_pushresultsize(&buffer, newlen);
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_pathRoot(lua_State* L) {
    size_t len, newlen;
    const char* filepath = luaL_checklstring(L, 1, &len);
    luaL_Buffer buffer;
    luaL_buffinitsize(L, &buffer, len);

    newlen = minfs_path_parent(filepath, buffer.b, len);
    if (FS_FAILED(newlen)) {
        lua_pushnil(L);
    } else {
        luaL_pushresultsize(&buffer, newlen);
    }
    return 1;
}

static int fs_fileMD5(lua_State* L) {
    size_t len;
    const char* filepath = luaL_checklstring(L, 1, &len);
    cyMD5_CTX md5;
    minfs_uint64_t fsize;
    void* data;
    FILE* f;
    unsigned char digest[CY_MD5_LEN];
    char md5str[CY_MD5_STR_LEN];

    if (!minfs_is_file(filepath)) {
        lua_pushnil(L);
        return 1;
    }

    fsize = minfs_get_file_size(filepath);
    data = lua_newuserdata(L, fsize);

    f = fopen(filepath, "rb");
    fread(data, 1, fsize, f);
    fclose(f);
    cyMD5Init(&md5);
    cyMD5Update(&md5, data, (cyUint)fsize);
    cyMD5Final(&md5, digest);
    cyMD5DigestToString(digest, md5str);

    lua_pushlstring(L, md5str, CY_MD5_STR_LEN-1);
    return 1;
}

static int fs_stringMD5(lua_State* L) {
    size_t len;
    const char* string = luaL_checklstring(L, 1, &len);
    cyMD5_CTX md5;
    unsigned char digest[CY_MD5_LEN];
    char md5str[CY_MD5_STR_LEN];

    cyMD5Init(&md5);
    cyMD5Update(&md5, string, (cyUint)len);
    cyMD5Final(&md5, digest);
    cyMD5DigestToString(digest, md5str);

    lua_pushlstring(L, md5str, CY_MD5_STR_LEN-1);
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef struct readDirData {
    lua_State* L;
    int* tableindex;
    int  stackindex;
} readDirData_t;

void readDirCallback(const char* origpath, const char* file, void* opaque) {
    lua_State* L = ((readDirData_t*)opaque)->L;
    int* tableindex = ((readDirData_t*)opaque)->tableindex;
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    luaL_addstring(&buffer, origpath);
    luaL_addstring(&buffer, "/");
    luaL_addstring(&buffer, file);
    luaL_pushresult(&buffer);
    lua_rawseti(L, -2, (*tableindex)++);
}

static int fs_readDir(lua_State* L) {
    const char* filepath = luaL_checkstring(L, 1);
    char guessstratch[512];
    char* scratch;
    int tableindex;
    size_t scratchsize = sizeof(guessstratch);
    readDirData_t cbdata = {L, &tableindex, 0};

    lua_newtable(L);
    tableindex = 1;
    if (minfs_read_directory(filepath, guessstratch, sizeof(guessstratch), readDirCallback, &cbdata) == NO_MEM) {
        // restart with new table & scratch data
        for (scratch = lua_newuserdata(L, scratchsize); minfs_read_directory(filepath, scratch, scratchsize, readDirCallback, &cbdata) == NO_MEM; ) {
            scratchsize *= 2;
            lua_pop(L, 2);
        }
        // remove the scratch from the stack
        lua_remove(L, -2);
    }
    return 1; // return table 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// the table being written is assumed to be at the top of the stack
void fs_read_entries(lua_State* L, const char* origpath, int* tableindex) {
    char* scratch = NULL;
    size_t scratchsize = LUAL_BUFFERSIZE;
    luaL_Buffer buffer;
    MinFSDirectoryEntry_t* entries;
    
    scratch = realloc(scratch, scratchsize);
    while(!(entries = minfs_read_directory_entries(origpath, scratch, scratchsize))) {
        scratch = realloc(scratch, scratchsize*2);
        scratchsize *= 2;
    }

    for (; entries && entries->entryNameLen != 0; entries = entries->next) {
        luaL_buffinit(L, &buffer);
        luaL_addstring(&buffer, origpath);
        luaL_addstring(&buffer, "/");
        luaL_addstring(&buffer, entries->entryName);
        luaL_pushresult(&buffer);
        if (minfs_is_directory(lua_tostring(L, -1))) {
            lua_pushvalue(L, -2); // the table is assumed to be at the top of the stack
            fs_read_entries(L, lua_tostring(L, -2), tableindex);
            lua_pop(L, 1);
        }
        lua_rawseti(L, -2, (*tableindex)++);
    }

    free(scratch);
}

static int fs_readDirRecursive(lua_State* L) {
    const char* filepath = luaL_checkstring(L, 1);
    int tableindex = 0;

    lua_newtable(L);
    fs_read_entries(L, filepath, &tableindex);
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

//Lua entry point calls
luaFILESYSTEM_EXPORT int luaFILESYSTEM_API luaopen_filesystem(lua_State *L) {
    static const luaL_Reg filesystemlib[] = {
        {"modifieddate",fs_modifiedDate},
        {"filesize",fs_fileSize},
        {"currentfiledate", fs_currentDate},
        {"exists",fs_exists},
        {"isfile",fs_isFile},
        {"isdirectory",fs_isDirectory},
        {"canonical", fs_canonical},
        {"makedirectories",fs_makeDirectories},
        {"getcurrentpath",fs_getCurrentPath},
        {"filewithext", fs_fileWithExt},
        {"pathwithoutext", fs_pathWithoutExt},
        {"parentpath", fs_pathRoot},
        {"readdir", fs_readDir},
        {"readdirrecursive", fs_readDirRecursive},
        {"wildcardpathmatch", fs_wildcardPathMatch},
        {"fileMD5", fs_fileMD5},
        {"stringMD5", fs_stringMD5},
        {NULL, NULL}
    };

    luaL_newlib(L, filesystemlib);
    //lua_setglobal(L, "filesystem");
    return 1;
}
