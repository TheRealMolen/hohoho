#include "noise.h"

#include <algorithm>
#include <cmath>


#define kPerlinOctaves      3


static const float kaPerlinOctaveAmplitude[kPerlinOctaves] =
{
  0.75f, 0.4f, 0.1f,
};


float perlinNoise1(long x)
{
  x = ( x<<13 ) ^ x;
  return ( 1.0f - ( (x * (x * x * 15731 + 789221L) + 1376312589L) & 0x7fffffff) * (1.0f / 1073741824.0f) );
}


float perlinSmoothedNoise1(long x)
{
  return perlinNoise1(x)*0.5f
       + perlinNoise1(x-1)*0.25f
       + perlinNoise1(x+1)*0.25f;
}


float perlinLerpedNoise1(float x)
{
  long  xInteger  = long(x);
  float xFraction = x - xInteger;

  float v1 = perlinSmoothedNoise1( xInteger );
  float v2 = perlinSmoothedNoise1( xInteger + 1 );

  return std::lerp( v1, v2, xFraction );
}


float perlinNoise1D(float x)
{
  float total = 0.0f;

  for( int octave = 0; octave < kPerlinOctaves; octave++ )
  {
    int   frequency = 1 << octave;
    float amplitude = kaPerlinOctaveAmplitude[octave];

    total += perlinLerpedNoise1( x * frequency ) * amplitude;
  }

  // map from [-1,1] to [0,1] and constrain
  return std::clamp( (total + 1.0f) * 0.5f, 0.0f, 1.0f );
}
