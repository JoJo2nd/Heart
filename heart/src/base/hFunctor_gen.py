#=============================================
# Functor Generator
# Author: Neil Richardson, Modified by James Moran
#=============================================

#=============================================
# Generate a seperated string based on a template.
# template could be "typename _P%NUM%" or something similar.
def genSeperatedString( range, template, seperator ):
    outString = ""
    for each in range:
        if each != 0:
            outString = outString + seperator
        outString = outString + template.replace( "%NUM%", str( each ) )
    return outString

#=============================================
# Generate a hFuncTraits class
def hFuncTraits_header( outFile ):
    outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
    outFile.write( "// hFuncTraits\n\n" )
    outFile.write( "template< typename _Fn >\n" )
    outFile.write( "struct hFuncTraits;\n\n" )

def hFuncTraits_gen( outFile, numParams, isClass ):
    outString = ""
    if isClass == 0:
        templateList = genSeperatedString( range( 0, numParams ), ", typename _P%NUM%", "" )
        paramList = genSeperatedString( range( 0, numParams ), "_P%NUM%", ", " )
        typedefList = genSeperatedString( range( 0, numParams ), "\ttypedef _P%NUM% param%NUM%_type;\n", "" )
        outString += "template< typename _R" + templateList + " >\n"
        outString += "struct hFuncTraits< _R(*)(" + paramList + ") >\n"
        outString += "{\n"
        outString += "  static const int PARAMS = " + str( numParams ) + ";\n"
        outString += "  typedef _R return_type;\n"
        outString += typedefList
        outString += "  typedef _R(*signature_type)(" + paramList + ");\n"
        outString += "};\n\n"
    else:
        templateList = genSeperatedString( range( 0, numParams ), ", typename _P%NUM%", "" )
        paramList = genSeperatedString( range( 0, numParams ), "_P%NUM%", ", " )
        typedefList = genSeperatedString( range( 0, numParams ), "\ttypedef _P%NUM% param%NUM%_type;\n", "" )
        outString += "template< typename _Ty, typename _R" + templateList + " >\n"
        outString += "struct hFuncTraits< _R(_Ty::*)(" + paramList + ") >\n"
        outString += "{\n"
        outString += "  static const int PARAMS = " + str( numParams ) + ";\n"
        outString += "  typedef _Ty class_type;\n"
        outString += "  typedef _R return_type;\n"
        outString += typedefList
        outString += "  typedef _R(*signature_type)(" + paramList + ");\n"
        outString += "};\n\n"
    outFile.write( outString )
    
#=============================================
# Generate the functor.
def hFunctor_header( outFile ):
    outFile.write( "//////////////////////////////////////////////////////////////////////////\n" )
    outFile.write( "// _hFunctor\n\n" )
    outFile.write( "template< typename _Fn, int >" )
    outFile.write( "class _hFunctor;" )

def hFunctor_gen( outFile, numParams ):
    outString = ""
    typedefList = genSeperatedString( range( 0, numParams ), "  typedef typename hFuncTraits< _Fn >::param%NUM%_type param%NUM%_type;\n", "" )
    paramList = genSeperatedString( range( 0, numParams ), "typename hFuncTraits< _Fn >::param%NUM%_type P%NUM%", ", " )
    paramTypeList = genSeperatedString( range( 0, numParams ), "typename hFuncTraits< _Fn >::param%NUM%_type", ", " )
    paramCallList = genSeperatedString( range( 0, numParams ), "P%NUM%", ", " )
    outString += "template< typename _Fn >\n"
    outString += "class _hFunctor< _Fn, " + str( numParams ) + " >\n"
    outString += "{\n"
    outString += "public:\n"
    outString += "  typedef typename hFuncTraits< _Fn >::return_type return_type;\n"
    outString += typedefList
    if numParams == 0:
        outString += "  typedef typename hFuncTraits< _Fn >::return_type(*stub_func)(void*);\n"
    else:
        outString += "  typedef typename hFuncTraits< _Fn >::return_type(*stub_func)(void*,"+paramTypeList+");\n"
    outString += "public:\n"
    
    outString += "  inline _hFunctor(): pThis_( NULL ), stubFunc_( NULL ){};\n"
    
    
    outString += "  inline return_type operator()(" + paramList + ")\n"
    outString += "  {\n"
    outString += "      hcAssert( stubFunc_ != NULL );\n"
    if numParams == 0:
        outString += "      return (*stubFunc_)(pThis_);\n"
    else:
        outString += "      return (*stubFunc_)(pThis_, " + paramCallList + ");\n"
    outString += "  }\n\n"
    
    outString += "  inline hBool operator == (const _hFunctor& rhs) const \n"
    outString += "  {\n"
    outString += "    return stubFunc_ == rhs.stubFunc_ && pThis_ == rhs.pThis_;\n"
    outString += "  }\n"
    
    outString += "  inline hBool isValid() const\n"
    outString += "  {\n"
    outString += "      return ( stubFunc_ != NULL );\n"
    outString += "  }\n\n"
    
    outString += "  template< _Fn _func >\n"
    outString += "  static _hFunctor< _Fn, " + str( numParams ) + " > bind()\n"
    outString += "  {\n"
    outString += "      _hFunctor< _Fn, " + str( numParams ) + " > Func;\n"
    outString += "      Func.pThis_ = NULL;\n"
    outString += "      Func.stubFunc_ = &global_stub< _func >;\n"
    outString += "      return Func;\n"
    outString += "  }\n\n"
    
    outString += "  template< class _Ty, return_type(_Ty::*_func)("+paramTypeList+") >\n"
    outString += "  static _hFunctor< _Fn, " + str( numParams ) + " > bind( _Ty* pThis )\n"
    outString += "  {\n"
    outString += "      _hFunctor< _Fn, " + str( numParams ) + " > Func;\n"
    outString += "      Func.pThis_ = pThis;\n"
    outString += "      Func.stubFunc_ = &method_stub< _Ty, _func >;\n"
    outString += "      hcAssert( pThis != NULL );\n"
    outString += "      return Func;\n"
    outString += "  }\n\n"

    outString += "private:\n"
    outString += "  template< _Fn func >\n"
    if numParams == 0:
        outString += "  static return_type global_stub( void* )\n"
    else:
        outString += "  static return_type global_stub( void*, " + paramList + " )\n"
    outString += "  {\n"
    outString += "      return (*func)( " + paramCallList + " );\n"
    outString += "  }\n\n"

    outString += "  template< class _Ty, return_type (_Ty::*meth)("+paramTypeList+") >\n"
    if numParams == 0:
        outString += "  static return_type method_stub( void* pObj )\n"
    else:
        outString += "  static return_type method_stub( void* pObj, " + paramList + " )\n"
    outString += "  {\n"
    outString += "      _Ty* pThis = static_cast< _Ty* >( pObj );\n"
    outString += "      return (pThis->*meth)( " + paramCallList + " );\n"
    outString += "  }\n\n"
    outString += "private:\n"
    outString += "  void* pThis_;\n"
    outString += "  stub_func stubFunc_;\n"
    outString += "};\n\n"
    outFile.write( outString )
    
