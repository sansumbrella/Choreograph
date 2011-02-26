#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

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
	Vec2f	mPos;
};

void DevelopmentApp::setup()
{
	
	mBackgroundColor = Color( CM_HSV, 0, 0, 0 );
	mSequence.add( boost::bind( &DevelopmentApp::respond, this ), 2.0 );
	mSequence.add( &mPos, Vec2f( getWindowWidth(), getWindowHeight() ),
				  2.0f, Quadratic::easeInOut );
}

void DevelopmentApp::respond()
{
	mBackgroundColor = Color( CM_HSV, Rand::randFloat(0.0f,0.4f), 0.8f, 0.7f );
	console() << "Cue point was reached." << endl;
}

void DevelopmentApp::mouseDown( MouseEvent event )
{
	mSequence.reset();
}

void DevelopmentApp::update()
{
	mSequence.step( 1.0/60.0 );
}

void DevelopmentApp::draw()
{
	// clear out the window with black
	gl::clear( mBackgroundColor ); 
	gl::color( Color::white() );
	gl::drawSolidCircle( mPos, 24.0f );
}


CINDER_APP_BASIC( DevelopmentApp, RendererGl )
