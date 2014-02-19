
local wx = require "wx"
local wxUtil = require "wxutil"
local luadb = require "luadb"
local filesystem = require "filesystem"

local wxT = wxUtil.text
local function todoMsg() wx.wxMessageBox(wxT([[Implement this feature]]), wxT("TODO..."), wx.wxOK + wx.wxICON_INFORMATION) end
--local defaultCmd = [[cd $(scriptdir) && start $(lua) -lluadb -e "arg={$(args)}; luadb.attach($(port)); db=loadfile($(script)); ok, errstr = xpcall(db, function (msg) print(msg) print(debug.traceback()) end); luadb.dettach();"]]
local defaultCmd = [[cd $(scriptdir) && start $(lua) $(debugscript) $(args)]]
local defaultText = [[
local luadb = require "luadb"
luadb.attach($(port))
db=loadfile($(script))
local ok, errstr = xpcall(db, function (msg) print(msg) print(debug.traceback()) end)
luadb.dettach()
]]
local unknownSourceStr = "[Unknown Source]"

--Persistent values
persistentData = {
    debugCmd = defaultCmd,
    debugText = defaultText,
    debugCmdVars = {
        lua="lua.exe",
        port=8335,
        script="--auto var--",
        hostaddress="127.0.0.1",
        workingdir=".",
        scriptdir="--auto var--",
        debugscript="--auto var--",
        args=""
    },
    breakpoints = {},
    openFiles = {},
    windowSize = {
        width = 800,
        height = 600,
        maximize = true,
    },
    breakpointWindow = {
        width = 200,
        height = 200,    
    },
    perspective = "",
}

local AppIDs = {
    RamuhAppFrame = wxUtil.newID(),
    BreakpointFrame = wxUtil.newID(),
    CallstackFrame = wxUtil.newID(),
    DebugOutputFrame = wxUtil.newID(),
    LocalsFrame = wxUtil.newID(),
    WatchesFrame = wxUtil.newID(),
    DebugStart = wxUtil.newID(),
    DebugRun = wxUtil.newID(),
    DebugStep = wxUtil.newID(),
    DebugStepIn = wxUtil.newID(),
    DebugStepOut = wxUtil.newID(),
    DebugTrap = wxUtil.newID(),
    ToggleBreakpoint = wxUtil.newID(),
    DebugSourceNotebook = wxUtil.newID(),
    GotoLine = wxUtil.newID(),
    ShowBreakpoints = wxUtil.newID(),
    ShowCallstack = wxUtil.newID(),
    ShowLocals = wxUtil.newID(),
    ShowWatches = wxUtil.newID(),
    ShowOutput = wxUtil.newID(),
    ClearOutput = wxUtil.newID(),
    NewWatch = wxUtil.newID(),
    ShowDebugSettings = wxUtil.newID(),
}

local AppFrame = {
    frameID = AppIDs.RamuhAppFrame,

    frame = nil,
    auiMgr = nil,
    dbclient = nil,
    dbfile = nil,
    dbline = nil,
    dbrunning = nil,
    curFile = nil,
    curLine = nil,
}

local monoWidthFont = wx.wxFont(10, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL, false, "Courier New")
local italicMonoWidthFont = wx.wxFont(10, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_ITALIC, wx.wxFONTWEIGHT_NORMAL, false, "Courier New")
-- Markers for editor marker margin
local BREAKPOINT_MARKER_BIT         = 1
local BREAKPOINT_MARKER_VALUE   = 2 -- = 2^BREAKPOINT_MARKER_BIT
local CURRENT_LINE_MARKER_BIT       = 2
local CURRENT_LINE_MARKER_VALUE = 4 -- = 2^CURRENT_LINE_MARKER_BIT

