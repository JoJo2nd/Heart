local infiles=nil
local depfiles=nil
local progparams=nil
local materialpath=nil

--
-- package: COMPLEXMESH2
--
databuilder.addresource{
    package="COMPLEXMESH2",
    resname="HOUSE",
    restype="mesh",
    inputfiles={"src://mesh/house.mdf"},
}