// Some day, I will figure out how to use Catch nicely within Xcode to run some tests

const int TEST_ANIMS = 10000;
const float TEST_DURATION = 0.2f;
const float TEST_DT = 1.0f / 60.0f;

for( int i = 0; i < 50; ++i )
{
  CI_TIMELINE->clear();
  CI_TIMELINE->stepTo( 0.0f );
  CO_TIMELINE->clear();

  vector<ci::Anim<vec2>> cinder_anims( TEST_ANIMS, vec2( 0 ) );
  vector<co::Output<vec2>> choreograph_outputs( TEST_ANIMS, vec2( 0 ) );

  for( int i = 0; i < TEST_ANIMS; ++i )
  {
    CI_TIMELINE->apply( &cinder_anims[i], vec2( i, i * 2 ), TEST_DURATION );
    CO_TIMELINE->move( &choreograph_outputs[i] ).getSequence().rampTo( vec2( i, i * 2 ), TEST_DURATION );
  }


  Timer ci_timer( true );
  performanceCheckCinder();
  ci_timer.stop();

  Timer co_timer( true );
  performanceCheckChoreograph();
  co_timer.stop();


  console() << "Cinder: " << ci_timer.getSeconds() << ", Choreograph: " << co_timer.getSeconds() << endl;
  console() << "Time ratio (Cinder/Choreograph): " << ci_timer.getSeconds() / co_timer.getSeconds() << endl;
}

for( float t = 0.0f; t < TEST_ANIMS * TEST_DURATION; t += TEST_DT )
{
  CI_TIMELINE->step( TEST_DT );
}

for( float t = 0.0f; t < TEST_ANIMS * TEST_DURATION; t += TEST_DT )
{
  CO_TIMELINE->step( TEST_DT );
}

// Many anims, many tweens

// One anim, many tweens
