#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

#include "Tweening.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::tween;

class BasicTweenApp : public AppBasic {
public:
	void prepareSettings(Settings *settings);
	
	void setup();
	void update();
	void draw();
	void shutdown();
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	
	void playRandomTween();
	void tweenToMouse();
	
	void mouseDown( MouseEvent event );
	
	void resize( int width, int height );
	
	void fileDrop( FileDropEvent event );
	void respond();
	
private:	
	float mX, mY;
	float mStep;
	Vec3f mPos;
	ColorA mColor;
	Timeline mTimeline;
};

void BasicTweenApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize(600,400);
	settings->setTitle("BasicTween");
}

void BasicTweenApp::setup()
{	
	mX = getWindowWidth()/2;
	mY = getWindowHeight()/2;
	mPos = Vec3f(0,0,0);
	
	mStep = 1.0 / 60.0;
	
	mColor = ColorA( 0.5, 0.55, 0.52, 1.0 );
	playRandomTween();
	
	mTimeline.add( boost::bind( &BasicTweenApp::respond, this ), 2.0 );
	
}

void BasicTweenApp::update()
{
	// step our animation forward
	mTimeline.step( mStep );
	// step() also works, it uses 1.0/app::getFrameRate()
//	TweenManager::instance().step();
}

void BasicTweenApp::draw()
{
	gl::clear(mColor);
	
	gl::color(Color::black());
	gl::drawSolidCircle( Vec2i( mX, mY ), 20.0f );
	
	gl::color(Color::white());
	gl::drawSolidCircle( Vec2i(mPos.x, mPos.y), 15.0f );
	
}

void BasicTweenApp::respond()
{
	console() << "Tween completed." << endl;
}

void BasicTweenApp::resize(int width, int height)
{
	
}

//MouseEvents
void BasicTweenApp::mouseDown( MouseEvent event )
{
	tweenToMouse();
}

void BasicTweenApp::playRandomTween()
{
	// Reset the timeline to zero
	mTimeline.reset();
	
	// Tween a Vec3f
	Vec3f randomPos = Vec3f(Rand::randFloat(getWindowWidth()), Rand::randFloat(getWindowHeight()), 0.0f);
	
	// Create our tween
	mTimeline.add( &mPos, randomPos, 2.0 );
	
	// Tween our floats
	randomPos = Vec3f(Rand::randFloat(getWindowWidth()), Rand::randFloat(getWindowHeight()), 0.0f);
	mTimeline.add( &mX, randomPos.x, 2.0 );
	mTimeline.add( &mY, randomPos.y, 2.0 );
}

void BasicTweenApp::tweenToMouse()
{
	// Reset the timeline to zero
	mTimeline.reset();
	
	// Move our properties to the mouse position with with different easing
	Vec3f mousePos = Vec3f( getMousePos().x, getMousePos().y, 0.0f );
	// Tween a Vec3f all at once with custom easing
	TweenRef newTween = mTimeline.replace( &mPos, mousePos, 1.25, Back::easeOut );
	
	// Tween our floats
	mTimeline.replace( &mX, mousePos.x, 2.0, Back::easeInOut );
	mTimeline.replace( &mY, mousePos.y, 1.5, Back::easeInOut );
}

//KeyEvents
void BasicTweenApp::keyDown( KeyEvent event )
{
	switch( event.getChar() ){
		case 'r':
			mTimeline.reset();
			break;
		case 't':
			mStep *= -1;
			break;
		default:
			playRandomTween();
		break;
	}
}

void BasicTweenApp::keyUp( KeyEvent event )
{
	switch( event.getChar() ){
		default:
		break;
	}
}

void BasicTweenApp::fileDrop( FileDropEvent event )
{
	
}

void BasicTweenApp::shutdown()
{
//	TweenManager::instance().cancelAllTweens();
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