local function findValidFile(infilepath) --TODO!!
    local possiblepathprefixes = {
        nil,
        persistentData.debugCmdVars.scriptdir,
    }
    --[
    for k, v in pairs(possiblepathprefixes) do
        filepath = filesystem.canonical(infilepath, v)
        if filepath and filesystem.isfile(filepath) then return filepath end
    end
    return nil
end

local function serialise(o, file)
    if not file then return end
    if type(o) == "number" then
        file:write(string.format("%a", o))
    elseif type(o) == "string" then
        file:write(string.format("%q", o))
    elseif type(o) == "boolean" then
        file:write(tostring(o))
    elseif type(o) == "table" then
        file:write("{\n")
        for k, v in pairs(o) do
            file:write("["); serialise(k, file); file:write("]=")
            serialise(v, file)
            file:write(",\n")
        end
        file:write("}\n")
    end
end

local function loadTable(filepath) 
    file = io.open(filepath, "rb")
    if file then
        buf = "return " .. file:read("*a")
        file:close()
        chunk, err = load(buf)
        if err then print(err) end
    end
    return chunk
end

local function createMainMenu()
    local menu = wx.wxMenu()
    menu:Append(wx.wxID_OPEN, wxT("&Open\tCtrl+O"))
    menu:AppendSeparator()
    menu:Append(AppIDs.ShowDebugSettings, wxT("Settings..."))
    menu:AppendSeparator()
    menu:Append(wx.wxID_EXIT, wxT("E&xit"))
    return menu
end

local function createEditMenu()
    local menu = wx.wxMenu()
    menu:Append(wx.wxID_FIND, wxT("&Find\tCtrl+F"))
    menu:Append(AppIDs.GotoLine, wxT("&Goto Line\tCtrl+G"))
    return menu
end

local function createViewMenu()
    local menu = wx.wxMenu()
    menu:Append(AppIDs.ShowBreakpoints, wxT("&Breakpoints"))
    menu:Append(AppIDs.ShowOutput, wxT("&Debug Output"))
    menu:Append(AppIDs.ShowCallstack, wxT("&Callstack"))
    menu:Append(AppIDs.ShowLocals, wxT("&Locals"))
    menu:Append(AppIDs.ShowWatches, wxT("&Watches"))
    return menu
end

local function createDebugMenu()
    local menu = wx.wxMenu()
    menu:Append(AppIDs.DebugStart, wxT("&Start Debug...\tCtrl+F5"))
    menu:Append(AppIDs.DebugRun, wxT("&Run\tF5"))
    menu:Append(AppIDs.DebugStep, wxT("&Step\tF10"))
    menu:Append(AppIDs.DebugStepIn, wxT("Step&In\tF11"))
    menu:Append(AppIDs.DebugStepOut, wxT("Step&Out\tShift+F11"))
    menu:Append(AppIDs.DebugTrap, wxT("Trap\tCtrl+F9"))
    menu:AppendSeparator()
    menu:Append(AppIDs.ToggleBreakpoint, wxT("&Toggle Breakpoint\tF9"))
    return menu
end

local function addNotebookPage(notebook, statusbarupdate, infilepath)
    local filetext = ""
    local filepath = findValidFile(infilepath)
    if not filepath then return end
    if filesystem.isfile(filepath) then
        handle = io.open(filepath, "r")
        local buf = handle:read("*a")
        filetext = filetext .. buf
        handle:close()
    end
    --]]
    if not filetext then return end
    
    local stc = wxstc.wxStyledTextCtrl(notebook, wx.wxID_ANY, wx.wxDefaultPosition, wx.wxDefaultSize, wx.wxSUNKEN_BORDER)
    
    -- This chunk of the function is taken from wxLua "editor" app/sample
    stc:SetFont(monoWidthFont)
    stc:StyleSetFont(wxstc.wxSTC_STYLE_DEFAULT, monoWidthFont)
    for i = 0, 32 do stc:StyleSetFont(i, monoWidthFont) end
    stc:StyleSetForeground(0,  wx.wxColour(128, 128, 128)) -- White space
    stc:StyleSetForeground(1,  wx.wxColour(0,   127, 0))   -- Block Comment
    stc:StyleSetFont(1, italicMonoWidthFont)
    --stc:StyleSetUnderline(1, false)
    stc:StyleSetForeground(2,  wx.wxColour(0,   127, 0))   -- Line Comment
    stc:StyleSetFont(2, italicMonoWidthFont)    -- Doc. Comment
    --stc:StyleSetUnderline(2, false)
    stc:StyleSetForeground(3,  wx.wxColour(127, 127, 127)) -- Number
    stc:StyleSetForeground(4,  wx.wxColour(0,   127, 127)) -- Keyword
    stc:StyleSetForeground(5,  wx.wxColour(0,   0,   127)) -- Double quoted string
    stc:StyleSetBold(5,  true)
    --stc:StyleSetUnderline(5, false)
    stc:StyleSetForeground(6,  wx.wxColour(127, 0,   127)) -- Single quoted string
    stc:StyleSetForeground(7,  wx.wxColour(127, 0,   127)) -- not used
    stc:StyleSetForeground(8,  wx.wxColour(0,   127, 127)) -- Literal strings
    stc:StyleSetForeground(9,  wx.wxColour(127, 127, 0))  -- Preprocessor
    stc:StyleSetForeground(10, wx.wxColour(0,   0,   0))   -- Operators
    --stc:StyleSetBold(10, true)
    stc:StyleSetForeground(11, wx.wxColour(0,   0,   0))   -- Identifiers
    stc:StyleSetForeground(12, wx.wxColour(0,   0,   0))   -- Unterminated strings
    stc:StyleSetBackground(12, wx.wxColour(224, 192, 224))
    stc:StyleSetBold(12, true)
    stc:StyleSetEOLFilled(12, true)

    stc:StyleSetForeground(13, wx.wxColour(0,   0,  95))   -- Keyword 2 highlighting styles
    stc:StyleSetForeground(14, wx.wxColour(0,   95, 0))    -- Keyword 3
    stc:StyleSetForeground(15, wx.wxColour(127, 0,  0))    -- Keyword 4
    stc:StyleSetForeground(16, wx.wxColour(127, 0,  95))   -- Keyword 5
    stc:StyleSetForeground(17, wx.wxColour(35,  95, 175))  -- Keyword 6
    stc:StyleSetForeground(18, wx.wxColour(0,   127, 127)) -- Keyword 7
    stc:StyleSetBackground(18, wx.wxColour(240, 255, 255)) -- Keyword 8

    stc:StyleSetForeground(19, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(19, wx.wxColour(224, 255, 255))
    stc:StyleSetForeground(20, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(20, wx.wxColour(192, 255, 255))
    stc:StyleSetForeground(21, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(21, wx.wxColour(176, 255, 255))
    stc:StyleSetForeground(22, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(22, wx.wxColour(160, 255, 255))
    stc:StyleSetForeground(23, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(23, wx.wxColour(144, 255, 255))
    stc:StyleSetForeground(24, wx.wxColour(0,   127, 127))
    stc:StyleSetBackground(24, wx.wxColour(128, 155, 255))

    stc:StyleSetForeground(32, wx.wxColour(224, 192, 224))  -- Line number
    stc:StyleSetBackground(33, wx.wxColour(192, 192, 192))  -- Brace highlight
    stc:StyleSetForeground(34, wx.wxColour(0,   0,   255))
    stc:StyleSetBold(34, true)                              -- Brace incomplete highlight
    stc:StyleSetForeground(35, wx.wxColour(255, 0,   0))
    stc:StyleSetBold(35, true)                              -- Indentation guides
    stc:StyleSetForeground(37, wx.wxColour(192, 192, 192))
    stc:StyleSetBackground(37, wx.wxColour(255, 255, 255))
    
    stc:SetUseTabs(false)
    stc:SetTabWidth(4)
    stc:SetIndent(4)
    stc:SetIndentationGuides(true)
    
    stc:SetLexer(wxstc.wxSTC_LEX_LUA)
    
    stc:SetVisiblePolicy(wxstc.wxSTC_VISIBLE_SLOP, 3)
    stc:SetMarginWidth(0, stc:TextWidth(32, "99999_")) -- line # margin

    stc:SetMarginWidth(1, 16) -- marker margin
    stc:SetMarginType(1, wxstc.wxSTC_MARGIN_SYMBOL)
    stc:SetMarginSensitive(1, true)

    stc:MarkerDefine(BREAKPOINT_MARKER_BIT,   wxstc.wxSTC_MARK_ROUNDRECT, wx.wxWHITE, wx.wxRED)
    stc:MarkerDefine(CURRENT_LINE_MARKER_BIT, wxstc.wxSTC_MARK_ARROW,     wx.wxBLACK, wx.wxGREEN)
    
    stc:SetMarginWidth(2, 16) -- fold margin
    stc:SetMarginType(2, wxstc.wxSTC_MARGIN_SYMBOL)
    stc:SetMarginMask(2, wxstc.wxSTC_MASK_FOLDERS)
    stc:SetMarginSensitive(2, true)

    stc:SetFoldFlags(wxstc.wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED + wxstc.wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED)
    
    stc:SetProperty("fold", "1")
    stc:SetProperty("fold.compact", "1")
    stc:SetProperty("fold.comment", "1")
    
    local grey = wx.wxColour(0, 0, 0)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDEROPEN,    wxstc.wxSTC_MARK_BOXMINUS, wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDER,        wxstc.wxSTC_MARK_BOXPLUS,  wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDERSUB,     wxstc.wxSTC_MARK_VLINE,    wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDERTAIL,    wxstc.wxSTC_MARK_LCORNER,  wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDEREND,     wxstc.wxSTC_MARK_BOXPLUSCONNECTED,  wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDEROPENMID, wxstc.wxSTC_MARK_BOXMINUSCONNECTED, wx.wxWHITE, grey)
    stc:MarkerDefine(wxstc.wxSTC_MARKNUM_FOLDERMIDTAIL, wxstc.wxSTC_MARK_TCORNER,  wx.wxWHITE, grey)
    grey:delete()

    -- Note: these keywords are shamelessly ripped from scite 1.68
    stc:SetKeyWords(0,
        [[and break do else elseif end false for function if
        in local nil not or repeat return then true until while]])
    stc:SetKeyWords(1,
        [[_VERSION assert collectgarbage dofile error gcinfo loadfile loadstring
        print rawget rawset require tonumber tostring type unpack]])
    stc:SetKeyWords(2,
        [[_G getfenv getmetatable ipairs loadlib next pairs pcall
        rawequal setfenv setmetatable xpcall
        string table math coroutine io os debug
        load module select]])
    stc:SetKeyWords(3,
        [[string.byte string.char string.dump string.find string.len
        string.lower string.rep string.sub string.upper string.format string.gfind string.gsub
        table.concat table.foreach table.foreachi table.getn table.sort table.insert table.remove table.setn
        math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.deg math.exp
        math.floor math.frexp math.ldexp math.log math.log10 math.max math.min math.mod
        math.pi math.pow math.rad math.random math.randomseed math.sin math.sqrt math.tan
        string.gmatch string.match string.reverse table.maxn
        math.cosh math.fmod math.modf math.sinh math.tanh math.huge]])
    stc:SetKeyWords(4,
        [[coroutine.create coroutine.resume coroutine.status
        coroutine.wrap coroutine.yield
        io.close io.flush io.input io.lines io.open io.output io.read io.tmpfile io.type io.write
        io.stdin io.stdout io.stderr
        os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename
        os.setlocale os.time os.tmpname
        coroutine.running package.cpath package.loaded package.loadlib package.path
        package.preload package.seeall io.popen
        debug.debug debug.getfenv debug.gethook debug.getinfo debug.getlocal
        debug.getmetatable debug.getregistry debug.getupvalue debug.setfenv
        debug.sethook debug.setlocal debug.setmetatable debug.setupvalue debug.traceback]])
    
    stc:AppendText(filetext)
    
    --[
    local function toggleDebugMarker(line)
        local markers = stc:MarkerGet(line)
        if bit32.band(BREAKPOINT_MARKER_VALUE, markers) == BREAKPOINT_MARKER_VALUE then
            stc:MarkerDelete(line, BREAKPOINT_MARKER_BIT)
            persistentData.breakpoints[string.format("%s?%u", filepath, line)]=nil
        else
            stc:MarkerAdd(line, BREAKPOINT_MARKER_BIT)
            persistentData.breakpoints[string.format("%s?%u", filepath, line)]=true
        end
    end
    --]]
    stc:Connect(wxstc.wxEVT_STC_UPDATEUI,
        function (event)
            if not statusbarupdate then return end
            local pos  = stc:GetCurrentPos()
            local line = stc:LineFromPosition(pos)
            local col  = 1 + pos - stc:PositionFromLine(line)
            statusbarupdate("Line "..tostring(line + 1).." Col "..tostring(col))
        end)
    stc:Connect(wxstc.wxEVT_STC_MARGINCLICK,
        function (event)
            local line = stc:LineFromPosition(event:GetPosition())
            local margin = event:GetMargin()
            if margin == 1 then
                toggleDebugMarker(line)
            elseif margin == 2 then
                local level = stc:GetFoldLevel(line)
                if bit32.band(level, wxstc.wxSTC_FOLDLEVELHEADERFLAG) then
                    stc:ToggleFold(line)
                end
            end
        end)
    
    -- enable any saved breakpoints
    local breakpointkey = "(.+)%?(%d+)"
    for k, v in pairs(persistentData.breakpoints) do
        local file, line = string.match(k, breakpointkey)
        if file == filepath and line then
            line = tonumber(line)
            if bit32.band(BREAKPOINT_MARKER_VALUE, stc:MarkerGet(line)) ~= BREAKPOINT_MARKER_VALUE then
                stc:MarkerAdd(line, BREAKPOINT_MARKER_BIT)
            end
        end
    end
    
    stc:SetReadOnly(true)
    notebook:AddPage(stc, wxT(filepath), true)
    return stc -- Return so it can be added to the list of open documents
end

local function createBreakpointsFrame(parent, breakpointset, breakpointunset)
    local list =  wx.wxListView(parent, 
        AppIDs.BreakpointFrame,
        wx.wxDefaultPosition,
        wx.wxSize(100, 100),
        wx.wxLC_REPORT+wx.wxLC_SORT_ASCENDING)
    list:InsertColumn(0, "Breakpoints", wx.wxLIST_FORMAT_LEFT, wx.wxLIST_AUTOSIZE_USEHEADER)
    
    local function sortfunction(a, b)
        return list:GetItemText(a) < list:GetItemText(b)
    end
    
    -- add all the saved breakpoints
    for k, v in pairs(persistentData.breakpoints) do
        local file, line = string.match(k, "(.+)%?(%d+)")
        list:InsertItem(0, string.format("%s(%d)", file,line+1))
    end
    list:SortItems(sortfunction, 0)
    
    --insert a proxy into the breakpoints table, to catch value assignments
    local realbreakpoints = persistentData.breakpoints
    local _breakpointproxy = {}
    local accessmt = {
        __index = function(t, k) return realbreakpoints[k] end,
        __newindex = function(t, k, v)
            local file, line = string.match(k, "(.+)%?(%d+)")
            local key = string.format("%s(%d)", file,line+1)
            if v == nil then
                local i = list:FindItem(-1, key)
                while i ~= -1 do
                    list:DeleteItem(i)
                    i = list:FindItem(i, key)
                end
                breakpointunset(file, line)
            else
                local i = list:FindItem(-1, key)
                if i == -1 then
                    list:InsertItem(0, key)
                end
                breakpointset(file, line)
            end
            realbreakpoints[k]=v
            list:SortItems(sortfunction, 0)
        end,
        __pairs = function()
            return function(_, k) return next(realbreakpoints, k) end
        end,
        __ipairs = function() assert("not written") end,
    }
    setmetatable(_breakpointproxy, accessmt)
    persistentData.breakpoints = _breakpointproxy
    
    return list
end

local function createCallstackFrame(parent)
    local list =  wx.wxListView(parent, 
        AppIDs.CallstackFrame,
        wx.wxDefaultPosition,
        wx.wxSize(100, 100),
        wx.wxLC_REPORT)
    list:InsertColumn(0, "Callstack", wx.wxLIST_FORMAT_LEFT, wx.wxLIST_AUTOSIZE_USEHEADER)
    return list
end

local function createDebugOutputFrame(parent)
    local frame = wx.wxTextCtrl(parent,
        AppIDs.DebugOutputFrame,
        "",
        wx.wxDefaultPosition,
        wx.wxSize(100, 100),
        wx.wxTE_READONLY+wx.wxTE_MULTILINE+wx.wxTE_RICH+wx.wxHSCROLL)
    return frame
end

local function createLocalsFrame(parent)
    local frame = wx.wxTreeCtrl(parent,
        AppIDs.LocalsFrame,
        wx.wxDefaultPosition,
        wx.wxSize(100, 100),
        wx.wxTR_SINGLE+wx.wxTR_HAS_BUTTONS+wx.wxTR_LINES_AT_ROOT+wx.wxTR_ROW_LINES+wx.wxTR_HIDE_ROOT)
    return frame
end

local function createSettingsFrame(parent)
    local frame = wx.wxPanel(parent, wx.wxID_ANY)
    local exeentry = wx.wxTextCtrl(frame, 
        wx.wxID_ANY,
        persistentData.debugCmdVars.lua,
        wx.wxDefaultPosition,
        wx.wxSize(500, 50))
    local debugscriptentry = wx.wxTextCtrl(frame, 
        wx.wxID_ANY,
        persistentData.debugText, 
        wx.wxDefaultPosition,
        wx.wxSize(500, 175),
        wx.wxTE_MULTILINE+wx.wxTE_DONTWRAP)
    local portentry = wx.wxTextCtrl(frame, 
        wx.wxID_ANY,
        tostring(persistentData.debugCmdVars.port),
        wx.wxDefaultPosition,
        wx.wxDefaultSize,
        0,
        wx.wxGenericValidatorInt(wxlua.wxLuaObject(1)))
    local hostaddressentry = wx.wxTextCtrl(frame, 
        wx.wxID_ANY,
        persistentData.debugCmdVars.hostaddress)
    local argsentry = wx.wxTextCtrl(frame, 
        wx.wxID_ANY,
        persistentData.debugCmdVars.args,
        wx.wxDefaultPosition,
        wx.wxSize(500, 50),
        wx.wxTE_DONTWRAP)
        
    exeentry:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(event)
            persistentData.debugCmdVars.lua=exeentry:GetValue()
        end)
    debugscriptentry:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(event)
            persistentData.debugText=debugscriptentry:GetValue()
        end)
    portentry:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(event)
            persistentData.debugCmdVars.port=tonumber(portentry:GetValue())
        end)
    hostaddressentry:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(event)
            persistentData.debugCmdVars.hostaddress=hostaddressentry:GetValue()
        end)        
    argsentry:Connect(wx.wxEVT_COMMAND_TEXT_UPDATED, function(event)
            persistentData.debugCmdVars.args=argsentry:GetValue()
        end)
    
    local gridsizer = wx.wxFlexGridSizer(5, 2, 4, 4)
    gridsizer:Add(wx.wxStaticText(frame, wx.wxID_ANY, wxT("Lua Exe Path")), wx.wxEXPAND+wx.wxALL)
    gridsizer:Add(exeentry, wx.wxALIGN_RIGHT)
    gridsizer:Add(wx.wxStaticText(frame, wx.wxID_ANY, wxT("Debug Run Script")), wx.wxEXPAND+wx.wxALL)
    gridsizer:Add(debugscriptentry, wx.wxALIGN_RIGHT)
    gridsizer:Add(wx.wxStaticText(frame, wx.wxID_ANY, wxT("Debug Port")), wx.wxEXPAND+wx.wxALL)
    gridsizer:Add(portentry, wx.wxALIGN_RIGHT)
    gridsizer:Add(wx.wxStaticText(frame, wx.wxID_ANY, wxT("Debug Host Address")), wx.wxEXPAND+wx.wxALL)
    gridsizer:Add(hostaddressentry, wx.wxALIGN_RIGHT)
    gridsizer:Add(wx.wxStaticText(frame, wx.wxID_ANY, wxT("Debug Args")), wx.wxEXPAND+wx.wxALL)
    gridsizer:Add(argsentry, wx.wxALIGN_RIGHT)
    frame:SetSizerAndFit(gridsizer)
    return frame
