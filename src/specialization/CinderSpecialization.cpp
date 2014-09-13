
#include "choreograph/Choreograph.hpp"
#include "cinder/Quaternion.h"

using namespace cinder;

namespace choreograph
{

// Specialization of lerpT for Quaternions to use slerping
// Currently untested.
template<>
inline quat lerpT( const quat &start, const quat &end, float time )
{
  quat val = glm::normalize( glm::slerp( start, end, time ) );
  vec3 axis = glm::axis( val );
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
