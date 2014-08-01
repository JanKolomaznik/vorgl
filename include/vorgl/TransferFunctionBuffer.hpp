#pragma once

/*#include "MedV4D/Common/Common.h"
#include "MedV4D/GUI/utils/OGLTools.h"
#include "MedV4D/GUI/managers/OpenGLManager.h"*/

#if defined _WIN64 || defined _WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif //NOMINMAX
#	include <windows.h>
#	undef near
#	undef far
#endif

#include <GL/gl.h>

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>

#include <soglu/GLSLShader.hpp>

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

class TransferFunctionBuffer1D: public std::vector<RGBAf>
{
public:
	typedef std::shared_ptr< TransferFunctionBuffer1D > Ptr;

	typedef RGBAf 			ValueType;
	typedef std::vector<ValueType>	Buffer;

	typedef Buffer::iterator		Iterator;
	typedef Buffer::const_iterator	ConstIterator;

	typedef ValueType 		value_type;
	typedef Iterator 		iterator;
	typedef ConstIterator 		const_iterator;

	typedef glm::fvec2		MappedInterval;



	TransferFunctionBuffer1D( size_t aSize = 0, MappedInterval aMappedInterval = MappedInterval( 0.0f, 1.0f ) ) : Buffer(aSize), mMappedInterval(aMappedInterval)
	{ /*empty*/ }

	~TransferFunctionBuffer1D()
	{ /*empty*/ }


	/*Iterator
	begin();

	Iterator
	end();

	ConstIterator
	begin()const;

	ConstIterator
	end()const;

	size_t
	size()const
	{ return mBuffer.size(); }

	void
	resize(size_t aSize)
	{ mBuffer.resize(); }*/

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	Iterator
	GetNearest( float aValue );

	ConstIterator
	GetNearest( float aValue )const;

	int
	GetNearestIndex( float aValue )const;

	/*ValueType &
	operator[]( size_t aIdx )
	{
		assert( aIdx < mSize );
		return mBuffer[ aIdx ];
	}

	ValueType
	operator[]( size_t aIdx ) const
	{
		assert( aIdx < mBuffer.size() );
		return mBuffer[ aIdx ];
	}*/

	void
	setMappedInterval( MappedInterval aMappedInterval );
protected:
	//std::vector<RGBAf> mBuffer;
	MappedInterval mMappedInterval;
private:

};

//struct GLTransferFunctionBuffer1D;

struct GLTransferFunctionBuffer1D
{
	typedef glm::fvec2 MappedInterval;
	typedef std::shared_ptr< GLTransferFunctionBuffer1D > Ptr;
	typedef std::shared_ptr< const GLTransferFunctionBuffer1D > ConstPtr;
	typedef std::weak_ptr< GLTransferFunctionBuffer1D > WPtr;
	typedef std::weak_ptr< const GLTransferFunctionBuffer1D > ConstWPtr;

	~GLTransferFunctionBuffer1D()
	{
		assert(soglu::isGLContextActive());
		glDeleteTextures(1, &(mGLTextureID.value));
		//OpenGLManager::getInstance()->deleteTextures( mGLTextureID );
	}

	friend GLTransferFunctionBuffer1D::Ptr createGLTransferFunctionBuffer1D( const TransferFunctionBuffer1D &aTransferFunction );

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	soglu::TextureId
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

	soglu::TextureId mGLTextureID;
	MappedInterval mMappedInterval;
	int mSampleCount;
};

GLTransferFunctionBuffer1D::Ptr
createGLTransferFunctionBuffer1D(const TransferFunctionBuffer1D &aTransferFunction);

struct TransferFunctionBufferInfo
{
	/*TransferFunctionBufferInfo( M4D::Common::IDNumber aId, M4D::GUI::GLTransferFunctionBuffer1D::Ptr aTfGLBuffer, M4D::GUI::TransferFunctionBuffer1D::Ptr aTfBuffer ):
		id(aId), tfGLBuffer( aTfGLBuffer ), tfBuffer( aTfBuffer )
	{ }*/
	TransferFunctionBufferInfo():id(-1)
	{ }

	/*M4D::Common::IDNumber*/int id;
	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLBuffer;
	vorgl::TransferFunctionBuffer1D::Ptr tfBuffer;

	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLIntegralBuffer;
	vorgl::TransferFunctionBuffer1D::Ptr tfIntegralBuffer;
};


inline void
setUniform(soglu::GLSLProgram &aProgram, const std::string &aUniformName, const vorgl::GLTransferFunctionBuffer1D &aTransferFunction, soglu::TextureUnitId aTextureUnit)
{
	soglu::gl::bindTexture(aTextureUnit, soglu::TextureTarget::Texture1D, aTransferFunction.getTextureID());
	aProgram.setUniformByName(aUniformName + ".data", aTextureUnit);
	aProgram.setUniformByName(aUniformName + ".interval", aTransferFunction.getMappedInterval() );
	aProgram.setUniformByName(aUniformName + ".sampleCount", aTransferFunction.getSampleCount() );
}

} /*vorgl*/

