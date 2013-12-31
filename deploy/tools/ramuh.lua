
wx = require "wx"
wxUtil = require "wxutil"

local wxT = wxUtil.text
local function todoMsg() wx.wxMessageBox(wxT([[Implement this feature]]), wxT("TODO..."), wx.wxOK + wx.wxICON_INFORMATION) end

local AppIDs = {
    RamuhAppFrame = wxUtil.newID(),
    DebugRun = wxUtil.newID(),
    DebugStep = wxUtil.newID(),
    DebugStepIn = wxUtil.newID(),
    DebugStepOut = wxUtil.newID(),
    ToggleBreakpoint = wxUtil.newID(),
    DebugSourceNotebook = wxUtil.newID(),
    GotoLine = wxUtil.newID(),
}

local AppFrame = {
    frameID = AppIDs.RamuhAppFrame,

    frame = nil,
    auiMgr = nil,
}

local monoWidthFont = wx.wxFont(10, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_NORMAL, wx.wxFONTWEIGHT_NORMAL, false, "Courier New")
local italicMonoWidthFont = wx.wxFont(10, wx.wxFONTFAMILY_MODERN, wx.wxFONTSTYLE_ITALIC, wx.wxFONTWEIGHT_NORMAL, false, "Courier New")

local function createMainMenu()
    local menu = wx.wxMenu()
    menu:Append(wx.wxID_OPEN, wxT("&Open\tCtrl+O"))
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

local function createDebugMenu()
    local menu = wx.wxMenu()
    menu:Append(AppIDs.DebugRun, wxT("&Run\tF5"))
    menu:Append(AppIDs.DebugStep, wxT("&Step\tF10"))
    menu:Append(AppIDs.DebugStepIn, wxT("Step&In\tF11"))
    menu:Append(AppIDs.DebugStepOut, wxT("Step&Out\tShift+F11"))
    menu:AppendSeparator()
    menu:Append(AppIDs.ToggleBreakpoint, wxT("&Toggle Breakpoint\tF9"))
    return menu
end

local function addNotebookPage(notebook, statusbarupdate, filepath)
    -- add a test page
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
    
    
    local filetext = "Some Dummy Text..."
    --[
    local handle = io.open(filepath, "rb")
    if handle then
        filetext = handle:read("*a")
        handle:close()
    else
        return
    end
    --]]
    stc:AppendText(filetext)
    
    stc:Connect(wxstc.wxEVT_STC_UPDATEUI,
        function (event)
            if not statusbarupdate then return end
            local pos  = stc:GetCurrentPos()
            local line = stc:LineFromPosition(pos)
            local col  = 1 + pos - stc:PositionFromLine(line)

            statusbarupdate("Line "..tostring(line + 1).." Col "..tostring(col))
        end)
    
    stc:SetReadOnly(true)
    notebook:AddPage(stc, wxT(filepath), true)
    return stc -- Return so it can be added to the list of open documents
end

function AppFrame:create(parent, id, title, pos, size, style)
    local frame = wx.wxFrame(wx.NULL,
                        self.frameID,
                        wxUtil.text("Ramuh - Visual Lua debugger"),
                        wx.wxDefaultPosition,
                        wx.wxSize(800, 600))
    local w, h = frame:GetClientSizeWH()
    local openDocuments = {}
    local statusBar = frame:CreateStatusBar( 2 )
    local statusBarUpdate = function (txt) frame:SetStatusText(txt, 1) end
    --// tell wxAuiManager to manage this frame
    local auiMgr = wxaui.wxAuiManager()
    auiMgr:SetManagedWindow(frame);
    
    local menubar = wx.wxMenuBar()
    menubar:Append(createMainMenu(), wxT("&File"))
    menubar:Append(createEditMenu(), wxT("&Edit"))
    menubar:Append(createDebugMenu(), wxT("&Debug"))
    
    frame:SetMenuBar(menubar)
    
    local notebookstyle = wxaui.wxAUI_NB_DEFAULT_STYLE + wxaui.wxAUI_NB_TAB_EXTERNAL_MOVE + wx.wxNO_BORDER
    local notebook = wxaui.wxAuiNotebook(frame, AppIDs.DebugSourceNotebook, wx.wxPoint(w, h), wx.wxSize(430,200), notebookstyle)
    auiMgr:AddPane(notebook, wxaui.wxAuiPaneInfo():Name(wxT("open_documents")):CenterPane():PaneBorder(false))
    
    --// "commit" all changes made to wxAuiManager
    auiMgr:Update();

    frame:Connect(wx.wxEVT_DESTROY,
        function(event)
            if (event:GetEventObject():DynamicCast("wxObject") == frame:DynamicCast("wxObject")) then
                -- You must ALWAYS UnInit() the wxAuiManager when closing
                -- since it pushes event handlers into the frame.
                auiMgr:UnInit()
            end
        end)
    frame:Connect(wx.wxID_OPEN, wx.wxEVT_COMMAND_MENU_SELECTED, 
        function(event)
            local fileDialog = wx.wxFileDialog(frame, "Open file", "", "", "Lua files (*.lua)|*.lua|Text files (*.txt)|*.txt|All files (*)|*", wx.wxFD_OPEN + wx.wxFD_FILE_MUST_EXIST)
            if fileDialog:ShowModal() == wx.wxID_OK then
                local filepath = fileDialog:GetPath()
                if not openDocuments[filepath] then
                    openDocuments[filepath] = addNotebookPage(notebook, statusBarUpdate, filepath)
                end
                if openDocuments[filepath] then
                    notebook:SetSelection(notebook:GetPageIndex(openDocuments[filepath]))
                else
                    wx.wxMessageBox(wxT("Couldn't open file "..filepath, wxT("Error"), wx.wxOK))
                end
            end
            fileDialog:Destroy()
        end)
    frame:Connect(AppIDs.DebugSourceNotebook, wxaui.wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE, 
        function(event)
            openDocuments[notebook:GetPageText(event:GetSelection())] = nil
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
    frame:Connect(AppIDs.DebugRun, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.DebugStep, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.DebugStepIn, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.DebugStepOut, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)
    frame:Connect(AppIDs.ToggleBreakpoint, wx.wxEVT_COMMAND_MENU_SELECTED, todoMsg)

    self.frame = frame
    self.auiMgr = auiMgr
    self.openDocuments = openDocuments
    return self
end
--[[
for i,v in pairs(wxaui) do
    if string.find(i, "CLOSE") then
        print(i)
    end
end
--]]
local myframe = AppFrame:create()
myframe.frame:Show()

-- Call wx.wxGetApp():MainLoop() last to start the wxWidgets event loop,
-- otherwise the wxLua program will exit immediately.
-- Does nothing if running from wxLua, wxLuaFreeze, or wxLuaEdit since the
-- MainLoop is already running or will be started by the C++ program.
wx.wxGetApp():MainLoop()