def hFunctor_footer( outFile ):
    outString = ""
    outString += "template< typename _Fn >\n"
    outString += "struct hFunctor\n"
    outString += "{\n"
    outString += "  typedef _hFunctor< _Fn, hFuncTraits< _Fn >::PARAMS > type;\n"
    outString += "};\n";
    outFile.write( outString )

#=============================================
# Generate the functor file.
outFile = open( "hFunctor.h", "w+" )

outString = ""
outString += "//----------------------------------------------------------------------------\n"
outString += "//\n"
outString += "//  Title     :   hFunctor.h\n"
outString += "//  Desc      :   Function object.\n"
outString += "//  Author        :   Neil Richardson, James Moran & Autogeneration Script\n"
outString += "//\n"
outString += "//  Usage       :\n"
outString += "//\n"
outString += "//    These are used much like function pointers, but allow classes to register\n"
outString += "//    their methods, without any of the horrid syntax. To declare a functor, first\n"
outString += "//    it is best to typedef hFunctor:\n"
outString += "//\n"
outString += "//    typedef hFunctor< void(*)( int ) >::type TMyFunctor;\n"
outString += "//\n"
outString += "//    This functor will take functions with a \"void function(int a)\" signature.\n"
outString += "//    Finally, an example of usage:\n"
outString += "//\n"
outString += "//    extern void myTestFunc( int a );\n"
outString += "//\n"
outString += "//    TMyFunctor myFunctor;\n"
outString += "//\n"
outString += "//    myFunctor = TMyFunctor::bind< myTestFunc > ();      // Bind myTestFunc to the functor.\n"
outString += "//    myFunctor( 1 );                                     // Call the functor, redirects to myTestFunc\n"
outString += "//\n"
outString += "//    class MyTestClass\n"
outString += "//    {\n"
outString += "//        void myTestMethod( int a )\n"
outString += "//    };\n"
outString += "//\n"
outString += "//    MyTestClass ClassInstance;\n"
outString += "//\n"
outString += "//    // Bind myTestMethod for ClassInstance to myFunctor. This removes previous binding.\n"
outString += "//    myFunctor = TMyFunctor::bind< MyTestClass, &MyTestClass::myTestMethod >( &ClassInstance );\n"
outString += "//\n"
outString += "//----------------------------------------------------------------------------\n"
outString += "#ifndef __hFUNCTOR_H__\n"
outString += "#define __hFUNCTOR_H__\n\n"
outString += "\n\n"
outString += "namespace Heart\n{\n\n"
#outString += "#include \"hTypes.h\"\n\n"
#outString += "#include \"hDebugMacros.h\"\n\n"
outFile.write( outString )

numParams = 9

hFuncTraits_header( outFile )
for each in range( 0, numParams ):
    hFuncTraits_gen( outFile, each, 0 )
for each in range( 0, numParams ):
    hFuncTraits_gen( outFile, each, 1 )

hFunctor_header( outFile )
for each in range( 0, numParams ):
    hFunctor_gen( outFile, each )

hFunctor_footer( outFile )

outString = "\n"
outString +=  "} //namespace Heart\n"
outString += "\n"
outString += "#if defined (HEART_PLAT_WINDOWS)\n"
outString += "#    define hFUNCTOR_TYPEDEF(ty, tydef) typedef Heart::hFunctor< ##ty >::type tydef\n"
outString += "#elif defined (HEART_PLAT_LINUX)\n"
outString += "#    define hFUNCTOR_TYPEDEF(ty, tydef) typedef Heart::hFunctor< ty >::type tydef\n"
outString += "#endif\n"
outString += "#define hFUNCTOR_BINDSTATIC(fnr, proc) fnr::bind< proc >()\n"
outString += "#define hFUNCTOR_BINDMEMBER(fnr, klass, proc, th1s) fnr::bind< klass, &klass::proc >(th1s)\n"
outString += "\n"
outString += "#endif\n"
outFile.write( outString )

print "hFunctor.h written"