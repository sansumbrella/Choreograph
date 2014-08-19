
#include "choreograph/Choreograph.hpp"
#include "cinder/Quaternion.h"

using namespace cinder;

namespace choreograph
{

// Specialization of lerpT for Quaternions to use slerping.
template<>
inline Quatf lerpT( const Quatf &start, const Quatf &end, float time )
{
	Quatf val = start.slerp( time, end ).normalized();
	if( std::isfinite( val.getAxis().x ) && std::isfinite( val.getAxis().y ) && std::isfinite( val.getAxis().z ) )
		return val;
	else
		return Quatf::identity();
}

template<>
inline Quatd lerpT( const Quatd &start, const Quatd &end, float time )
{
	Quatd val = start.slerp( time, end ).normalized();
	if( std::isfinite( val.getAxis().x ) && std::isfinite( val.getAxis().y ) && std::isfinite( val.getAxis().z ) )
		return val;
	else
		return Quatd::identity();
}

} // namespace choreograph
