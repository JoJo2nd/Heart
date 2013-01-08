
SlnName = "NewTestBed"
BinType = "game"
DebugSuffix = "_d"
ReleaseSuffix = "_r"
heartBuildRoot=os.getenv("HEART_BUILD_ROOT")
heartRepoRoot=os.getenv("HEART_REPO_ROOT")
heartBinToolRoot=os.getenv("HEART_BIN_TOOL_ROOT")
heartBinGameRoot=os.getenv("HEART_BIN_GAME_ROOT")
heartBinRoot=os.getenv("HEART_BIN_ROOT")
heartProjectRoot="../project_scripts/"
heartProjectCommonRoot="../project_common/"

dofile (heartProjectCommonRoot.."heart_common_proj.lua")

solution (SlnName)
    location (SlnOutput)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    libdirs {LibDirs}
    
    configuration (DebugCfgName)
        targetsuffix(DebugSuffix)
        
    configuration (ReleaseCfgName)
        targetsuffix(ReleaseSuffix)
    
    dofile (heartProjectRoot.."testbed.proj.lua") --1st project becomes the start up project
    dofile (heartProjectCommonRoot.."include_heart_projects.lua")


