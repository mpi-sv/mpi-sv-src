 /*
 ** KF-Ray, raytracer parallèle
 **  vector.c
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 */

 /*************************************************************************
 *   This file is part of KF-Ray.
 *
 *   KF-Ray is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   KF-Ray is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KF-Ray.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vector.h"


t_vector	CreateVector(float x0, float y0, float z0)
{
	t_vector vect;

	vect.x = x0;
	vect.y = y0;
	vect.z = z0;

	return	vect;
}


t_vector	*CopyVector(t_vector *vect0)
{
	t_vector *vect;
	vect = (t_vector *) malloc(sizeof (t_vector));

	vect->x = vect0->x;
	vect->y = vect0->y;
	vect->z = vect0->z;

	return	vect;
}


void		PrintVect(t_vector v)
{
	printf("(%f, %f, %f)", v.x, v.y, v.z);
}


float		Module(t_vector v)
{

	return	sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}


t_vector	Normalize(t_vector v)
{
	float norm = Module(v);

	if (norm!=0)
	{
		v.x /= norm;
		v.y /= norm;
		v.z /= norm;
	}
	else
		printf("!Warning : Vecteur nul\n");

	return	v;
}

t_vector	Addition(t_vector u, t_vector v)
{
	t_vector w;

	w.x = u.x + v.x;
	w.y = v.y + u.y;
	w.z = v.z +u.z;

	return	w;
}

t_vector	Substraction(t_vector v, t_vector u)
{
	t_vector w;

	w.x = v.x - u.x;
	w.y = v.y - u.y;
	w.z = v.z - u.z;

	return	w;

}


float		DotProd(t_vector u,t_vector v)
{

      return (u.x*v.x+u.y*v.y+u.z*v.z);
}


t_vector	CrossProd(t_vector u, t_vector v)
{
	t_vector w;

	w.x = u.x * v.y - v.x * u.y;
	w.y = u.y * v.z - v.y * u.z;
	w.z = u.z * v.x - v.z * u.x;

	return	w;
}


t_vector	Prod(float lambda,t_vector v)
{
	t_vector w;

	w = v;
	w.x *= lambda;
	w.y *= lambda;	w.z *= lambda;
	return	w;
}


float		Angle(t_vector u, t_vector v)
{
	float	product = DotProd(u, v);
	float	norme = Module(u)*Module(v);

	return	(float) (acos((double) product/norme));
}



