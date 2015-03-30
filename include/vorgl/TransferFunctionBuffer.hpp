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
#include <boost/variant.hpp>

namespace vorgl
{

struct RGBAf
{
	RGBAf(): r(0.0f), g(0.0f), b(0.0f), a(0.0f)
	{}
	RGBAf(float aR, float aG, float aB, float aA): r(aR), g(aG), b(aB), a(aA)
	{}

	RGBAf(const float *aBuffer): r(aBuffer[0]), g(aBuffer[1]), b(aBuffer[2]), a(aBuffer[3])
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

inline RGBAf operator*(const RGBAf &aArg, float aMultiplier)
{
	return RGBAf(aMultiplier * aArg.r, aMultiplier * aArg.g, aMultiplier * aArg.b, aMultiplier * aArg.a);
}


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

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	Iterator
	GetNearest( float aValue );

	ConstIterator
	GetNearest( float aValue )const;

	int
	GetNearestIndex( float aValue )const;

	void
	setMappedInterval( MappedInterval aMappedInterval );

	MappedInterval
	mappedInterval() const
	{
		return mMappedInterval;
	}
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
		/*assert(soglu::isGLContextActive());
		glDeleteTextures(1, &(mGLTextureID.value));*/
		//OpenGLManager::getInstance()->deleteTextures( mGLTextureID );
	}

	friend GLTransferFunctionBuffer1D::Ptr createGLTransferFunctionBuffer1D( const TransferFunctionBuffer1D &aTransferFunction );

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	soglu::TextureId
	getTextureID()const
	{ return mTexture; }

	int
	getSampleCount()const
	{
		return mSampleCount;
	}
private:
	GLTransferFunctionBuffer1D(soglu::TextureObject &&aTexture, MappedInterval aMappedInterval, int aSampleCount )
		: mTexture(std::move(aTexture))
		, mMappedInterval( aMappedInterval )
		, mSampleCount( aSampleCount )
	{ /* empty */ }

	//soglu::TextureId mGLTextureID;
	soglu::TextureObject mTexture;
	MappedInterval mMappedInterval;
	int mSampleCount;
};


struct GLTransferFunctionBuffer2D
{
	typedef std::pair<glm::fvec2, glm::fvec2> MappedInterval;
	typedef std::pair<int, int> Resolution;
	typedef std::shared_ptr< GLTransferFunctionBuffer2D > Ptr;
	typedef std::shared_ptr< const GLTransferFunctionBuffer2D > ConstPtr;
	typedef std::weak_ptr< GLTransferFunctionBuffer2D > WPtr;
  typedef std::weak_ptr< const GLTransferFunctionBuffer2D > ConstWPtr;
  typedef std::array<float, 3> EigenvalueProcessingParameters;

	~GLTransferFunctionBuffer2D()
	{
		/*assert(soglu::isGLContextActive());
		glDeleteTextures(1, &(mGLTextureID.value));*/
		//OpenGLManager::getInstance()->deleteTextures( mGLTextureID );
	}

  friend GLTransferFunctionBuffer2D::Ptr
    createGLTransferFunctionBuffer2D(
    const RGBAf *aData,
    int aWidth,
    int aHeight,
    glm::fvec2 aFrom,
    glm::fvec2 aTo,
    bool eigenvalueProcessPrimary,
    bool eigenvalueProcessSecondary,
    std::array<float, 3> primaryProcessingParameters,
    int primaryValuesMultiplier,
    std::array<float, 3> secondaryProcessingParameters,
    int secondaryValuesMultiplier
    );

	MappedInterval
	getMappedInterval()const
	{ return mMappedInterval; }

	soglu::TextureId
	getTextureID()const
	{ return mTexture; }

	Resolution
	getResolution()const
	{
		return mResolution;
	}

  bool
  getEigenvalueProcessPrimary() const 
  {
    return this->eigenvalueProcessPrimary;
  }

  bool
  getEigenvalueProcessSecondary() const
  {
    return this->eigenvalueProcessSecondary;
  }

  glm::fvec3
  getPrimaryParameters() const
  {
    return this->eigenvaluePrimaryParameters;
  }

  glm::int32
  getPrimaryValuesMultiplier() const
  {
    return this->primaryValuesMultiplier;
  }

  glm::fvec3
  getSecondaryParameters() const
  {
    return this->eigenvalueSecondaryParameters;
  }

