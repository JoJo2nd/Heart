/********************************************************************
	created:	2008/08/01
	created:	1:8:2008   19:25
	filename: 	ResourceBuilder.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef ResourceBuilder_h__
#define ResourceBuilder_h__

#include <string>
#include <map>
#include <list>
#include "PackCommon.h"
#include <wchar.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "huCRC32.h"
#include <shlwapi.h>

class ArchiveBuilder;
class ResourceBuilder;
class ResourceFileSystem;

template< typename _Ty >
class ResourceDataPtr
{
public:
	ResourceDataPtr() :
		pParent_( NULL ),
		DataOffset_( 0 )
	{

	}
	~ResourceDataPtr()
	{

	}

	_Ty& operator [] ( const hUint32 i );
	operator _Ty* ();
	_Ty* operator -> ();

	_Ty&				GetElement( const hUint32 i );

	ResourceBuilder*	pParent_;
	hUint32				DataOffset_;
};

class ResourceBuilder
{
public:

	typedef std::list< GameResource* > DependencyList;

	ResourceBuilder()
	{
		HadError_ = hFalse;
		HadWarnings_ = hFalse;
		ErrorString_ = "No Error";
		WarningString_ = "No Warnings"; 
		pDataBuffer_ = NULL;
		pResourceFileSystem_ = NULL;
		paramHash_ = 0;
		inputKey_ = 0;
	}
	virtual										~ResourceBuilder();

	virtual hBool								BuildResource() = 0;
	virtual hUint32								GetDataSize() = 0;
	virtual hByte*								GetDataPtr() = 0;
	virtual void								SetInputFilename( const std::string& filename )
	{
		inputFilename_ = filename;
		UpdateParameterHash();
	}
	virtual void								SetResourceName( const std::string& resourceName )
	{
		resourceName_ = resourceName;
		UpdateParameterHash();
	}
	virtual const std::string&					GetResourceName() const
	{
		return resourceName_;
	}
	virtual void								SetResourcePath( const std::string& resourcePath ) 
	{
		resourcePath_ = resourcePath;
		UpdateParameterHash();
	}
	virtual void								SetResourceDependenciesPath( const std::string& depsPath )
	{
		resourceDepsPath_ = depsPath;
	}
	virtual const std::string&					GetResourceDependenciesPath() const
	{
		return resourceDepsPath_;
	}
	const std::string&							GetErrorString() const;
	void										ThrowFatalError( const char* error, ... );
	const std::string&							GetWarningString() const;
	void										SetWarningString( const char* error, ... );
	hBool										HadError() const;
	hBool										HadWarnings() const;
	void										SetParam( const std::string& name, const std::string& value );
	const char*									GetParam( const char* name );
	void*										GetWorkingDataBuffer() const { return pDataBuffer_; }
	void										SetResourceFileSystem( ResourceFileSystem* pFileSys ) { pResourceFileSystem_ = pFileSys; }
	DependencyList&								GetDependencies() { return dependencies_; }
	hUint32										GetParamHash() { return paramHash_; }
	hUint32										GetInputKey() { UpdateInputKey(); return inputKey_; }
	void										SetOutputFilesDirectory( const char* dir ) { outputFilesDirectory_ = dir; }

protected:

	friend class ArchiveBuilder;

	typedef std::map< std::string, std::string >		ParamMap;
	typedef std::list< std::string >					DepList;

	hUint32							setBufferSize( hUint32 offsettobuffer, hUint32 buffersize );
	hUint32							addDataToBuffer( const void* data, hUint32 sizeinbytes );
	template< typename _Ty >
	_Ty*							getDataFromBuffer( hUint32 sizeinbytes, ResourceDataPtr< _Ty >& ptr );						
	void							freeDataBuffer();

	ResourceFileSystem*				pResourceFileSystem() { hcAssertMsg( pResourceFileSystem_, "Resource File System not set" ); return pResourceFileSystem_; }
	GameResource*					AddDependency( const std::string& name, const std::string& inputfilename, const std::string& dll, hUint32& depResId );
	void							UpdateParameterHash();
	void							UpdateInputKey();

	std::string						outputFilesDirectory_;
	std::string						inputFilename_;
	std::string						resourceName_;
	std::string						resourcePath_;
	std::string						resourceDepsPath_;
	ParamMap						myParams_;

	// these make life easier when building the resource data [11/26/2008 James]
	hUint32							OffsetToBuffer_;
	hByte*							pDataBuffer_;
	hUint32							BufferSize_;
	hUint32							BufferOffset_;

private:

	std::string						ErrorString_;
	std::string						WarningString_;
	DepList							DepList_;
	hBool							HadError_;
	hBool							HadWarnings_;
	ResourceFileSystem*				pResourceFileSystem_;
	DependencyList					dependencies_;
	hUint32							paramHash_;
	hUint32							inputKey_;
};

inline ResourceBuilder::~ResourceBuilder()
{
	if ( pDataBuffer_ )
	{
		delete[] pDataBuffer_;
	}
}

inline const std::string& ResourceBuilder::GetErrorString() const
{
	return ErrorString_;
}

inline const std::string& ResourceBuilder::GetWarningString() const
{
	return WarningString_;
}

inline void ResourceBuilder::ThrowFatalError( const char* error, ... )
{
	char buf[ 4096 ];
	va_list args;
	va_start( args, error );
	vsprintf( buf, error, args );
	va_end( args );

	if ( ErrorString_ == "No Error" )
	{
		ErrorString_ = buf;
	}
	else
	{
		ErrorString_ += buf;
	}

	ErrorString_ += "\n";
	HadError_ = hTrue;

	// throw except for builder to catch
	throw std::exception(ErrorString_.c_str());
}

inline void ResourceBuilder::SetWarningString( const char* error, ... )
{
	char buf[ 4096 ];
	va_list args;
	va_start( args, error );
	vsprintf( buf, error, args );
	va_end( args );

	if ( WarningString_ == "No Warnings" )
	{
		WarningString_ = buf;
	}
	else
	{
		WarningString_ += buf;
	}

	WarningString_ += "\n";
	HadWarnings_ = hTrue;
}

inline hBool ResourceBuilder::HadError() const 
{
	return HadError_;
}

inline hBool ResourceBuilder::HadWarnings() const
{
	return HadWarnings_;
}

inline void ResourceBuilder::SetParam( const std::string& name, const std::string& value )
{
	if ( myParams_[ name ] != "" )
	{
		ThrowFatalError( "Tried to set the same param value twice" );
		return;
	}

	myParams_[ name ] = value;

	UpdateParameterHash();
}

inline const char* ResourceBuilder::GetParam( const char* name )
{
	return myParams_[ name ].c_str();
}

inline hUint32 ResourceBuilder::setBufferSize( hUint32 offsettobuffer, hUint32 buffersize )
{
	OffsetToBuffer_ = offsettobuffer;
	BufferOffset_ = 0;
	BufferSize_ = buffersize;
	pDataBuffer_ = new hByte[ BufferSize_ ];

	return BufferSize_;
}

inline hUint32 ResourceBuilder::addDataToBuffer( const void* data, hUint32 sizeinbytes )
{
	if ( !pDataBuffer_ )
	{
		setBufferSize( 0, sizeinbytes );
	}

	if ( BufferOffset_ + sizeinbytes > BufferSize_ )
	{
		//double
		BufferSize_ = ( BufferSize_ + sizeinbytes ) << 1;
		hcAssert( (BufferOffset_ + sizeinbytes) <= BufferSize_ );
		hByte* ptmp = new hByte[ BufferSize_ ];
		memcpy( ptmp, pDataBuffer_, BufferOffset_ );

		delete[] pDataBuffer_;
		pDataBuffer_ = ptmp;
	}

	hUint32 ret = BufferOffset_;

	memcpy( pDataBuffer_ + BufferOffset_, data, sizeinbytes );

	BufferOffset_ += sizeinbytes;

	hcAssert( BufferOffset_ <= BufferSize_ );

	return OffsetToBuffer_ + ret;
}

inline GameResource* ResourceBuilder::AddDependency( const std::string& name, const std::string& depinputfilename, const std::string& dll, hUint32& depResId )
{
	GameResource* newRes = new GameResource;
	std::string finaldepinputfilename = depinputfilename;

	if ( PathIsRelative( depinputfilename.c_str() ) )
	{
		//Path is relative to resource, so we make it relative to gamedata folder
		char wd[MAX_PATH];
		char final[MAX_PATH];
		strcpy_s( final, MAX_PATH, inputFilename_.c_str() ); 
		
		//remove file name e.g. "bleh.txt"
		PathRemoveFileSpec( final );
		//build the file path
		PathCombine( wd, final, depinputfilename.c_str() );
		//resolve and remove .. & . elements of the path
		PathCanonicalize( final, wd );

		if ( PathFileExists( final ) )
		{
			// if the file exists then change the depinputfilename
			// if not its possible that the path is a "memory file"
			finaldepinputfilename = final;
		}
	}

	newRes->name_ = name;
	newRes->outputPath_ = GetResourceDependenciesPath() + "\\" + name;
	newRes->inputResourcePath_ = finaldepinputfilename;
	newRes->dllName_ = dll;
	newRes->depPath_ = GetResourceDependenciesPath() + "\\" + name + "_dep";

	newRes->buildDLLInfo_ = NULL;
	newRes->pBuilder_ = NULL;

	std::string path = newRes->outputPath_;
	hUint32 pos;
	while( ( pos = path.find_first_of( '\\' ) ) < path.size() )
	{
		path[ pos ] = '/';
	}

	depResId = huCRC32::StringCRC( path.c_str() );

	dependencies_.push_back( newRes );

	return newRes;
}

template< typename _Ty >
_Ty* ResourceBuilder::getDataFromBuffer( hUint32 sizeinbytes, ResourceDataPtr< _Ty >& ptr )
{
	if ( !pDataBuffer_ )
	{
		setBufferSize( 0, sizeinbytes );
	}

	if ( BufferOffset_ + sizeinbytes > BufferSize_ )
	{
		//double
		BufferSize_ = ( BufferSize_ + sizeinbytes ) << 1;
		hcAssert( (BufferOffset_ + sizeinbytes) <= BufferSize_ );
		hByte* ptmp = new hByte[ BufferSize_ ];
		memset( ptmp, 0xCD, BufferSize_ );
		memcpy( ptmp, pDataBuffer_, BufferOffset_ );

		delete[] pDataBuffer_;
		pDataBuffer_ = ptmp;
	}

	hUint32 ret = BufferOffset_;

	ptr.pParent_ = this;
	ptr.DataOffset_ = BufferOffset_;

	BufferOffset_ += sizeinbytes;

	hcAssert( BufferOffset_ <= BufferSize_ );

	return (_Ty*)(OffsetToBuffer_ + ret);
}

inline void ResourceBuilder::freeDataBuffer()
{
	delete[] pDataBuffer_;
	pDataBuffer_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void ResourceBuilder::UpdateParameterHash()
{
	std::string combine;
	combine += inputFilename_;
	combine += resourcePath_;
	combine += resourceName_;
	for ( ParamMap::iterator i = myParams_.begin(), e = myParams_.end(); i != e; ++i )
	{
		combine += i->first;
		combine += i->second;
	}

	paramHash_ = huCRC32::StringCRC( combine.c_str() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename _Ty >
_Ty& ResourceDataPtr<_Ty>::operator[]( const hUint32 i )
{
	hcAssert( pParent_ );
	return ((_Ty*)( (hByte*)pParent_->GetWorkingDataBuffer() + DataOffset_ ))[ i ];
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename _Ty >
_Ty* ResourceDataPtr<_Ty>::operator->()
{
	hcAssert( pParent_ );
	return (_Ty*)( (hByte*)pParent_->GetWorkingDataBuffer() + DataOffset_ ); 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename _Ty >
ResourceDataPtr<_Ty>::operator _Ty*()
{
	hcAssert( pParent_ );
	return (_Ty*)( (hByte*)pParent_->GetWorkingDataBuffer() + DataOffset_ );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template< typename _Ty >
_Ty& ResourceDataPtr<_Ty>::GetElement( const hUint32 i )
{
	hcAssert( pParent_ );
	return ((_Ty*)( (hByte*)pParent_->GetWorkingDataBuffer() + DataOffset_ ))[ i ];
}

typedef ResourceBuilder* (*ResourceCreator)();
typedef void (*ResourceDestroyer)(ResourceBuilder*);

#endif // ResourceBuilder_h__