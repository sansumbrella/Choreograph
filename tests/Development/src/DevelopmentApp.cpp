#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DevelopmentApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void DevelopmentApp::setup()
{
}

void DevelopmentApp::mouseDown( MouseEvent event )
{
}

void DevelopmentApp::update()
{
}

void DevelopmentApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}


CINDER_APP_BASIC( DevelopmentApp, RendererGl )
