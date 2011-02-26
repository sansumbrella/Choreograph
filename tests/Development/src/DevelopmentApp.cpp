#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "Choreograph.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::tween;

class DevelopmentApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void respond();
	
private:
	Sequence mSequence;
	Color	mBackgroundColor;
};

void DevelopmentApp::setup()
{
	
	mBackgroundColor = Color( CM_HSV, 0, 0, 0 );
	mSequence.add( boost::bind( &DevelopmentApp::respond, this ), 2.0 );
}

void DevelopmentApp::respond()
{
	mBackgroundColor = Color( CM_HSV, 0.5f, 0.8f, 0.3f );
	console() << "Cue point was reached." << endl;
}

void DevelopmentApp::mouseDown( MouseEvent event )
{
}

void DevelopmentApp::update()
{
	mSequence.step( 1.0/60.0 );
}

void DevelopmentApp::draw()
{
	// clear out the window with black
	gl::clear( mBackgroundColor ); 
}


CINDER_APP_BASIC( DevelopmentApp, RendererGl )
