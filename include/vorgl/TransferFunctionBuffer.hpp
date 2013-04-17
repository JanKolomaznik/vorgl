#pragma once

/*#include "MedV4D/Common/Common.h"
#include "MedV4D/GUI/utils/OGLTools.h"
#include "MedV4D/GUI/managers/OpenGLManager.h"*/
#include <GL/glew.h>

#include <boost/shared_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

#include <soglu/OGLTools.hpp>
#include <soglu/CgFXShader.hpp>
#include <cassert>

namespace vorgl
{

struct RGBAf
{
	RGBAf(): r(0.0f), g(0.0f), b(0.0f), a(0.0f)
	{}
	RGBAf(float aR, float aG, float aB, float aA): r(aR), g(aG), b(aB), a(aA)
	{}
	
	template< size_t tCoord >
	struct ValueAccessor
	{
		float &
		operator()( RGBAf & aData ) const
		{
			return (&aData.r)[tCoord];
		}
		float
		operator()( const RGBAf & aData ) const
		{
			return (&aData.r)[tCoord];
		}
	};
	
	float 
	operator[](size_t aIdx)const
	{
		return (&r)[aIdx];
	}
	
	float &
	operator[](size_t aIdx)
	{
		return (&r)[aIdx];
	}
	
	RGBAf operator+(const RGBAf &aArg)const
	{
		return RGBAf(r+aArg.r, g+aArg.g, b+aArg.b, a+aArg.a);
	}
	
	float r, g, b, a;
};
	
class TransferFunctionBuffer1D
{
public:
	typedef boost::shared_ptr< TransferFunctionBuffer1D > Ptr;

	typedef RGBAf 			ValueType;
	typedef ValueType *		Iterator;
	typedef const ValueType*	ConstIterator;

	typedef ValueType 		value_type;
	typedef Iterator 		iterator;
	typedef ConstIterator 		const_iterator;

	typedef glm::fvec2		MappedInterval;

	TransferFunctionBuffer1D( size_t aSize = 0, MappedInterval aMappedInterval = MappedInterval( 0.0f, 1.0f ) );

	~TransferFunctionBuffer1D();


	Iterator
	Begin();

	Iterator
	End();

	ConstIterator
	Begin()const;

	ConstIterator
	End()const;

	size_t
	Size()const;

	void
	Resize( size_t aSize );

	MappedInterval
	GetMappedInterval()const;

	Iterator
	GetNearest( float aValue );
	
	ConstIterator
	GetNearest( float aValue )const;

	int
	GetNearestIndex( float aValue )const;

	ValueType &
	operator[]( size_t aIdx )
	{
		assert( aIdx < mSize );
		return mBuffer[ aIdx ];
	}

	ValueType
	operator[]( size_t aIdx ) const
	{
		assert( aIdx < mSize );
		return mBuffer[ aIdx ];
	}

	void
	SetMappedInterval( MappedInterval aMappedInterval );
protected:
	RGBAf	*mBuffer;
	size_t	mSize;
	MappedInterval mMappedInterval;
private:

};

//struct GLTransferFunctionBuffer1D;

struct GLTransferFunctionBuffer1D
{
	typedef glm::fvec2 MappedInterval;
	typedef boost::shared_ptr< GLTransferFunctionBuffer1D > Ptr;
	typedef boost::shared_ptr< const GLTransferFunctionBuffer1D > ConstPtr;
	typedef boost::weak_ptr< GLTransferFunctionBuffer1D > WPtr;
	typedef boost::weak_ptr< const GLTransferFunctionBuffer1D > ConstWPtr;

	~GLTransferFunctionBuffer1D()
	{
		assert(soglu::isGLContextActive());
		glDeleteTextures(1, &mGLTextureID);
		//OpenGLManager::getInstance()->deleteTextures( mGLTextureID );
	}
	
	friend GLTransferFunctionBuffer1D::Ptr createGLTransferFunctionBuffer1D( const TransferFunctionBuffer1D &aTransferFunction );

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	GLuint
	getTextureID()const
	{ return mGLTextureID; }

	int
	getSampleCount()const
	{
		return mSampleCount;
	}
private:
	GLTransferFunctionBuffer1D( GLuint aGLTextureID, MappedInterval aMappedInterval, int aSampleCount )
		: mGLTextureID( aGLTextureID ), mMappedInterval( aMappedInterval ), mSampleCount( aSampleCount )
	{ /* empty */ }

	GLuint	mGLTextureID;
	MappedInterval mMappedInterval;
	int mSampleCount;
};

GLTransferFunctionBuffer1D::Ptr
createGLTransferFunctionBuffer1D( const TransferFunctionBuffer1D &aTransferFunction );

struct TransferFunctionBufferInfo
{
	/*TransferFunctionBufferInfo( M4D::Common::IDNumber aId, M4D::GUI::GLTransferFunctionBuffer1D::Ptr aTfGLBuffer, M4D::GUI::TransferFunctionBuffer1D::Ptr aTfBuffer ):
		id(aId), tfGLBuffer( aTfGLBuffer ), tfBuffer( aTfBuffer )
	{ }*/
	TransferFunctionBufferInfo():id(0)
	{ }

	/*M4D::Common::IDNumber*/int id;
	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLBuffer;
	vorgl::TransferFunctionBuffer1D::Ptr tfBuffer;

	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLIntegralBuffer;
	vorgl::TransferFunctionBuffer1D::Ptr tfIntegralBuffer;
};

inline void
setCgFXParameter(CGeffect &aEffect, std::string aName, const GLTransferFunctionBuffer1D &aTransferFunction)
{
	//TODO
	{
		CGparameter cgParameter = cgGetNamedEffectParameter(aEffect, (aName + ".data").c_str());
		cgGLSetupSampler( cgParameter, aTransferFunction.getTextureID() );
	}

	soglu::detail::setCgFXParameter(aEffect, aName + ".interval", aTransferFunction.getMappedInterval() );

	soglu::detail::setCgFXParameter(aEffect, aName + ".sampleCount", aTransferFunction.getSampleCount() );
}

} /*vorgl*/