  glm::int32
  getSecondaryValuesMultiplier() const
  {
    return this->secondaryValuesMultiplier;
  }

private:
  GLTransferFunctionBuffer2D(soglu::TextureObject &&aTexture, MappedInterval aMappedInterval, Resolution aResolution, bool eigenvalueProcessPrimary, bool eigenvalueProcessSecondary,
    EigenvalueProcessingParameters primaryProcessingParameters, int primaryValuesMultiplier, EigenvalueProcessingParameters secondaryProcessingParameters, int secondaryValuesMultiplier)
		: mTexture(std::move(aTexture))
		, mMappedInterval( aMappedInterval )
		, mResolution( aResolution )
    , eigenvalueProcessPrimary(eigenvalueProcessPrimary)
    , eigenvalueProcessSecondary(eigenvalueProcessSecondary)
    , primaryValuesMultiplier(primaryValuesMultiplier)
    , secondaryValuesMultiplier(secondaryValuesMultiplier)
	{
    this->eigenvaluePrimaryParameters.r = primaryProcessingParameters[0];
    this->eigenvaluePrimaryParameters.g = primaryProcessingParameters[1];
    this->eigenvaluePrimaryParameters.b = primaryProcessingParameters[2];

    this->eigenvalueSecondaryParameters.r = primaryProcessingParameters[0];
    this->eigenvalueSecondaryParameters.g = primaryProcessingParameters[1];
    this->eigenvalueSecondaryParameters.b = primaryProcessingParameters[2];
  }

	//soglu::TextureId mGLTextureID;
	soglu::TextureObject mTexture;
	MappedInterval mMappedInterval;
	Resolution mResolution;

  glm::fvec3 eigenvaluePrimaryParameters;
  glm::int32 primaryValuesMultiplier;
  glm::fvec3 eigenvalueSecondaryParameters;
  glm::int32 secondaryValuesMultiplier;

  bool eigenvalueProcessPrimary = false;
  bool eigenvalueProcessSecondary = false;
};


GLTransferFunctionBuffer1D::Ptr
createGLTransferFunctionBuffer1D(const TransferFunctionBuffer1D &aTransferFunction);

GLTransferFunctionBuffer2D::Ptr
createGLTransferFunctionBuffer2D(
  const RGBAf *aData,
  int aWidth,
  int aHeight,
  glm::fvec2 aFrom,
  glm::fvec2 aTo,
  bool eigenvalueProcessPrimary,
  bool eigenvalueProcessSecondary,
  glm::fvec3 primaryProcessingParameters,
  glm::int32 primaryValuesMultiplier,
  glm::fvec3 secondaryProcessingParameters,
  glm::int32 secondaryValuesMultiplier
);

struct TransferFunctionBuffer1DInfo {
	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLBuffer;
	//vorgl::TransferFunctionBuffer1D::Ptr tfBuffer;

	vorgl::GLTransferFunctionBuffer1D::Ptr tfGLIntegralBuffer;
	//vorgl::TransferFunctionBuffer1D::Ptr tfIntegralBuffer;
};

struct TransferFunctionBuffer2DInfo {
	vorgl::GLTransferFunctionBuffer2D::Ptr tfGLBuffer;
};

typedef boost::variant<TransferFunctionBuffer1DInfo, TransferFunctionBuffer2DInfo> TFBufferInfoVariant;

struct TransferFunctionBufferInfo
{
	TransferFunctionBufferInfo():id(-1)
	{ }

	int id;
	TFBufferInfoVariant bufferInfo;
};


inline void
setUniform(soglu::GLSLProgram &aProgram, const std::string &aUniformName, const vorgl::GLTransferFunctionBuffer1D &aTransferFunction, soglu::TextureUnitId aTextureUnit)
{
	soglu::gl::bindTexture(aTextureUnit, soglu::TextureTarget::Texture1D, aTransferFunction.getTextureID());
	aProgram.setUniformByName(aUniformName + ".data", aTextureUnit);
	aProgram.setUniformByName(aUniformName + ".interval", aTransferFunction.getMappedInterval() );
	aProgram.setUniformByName(aUniformName + ".sampleCount", aTransferFunction.getSampleCount() );
}

inline void
setUniform(soglu::GLSLProgram &aProgram, const std::string &aUniformName, const vorgl::GLTransferFunctionBuffer2D &aTransferFunction, soglu::TextureUnitId aTextureUnit)
{
	soglu::gl::bindTexture(aTextureUnit, soglu::TextureTarget::Texture2D, aTransferFunction.getTextureID());
	aProgram.setUniformByName(aUniformName + ".data", aTextureUnit);
	aProgram.setUniformByName(aUniformName + ".from", aTransferFunction.getMappedInterval().first);
	aProgram.setUniformByName(aUniformName + ".to", aTransferFunction.getMappedInterval().second);
	aProgram.setUniformByName(aUniformName + ".xSamples", aTransferFunction.getResolution().first);
	aProgram.setUniformByName(aUniformName + ".eigenvalueProcessPrimary", aTransferFunction.getEigenvalueProcessPrimary());
  aProgram.setUniformByName(aUniformName + ".eigenvalueProcessSecondary", aTransferFunction.getEigenvalueProcessSecondary());
  aProgram.setUniformByName(aUniformName + ".primaryEigenvalueParameters", aTransferFunction.getPrimaryParameters());
  aProgram.setUniformByName(aUniformName + ".primaryValuesMultiplier", aTransferFunction.getPrimaryValuesMultiplier());
  aProgram.setUniformByName(aUniformName + ".secondaryEigenvalueParameters", aTransferFunction.getSecondaryParameters());
  aProgram.setUniformByName(aUniformName + ".secondaryValuesMultiplier", aTransferFunction.getSecondaryValuesMultiplier());
}

} /*vorgl*/

