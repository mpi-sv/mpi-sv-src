 /*
 ** KF-Ray, raytracer parallèle
 **  perlin.c repris de la version JAVA sur http://mrl.nyu.edu/~perlin/noise/ :
 **  JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE - COPYRIGHT 2002 KEN PERLIN
 **
 ** Karin AIT-SI-AMER & Florian DANG
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 */

#include <stdio.h>
#include <math.h>
#include "perlin.h"


float Pnoise(float x, float y, float z)
{

	int p[512];
	int i;
	for (i = 0; i < 256 ; i++)
		p[256+i] = p[i] = permutation[i];


    int X = (int)floor(x) & 255,	/* FIND UNIT CUBE THAT */
        Y = (int)floor(y) & 255,	/* CONTAINS POINT.     */
        Z = (int)floor(z) & 255;

    x -= floor(x);			/* FIND RELATIVE X,Y,Z */
    y -= floor(y);			/* OF POINT IN CUBE.   */
    z -= floor(z);

    float	u = Fade(x),		/* COMPUTE FADE CURVES */
			v = Fade(y),	/* FOR EACH OF X,Y,Z.  */
			w = Fade(z);

    int A = p[X]+Y,
        AA = p[A]+Z,
        AB = p[A+1]+Z,			/* HASH COORDINATES OF */
        B = p[X+1]+Y,
        BA = p[B]+Z,
        BB = p[B+1]+Z;			/* THE 8 CUBE CORNERS, */

	return Lerp(w, Lerp(v, Lerp(u, Grad(p[AA  ], x  , y  , z   ),	// AND ADD
					Grad(p[BA  ], x-1, y  , z   )),	// BLENDED
                            Lerp(u, Grad(p[AB  ], x  , y-1, z   ),	// RESULTS
                                    Grad(p[BB  ], x-1, y-1, z   ))),	// FROM  8
                    Lerp(v, Lerp(u, Grad(p[AA+1], x  , y  , z-1 ),	// CORNERS
                                    Grad(p[BA+1], x-1, y  , z-1 )),	// OF CUBE
                            Lerp(u, Grad(p[AB+1], x  , y-1, z-1 ),
                                    Grad(p[BB+1], x-1, y-1, z-1 ))));
}

float Fade(float t)
{
	 return t * t * t * (t * (t * 6 - 15) + 10);
}

float Lerp(float t, float a, float b)
{
	return a + t * (b - a);
}

float Grad(int hash, float x, float y, float z)
{
    int		h = hash & 15;		/* CONVERT LO 4 BITS OF HASH CODE */
    float	u = h < 8 ? x : y,	/* INTO 12 GRADIENT DIRECTIONS.   */
			v = h < 4 ? y : h==12||h==14 ? x : z;

    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
