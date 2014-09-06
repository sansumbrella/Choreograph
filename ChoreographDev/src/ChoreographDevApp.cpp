#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ChoreographDevApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ChoreographDevApp::setup()
{
}

void ChoreographDevApp::mouseDown( MouseEvent event )
{
}

void ChoreographDevApp::update()
{
}

void ChoreographDevApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( ChoreographDevApp, RendererGl )
