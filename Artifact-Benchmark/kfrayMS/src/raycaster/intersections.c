 /*
 ** KF-Ray, raytracer parallèle
 **  intersections.c
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
#include <math.h>
#include "intersections.h"

#define VARIANTE_SPH

bool	InterPlane(t_ray ray, t_plane plane, float *t)
{
	// Rayon : P = O + t*u
	// Plan : Ax + By + Cz + D = 0
	// normal = (A,B,C,D)

	// Fait un effet marrant
	/*
	if (DotProd(plane.normal, ray.dir) > 0.0f)
		plane.normal = Prod(-1.0f, plane.normal);
	*/
	if (fabs(DotProd(ray.dir, plane.normal)) < EPSILON)
		return FALSE;

	float t0 = (-plane.dist - DotProd(plane.normal, ray.start)) / DotProd(plane.normal, ray.dir);

	if (*t < t0 || t0 < EPSILON)
		return FALSE;

	*t = t0;

	return TRUE;
}


bool	InterSphere(t_ray ray, t_sphere sph, float *t)
{
	// Rayon : P = O + t*u
	// Sphere : CP = r

	// Vecteur centre sphere et origine rayon
	t_vector vect_dist = Substraction(sph.center, ray.start);

	float 	b = DotProd(ray.dir,vect_dist);

	// Discriminant réduit simplifié, le vecteur u est unitaire !
	float 	delta = b*b - DotProd(vect_dist,vect_dist) + sph.radius*sph.radius;

	if (delta < 0.0f)
		return FALSE;

	float 	t0 = b - sqrtf(delta);
	float	t1 = b + sqrtf(delta);

	if (*t < t0 && *t < t1)
		return FALSE;

	if (t0 < EPSILON)
		t0 = INFINITE;
	if (t1 < EPSILON)
		t1 = INFINITE;
	if (t0 == INFINITE && t1 == INFINITE)
		return FALSE; // le point ne peut être derriere le rayon ou trop proche de l'origine

	*t = (t1 < t0) ? t1 : t0;

	return TRUE;
}


bool	InterSphere2(t_ray ray, t_sphere sph, float *t)
{
	t_vector vect_dist = Substraction(sph.center, ray.start);

	float 	b = DotProd(ray.dir,vect_dist);

	float 	delta = b*b - DotProd(vect_dist,vect_dist) + sph.radius*sph.radius;

	if (delta < 0.0f)
		return FALSE;

	float 	t0 = b - sqrtf(delta);
	float	t1 = b + sqrtf(delta);

	bool		bool_tmp = FALSE;

	if ( (t0 > EPSILON) && (t0 < *t) )
	{
		*t = t0;
		bool_tmp = TRUE;
	}
	if ( (t1 > EPSILON) && (t1 < *t) )
	{
		*t = t1;
		bool_tmp = TRUE;
	}

	return	bool_tmp;
}