end

local function createWatchesFrame(parent)
    local frame = wx.wxTreeCtrl(parent,
        AppIDs.WatchesFrame,
        wx.wxDefaultPosition,
        wx.wxDefaultSize,
        wx.wxTR_SINGLE+wx.wxTR_HAS_BUTTONS+wx.wxTR_LINES_AT_ROOT+wx.wxTR_ROW_LINES+wx.wxTR_HIDE_ROOT)
    return frame
end

function AppFrame:create(parent, id, title, pos, size, style)
    local frame = wx.wxFrame(wx.NULL,
                        self.frameID,
                        wxUtil.text("Ramuh - Visual Lua debugger"),
                        wx.wxDefaultPosition,
                        wx.wxSize(persistentData.windowSize.width, persistentData.windowSize.height))
    persistentData.windowSize.width, persistentData.windowSize.height = frame:GetClientSizeWH()
    frame:Maximize(persistentData.windowSize.maximize)
    persistentData.windowSize.maximize = frame:IsMaximized()
    local openDocuments = {}
    local statusBar = frame:CreateStatusBar( 3 )
    local statusBarUpdate = function (txt) frame:SetStatusText(txt, 1) end
    --// tell wxAuiManager to manage this frame
    local auiMgr = wxaui.wxAuiManager()
    auiMgr:SetManagedWindow(frame);
    
    local menubar = wx.wxMenuBar()
    menubar:Append(createMainMenu(), wxT("&File"))
    menubar:Append(createEditMenu(), wxT("&Edit"))
    menubar:Append(createViewMenu(), wxT("&View"))
    menubar:Append(createDebugMenu(), wxT("&Debug"))
    
    frame:SetMenuBar(menubar)
    
    local notebookstyle = wxaui.wxAUI_NB_DEFAULT_STYLE + wxaui.wxAUI_NB_TAB_EXTERNAL_MOVE + wx.wxNO_BORDER
    local notebook = wxaui.wxAuiNotebook(frame, AppIDs.DebugSourceNotebook, wx.wxPoint(persistentData.windowSize.width, persistentData.windowSize.height), wx.wxSize(persistentData.windowSize.width, persistentData.windowSize.height), notebookstyle)
    auiMgr:AddPane(notebook, wxaui.wxAuiPaneInfo():Name(wxT("open_documents")):CenterPane():PaneBorder(false))
    
    local function clearCurrentLineMarker()
        if self.curLine ~= nil and self.curFile ~= nil then
            local doc=openDocuments[self.curFile]
            if doc ~= nil then 
                doc:MarkerDelete(self.curLine-1, CURRENT_LINE_MARKER_BIT)
            end
        end
        self.curLine = nil
        self.curFile = nil
    end
    local function setCurrentLineMarker(file, line)
        file = findValidFile(file)
        if file == self.curLine and line == self.curLine then 
            return
        end
        clearCurrentLineMarker()
        --update cursor (-1 on cursor update because text lines start at 0, Lua lines start at 1)
        local doc=nil
        file = findValidFile(file)
        --add new
        doc=openDocuments[file]
        if doc == nil then 
            self:openDocumentSilent(file) 
            doc=openDocuments[file]
        end
        if doc ~= nil then 
            notebook:SetSelection(notebook:GetPageIndex(doc))
            doc:MarkerAdd(line-1, CURRENT_LINE_MARKER_BIT)
            doc:GotoLine(line-1)
        end
        -- update
        self.curFile = file
        self.curLine = line
    end
    
    local breakpointslist = createBreakpointsFrame(frame, 
        function(file, line) 
            if self.dbclient ~= nil then self.dbclient:setbreakpoint(file, line+1) end
        end,
        function(file, line) 
            if self.dbclient ~= nil then self.dbclient:removebreakpoint(file, line+1) end
        end)
    auiMgr:AddPane(breakpointslist, wxaui.wxAuiPaneInfo():
        Name(wxT("breakpoints")):
        Right():
        Dockable(true):
        Floatable(true):
        PaneBorder(true):
        Caption("Breakpoints"))
    
    local debugoutput = createDebugOutputFrame(frame)
    auiMgr:AddPane(debugoutput, wxaui.wxAuiPaneInfo():
        Name(wxT("debug_output")):
        Bottom():
        Dockable(true):
        Floatable(true):
        PaneBorder(true):
        Caption("Debug Output"))
        
    local callstack = createCallstackFrame(frame)
    auiMgr:AddPane(callstack, wxaui.wxAuiPaneInfo():
        Name(wxT("callstack")):
        Float():
        Dockable(true):
        Floatable(true):
        PaneBorder(true):
        Caption("Callstack"))
        
    local localsview = createLocalsFrame(frame)
    auiMgr:AddPane(localsview, wxaui.wxAuiPaneInfo():
        Name(wxT("locals_view")):
        Float():
        Dockable(true):
        Floatable(true):
        PaneBorder(true):
        Caption("Local Variables"))    
    
    local watchesview = createWatchesFrame(frame)
    auiMgr:AddPane(watchesview, wxaui.wxAuiPaneInfo():
        Name(wxT("watches_view")):
        Float():
        Dockable(true):
        Floatable(true):
        PaneBorder(true):
        Caption("Variable Watch Window"))   

    local settingsview = createSettingsFrame(frame)
    auiMgr:AddPane(settingsview, wxaui.wxAuiPaneInfo():
        Name(wxT("settings_view")):
        Float():
        Dockable(false):
        Floatable(true):
        PaneBorder(true):
        Caption("Settings"))   
    
    --// "commit" all changes made to wxAuiManager
    auiMgr:Update();
    
    timer = wx.wxTimer(frame)

    local function gotoFileLine(file,line)
        if openDocuments[file] == nil then
            self:openDocumentSilent(file)
        end
        if openDocuments[file] then
            notebook:SetSelection(notebook:GetPageIndex(openDocuments[file]))
            openDocuments[file]:GotoLine(line-1)
        end
    end
    --//
    frame:Connect(wx.wxEVT_TIMER, function(event) self:tick() end)
    frame:Connect(wx.wxEVT_DESTROY,
        function(event)
            if (event:GetEventObject():DynamicCast("wxObject") == frame:DynamicCast("wxObject")) then
                timer:Stop()
                timer:delete()
                persistentData.windowSize.width, persistentData.windowSize.height = frame:GetClientSizeWH()
                persistentData.windowSize.maximize = frame:IsMaximized()
                persistentData.perspective = auiMgr:SavePerspective()
                file = io.open("ramuh_config", "wb")
                serialise(persistentData, file)
                file:close()
                -- You must ALWAYS UnInit() the wxAuiManager when closing
                -- since it pushes event handlers into the frame.
                auiMgr:UnInit()
            end
        end)
    frame:Connect(wx.wxID_OPEN, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            local fileDialog = wx.wxFileDialog(frame, "Open file", "", "", "Lua files (*.lua)|*.lua|Text files (*.txt)|*.txt|All files (*)|*", wx.wxFD_OPEN + wx.wxFD_FILE_MUST_EXIST)
            if fileDialog:ShowModal() == wx.wxID_OK then
                local filepath = filesystem.genericpath(fileDialog:GetPath())
                persistentData.openFiles[filepath] = false
                if not openDocuments[filepath] then
                    openDocuments[filepath] = addNotebookPage(notebook, statusBarUpdate, filepath)
                end
                if openDocuments[filepath] then
                    notebook:SetSelection(notebook:GetPageIndex(openDocuments[filepath]))
                    persistentData.openFiles[filepath] = true
                else
                    wx.wxMessageBox(wxT("Couldn't open file "..filepath, wxT("Error"), wx.wxOK))
                end
            end
            fileDialog:Destroy()
        end)
    frame:Connect(AppIDs.DebugSourceNotebook, wxaui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, 
        function(event)
            openDocuments[notebook:GetPageText(event:GetSelection())] = nil
            persistentData.openFiles[notebook:GetPageText(event:GetSelection())] = nil
        end)
    frame:Connect(wx.wxID_EXIT, wx.wxEVT_COMMAND_MENU_SELECTED, function (event) frame:Close() end )
    frame:Connect(wx.wxID_FIND, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.GotoLine, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if notebook:GetSelection() < 0 then return end
            local doc=openDocuments[notebook:GetPageText(notebook:GetSelection())]
            if doc then
                local entry=wx.wxTextEntryDialog(frame, wxT("Goto line..."), wxT("Goto line..."), "1")
                entry:SetValidator(wx.wxGenericValidatorInt(wxlua.wxLuaObject(1)))
                if entry:ShowModal() ~= wx.wxID_OK then return end
                doc:GotoLine(tonumber(entry:GetValue()))
            end
        end)
    frame:Connect(AppIDs.DebugStart, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if notebook:GetSelection() < 0 or self.dbclient ~= nil then return end
            local filename = findValidFile(notebook:GetPageText(notebook:GetSelection()))
            persistentData.debugCmdVars.script = "[["..filename.."]]"
            persistentData.debugCmdVars.scriptdir = filesystem.parentpath(filename)
            persistentData.debugCmdVars.debugscript = persistentData.debugCmdVars.scriptdir.."/ramuh_debug.tmp.lua"
            print ("script",persistentData.debugCmdVars.script)
            print ("script dir",persistentData.debugCmdVars.scriptdir)
            print ("debug script",persistentData.debugCmdVars.debugscript)
            local file=io.open(persistentData.debugCmdVars.debugscript, "w")
            local str=string.gsub(persistentData.debugText, "$%((%w+)%)", persistentData.debugCmdVars)
            file:write(str)
            file:close()
            local cmd = string.gsub(persistentData.debugCmd, "$%((%w+)%)", persistentData.debugCmdVars)
            print(cmd)
            local startok = os.execute(cmd)
            --[
            if startok then
                self.dbfile = nil
                self.dbline = nil
                self.dbrunning = nil
                self.dbclient = luadb.newclient()
                self.dbclient:setdebugmsg(function(str) debugoutput:AppendText(str) end)
                self.frame:SetStatusText(string.format("Debugger connecting to %s, %d",persistentData.debugCmdVars.hostaddress,persistentData.debugCmdVars.port), 0)
                self.dbclient:connect(persistentData.debugCmdVars.hostaddress, persistentData.debugCmdVars.port)
                if self.dbclient:isconnected() then
                    self.dbclient:addsearchpath(persistentData.debugCmdVars.scriptdir)
                    for k, v in pairs(persistentData.breakpoints) do
                        local file, line = string.match(k, "(.+)%?(%d+)")
                        self.dbclient:setbreakpoint(file, line+1)
                    end
                end
            end
        end)
    frame:Connect(AppIDs.DebugRun, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if self.dbclient and not self.dbclient:isrunning() then
                self.dbclient:run()
            end
        end)   
    frame:Connect(AppIDs.DebugStep, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if self.dbclient and not self.dbclient:isrunning() then
                self.dbclient:step()
            end
        end)
    frame:Connect(AppIDs.DebugStepIn, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if self.dbclient and not self.dbclient:isrunning() then
                self.dbclient:stepin()
            end
        end)
    frame:Connect(AppIDs.DebugStepOut, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if self.dbclient and not self.dbclient:isrunning() then
                self.dbclient:stepout()
            end
        end)
    frame:Connect(AppIDs.DebugTrap, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            if self.dbclient and self.dbclient:isrunning() then
                self.dbclient:trap()
            end
        end)
    frame:Connect(AppIDs.ToggleBreakpoint, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.ShowBreakpoints, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            auiMgr:GetPane(breakpointslist):Show(true)
            auiMgr:Update()
        end)
    frame:Connect(AppIDs.ShowOutput, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            auiMgr:GetPane(debugoutput):Show(true)
            auiMgr:Update()
        end)
    frame:Connect(AppIDs.ShowCallstack, wx.wxEVT_COMMAND_MENU_SELECTED,
        function(event)
            auiMgr:GetPane(callstack):Show(true)
            auiMgr:Update()
        end)
    frame:Connect(AppIDs.ShowLocals, wx.wxEVT_COMMAND_MENU_SELECTED,
        function(event)
            auiMgr:GetPane(localsview):Show(true)
            auiMgr:Update()
        end)       
    frame:Connect(AppIDs.ShowWatches, wx.wxEVT_COMMAND_MENU_SELECTED,
        function(event)
            auiMgr:GetPane(watchesview):Show(true)
            auiMgr:Update()
        end)
    frame:Connect(AppIDs.ShowDebugSettings, wx.wxEVT_COMMAND_MENU_SELECTED,
        function(event)
            auiMgr:GetPane(settingsview):Show(true)
            auiMgr:Update()
        end)
    frame:Connect(AppIDs.BreakpointFrame, wx.wxEVT_COMMAND_LIST_ITEM_ACTIVATED, 
        function(event)
            local file, line = string.match(event:GetItem():GetText(), "(.+)%((%d+)%)")
            gotoFileLine(file, line)
        end)
    frame:Connect(AppIDs.CallstackFrame, wx.wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
        function(event)
            local stacklevel, file, line = string.match(event:GetItem():GetText(), "(%d+):(.*):(%-?%d+)(.*)")
            print("active stacklevel = ", stacklevel)
            if file ~= unknownSourceStr and line ~= -1 then
                gotoFileLine(file, line)
                setCurrentLineMarker(file, line)
            end
            self:UpdateLocals(stacklevel-1)
            self:updateWatches(stacklevel-1)
        end)
    local onVarTreeItemExpand = function(event, treectrl)
        if not self.dbclient then return end
        local label = treectrl:GetItemText(treectrl:GetFirstChild(event:GetItem()))
        print(label)
        entry = string.match(label, "Expand:(%d+)")
        print(entry)
        if entry then
            info = self.dbclient:getmoreinfo(entry)
            treectrl:Delete(treectrl:GetFirstChild(event:GetItem()))
            for k,value in pairs(info.fields) do
                if value.typename == "table" then
                    itemtext = string.format("%s [Type=%s]", value.printablekey, value.typename)
                else
                    itemtext = string.format("%s = %s [Type=%s]", value.printablekey, value.printablevalue, value.typename)
                end
                local treeitem = treectrl:AppendItem(event:GetItem(), wxT(itemtext))
                if value.typename == "table" then
                    treectrl:AppendItem(treeitem, string.format("Expand:%s", value.entryindex))
                end
            end
        end    
    end
    frame:Connect(AppIDs.LocalsFrame, wx.wxEVT_COMMAND_TREE_ITEM_EXPANDING,
        function(event)
            onVarTreeItemExpand(event, localsview)
        end)
    frame:Connect(AppIDs.LocalsFrame, wx.wxEVT_COMMAND_TREE_ITEM_RIGHT_CLICK,
        function(event)
            print("Item right click")
        end)
    watchesview:Connect(wx.wxEVT_RIGHT_DOWN, 
        function(event)
            local menu = wx.wxMenu()
            menu:Append(AppIDs.NewWatch, wxT("Add New Watch..."))
            watchesview:Connect(AppIDs.NewWatch, wx.wxEVT_COMMAND_MENU_SELECTED, 
                function(event)
                    local entry=wx.wxTextEntryDialog(watchesview, wxT("Enter Variable Name"), wxT("Enter name"))
                    if entry:ShowModal() ~= wx.wxID_OK then return end
                    self:addWatch(entry:GetValue())
                end)
            watchesview:PopupMenu(menu)
        end)
    frame:Connect(AppIDs.WatchesFrame, wx.wxEVT_COMMAND_TREE_ITEM_EXPANDING,
        function(event)
            onVarTreeItemExpand(event, localsview)
        end)

    self.frame = frame
    self.auiMgr = auiMgr
    self.watchList = {}
    self.openDocuments = openDocuments
    self.openDocumentSilent = function (self, filepath)
        persistentData.openFiles[filepath] = false
        if not openDocuments[filepath] then
            openDocuments[filepath] = addNotebookPage(notebook, statusBarUpdate, filepath)
        end
        if openDocuments[filepath] then
            persistentData.openFiles[filepath] = true
            notebook:SetSelection(notebook:GetPageIndex(openDocuments[filepath]))
        end
    end
    self.updateCallstackView = function(self)
        if self.dbclient == nil and callstack:GetItemCount() > 0 then
            clearCurrentLineMarker()
            callstack:DeleteAllItems()
            self:UpdateLocals(0)
            self:updateWatches(0)
        elseif self.dbclient == nil or not self.dbclient:isconnected() then
            return
        elseif self.dbclient:isrunning() and self.dbbacktrace ~= nil then
            callstack:DeleteAllItems()
            callstack:InsertItem(0, wxT("Debugger running..."))
            self.dbbacktrace = nil
            self:UpdateLocals(0)
            self:updateWatches(0)
        elseif not self.dbclient:isrunning() and self.dbbacktrace == nil then
            local setline = false
            clearCurrentLineMarker()
            callstack:DeleteAllItems()
            self.dbbacktrace = self.dbclient:backtrace()
            for i, v in ipairs(self.dbclient:backtrace()) do
                if v.source ~= "" then
                    if not setline then
                        setCurrentLineMarker(v.source, v.currentline)
                        setline = true
                    end
                    callstack:InsertItem(i-1, wxT(string.format("%d:%s:%d: %s(%s)", i,  findValidFile(v.source), v.currentline, v.name, v.what)))
                else
                    callstack:InsertItem(i-1, wxT(string.format("%d:%s:%d: %s(%s)", i,  unknownSourceStr, v.currentline, v.name, v.what)))
                end
            end
            self:UpdateLocals(0)
            self:updateWatches(0)
        end
    end
    self.UpdateLocals = function(self, stacklevel)
        if not self.dbclient or self.dbclient:isrunning() then
            localsview:DeleteAllItems()
            return
        end
        self.dblocals = self.dbclient:getlocals(stacklevel)
        localsview:DeleteAllItems()
        local root = localsview:AddRoot(wxT("Locals"))
        for k, v in pairs(self.dblocals) do
            local itemtext
            if v.typename == "table" then
                itemtext = string.format("%s [Type=%s]", v.name, v.typename)
            else
                itemtext = string.format("%s = %s [Type=%s]", v.name, v.printablevalue, v.typename)
            end
            local item = localsview:AppendItem(root, wxT(itemtext))
            if v.fields then
                for field, value in pairs(v.fields) do
                    if value.typename == "table" then
                        itemtext = string.format("%s [Type=%s]", value.printablekey, value.typename)
                    else
                        itemtext = string.format("%s = %s [Type=%s]", value.printablekey, value.printablevalue, value.typename)
                    end
                    local treeitem = localsview:AppendItem(item, wxT(itemtext))
                    if value.typename == "table" then
                        localsview:AppendItem(treeitem, string.format("Expand:%s", value.entryindex))
                    end
                end
            end
        end
    end
    self.addWatch = function(self, watchname)
        self.watchList[watchname]=true
        self:updateWatches(0)
    end
    self.updateWatches = function(self, stacklevel)
        watchesview:DeleteAllItems()
        local root=watchesview:AddRoot(wxT("Watches"));
        local watch=nil
        for k, v in pairs(self.watchList) do
            if not self.dbclient or self.dbclient:isrunning() then
                watchesview:AppendItem(root, wxT("Cannot find variable \""..k.."\""))
            else
                watch=self.dbclient:getwatch(k, stacklevel)
                if not watch then
                    watchesview:AppendItem(root, wxT("Cannot find variable \""..k.."\""))
                else
                    local itemtext
                    if watch.typename == "table" then
                        itemtext = string.format("%s [Type=%s]", watch.name, watch.typename)
                    else
                        itemtext = string.format("%s = %s [Type=%s]", watch.name, watch.printablevalue, watch.typename)
                    end
                    local item = watchesview:AppendItem(root, wxT(itemtext))
                    if watch.fields then
                        for field, value in pairs(watch.fields) do
                            if value.typename == "table" then
                                itemtext = string.format("%s [Type=%s]", value.printablekey, value.typename)
                            else
                                itemtext = string.format("%s = %s [Type=%s]", value.printablekey, value.printablevalue, value.typename)
                            end
                            local treeitem = watchesview:AppendItem(item, wxT(itemtext))
                            if value.typename == "table" then
                                watchesview:AppendItem(treeitem, string.format("Expand:%s", value.entryindex))
                            end
                        end
                    end
                end
            end
        end
    end
    
    
    self.tick = function(self)
        self.frame:SetStatusText("No dbclient", 2)
        if self.dbclient then
            if not self.dbclient:isconnected() then
                self.dbfile = nil
                self.dbline = nil
                self.dbrunning = nil
                self.dbclient = nil
                self:updateCallstackView()
            --[[ Don't attempt reconnect, just reset debugger state
                self.frame:SetStatusText(string.format("Debugger connecting to %s, %d",persistentData.debugCmdVars.hostaddress,persistentData.debugCmdVars.port), 0)
                self.dbclient:connect(persistentData.debugCmdVars.hostaddress, persistentData.debugCmdVars.port)
                if self.dbclient:isconnected() then
                    self.dbclient:addsearchpath(persistentData.debugCmdVars.scriptdir)
                    for k, v in pairs(persistentData.breakpoints) do
                        local file, line = string.match(k, "(.+)%?(%d+)")
                        self.dbclient:setbreakpoint(file, line+1)
                    end
                end
            --]]
            else
                self.frame:SetStatusText(string.format("dbclient c:%s r:%s", tostring(self.dbclient:isconnected()), tostring(self.dbclient:isrunning())), 2)
                if self.dbclient:isconnected() then
                    self:updateCallstackView()
                    if self.dbclient:isrunning() then
                        self.frame:SetStatusText("Debugger Running", 0)
                    else
                        local file, line = self.dbclient:getcurrentfileline()
                        if file ~= nil and file ~= "" and line ~= nil and line < 1000000 and line >= 0 then
                            file = findValidFile(file)
                            self.frame:SetStatusText(string.format("Debugger Stopped %s(%d)", file, line), 0)
                        else
                            self.frame:SetStatusText("Debugger Stopped (Unknown Location)", 0)
                        end
                    end
                end
                self.dbclient:tick(0)
            end
        else
            self.frame:SetStatusText("No debug session active", 0)
        end
        
        timer:Start(16, wx.wxTIMER_ONE_SHOT)
    end
    
    auiMgr:LoadPerspective(persistentData.perspective)
    
    self:updateCallstackView()
    -- tick() every 16ms
    timer:Start(16, wx.wxTIMER_ONE_SHOT)
    return self
end

for k, v in pairs(filesystem.readdir("C:/dev/heart_lua")) do
    print(v)
end
print("=====")
for k, v in pairs(filesystem.readdirrecursive("C:/dev/heart_lua")) do
    print(v)
end
--[[
for i,v in pairs(wx) do
    if string.find(i, "wxEVT_") then
        print(i)
    end
end
--]]


local cfg = loadTable("ramuh_config")
if cfg then 
    local data = cfg()
    if data then
        for k, v in pairs(data) do
            persistentData[k] = v
        end
    end
end

local myframe = AppFrame:create()
myframe.frame:Show()

for k, v in pairs(persistentData.openFiles) do    
    myframe:openDocumentSilent(k)
end

-- Call wx.wxGetApp():MainLoop() last to start the wxWidgets event loop,
-- otherwise the wxLua program will exit immediately.
-- Does nothing if running from wxLua, wxLuaFreeze, or wxLuaEdit since the
-- MainLoop is already running or will be started by the C++ program.
wx.wxGetApp():MainLoop()