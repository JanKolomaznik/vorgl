#pragma once
#include <soglu/BoundingBox.hpp>
#include <soglu/Camera.hpp>

#include <soglu/OGLTools.hpp>
#include <soglu/GLMUtils.hpp>

namespace vorgl {


void
GLDrawVolumeSlice3D(
		const glm::fvec3 	&aMin,
		const glm::fvec3 	&aMax,
		float			sliceCoord,
		soglu::CartesianPlanes	plane
		);


void
GLDrawVolumeSlices_Buffered(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		glm::fvec3		*vertices,
		unsigned			*indices,
		float			cutPlane
		);

void
GLDrawVolumeSlices_Intermediate(
		const soglu::BoundingBox3D	&bbox,
		const soglu::Camera		&camera,
		unsigned 		numberOfSteps,
		glm::fvec3		*,
		unsigned		*,
		float			cutPlane
		);

} //namespace vorgl
