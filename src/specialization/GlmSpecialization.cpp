
#include <glm/gtc/quaterion.hpp>

using namespace glm;

namespace choreograph
{

// Specialization of lerpT for Quaternions to use slerping
// Currently untested.
template<>
inline quat lerpT( const quat &start, const quat &end, float time )
{
	quat val = quaternion::normalize( quaternion::slerp( start, end, time ) );
	vec3 axis = val.axis();
	if( std::isfinite( axis.x ) && std::isfinite( axis.y ) && std::isfinite( axis.z ) )
	{
		return val;
	}
	else
	{
		return quat();
	}
}

} // namespace choreograph
