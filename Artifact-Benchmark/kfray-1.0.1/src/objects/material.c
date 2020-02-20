 /*
 ** KF-Ray, raytracer parallèle
 **  material.c
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
#include "scene.h"


t_material	CreateMaterial	(int type0, float *color0, float reflection0,
				float specular0, float power0, float perlin0,
				float bump0, float density0, float refraction0)
{
	t_material mat;

	mat.type = type0;
	mat.color = CopyfArray(color0, 3);
	mat.specular = specular0;
	mat.reflection = reflection0;
	mat.power = power0;
	mat.perlin = perlin0;
	mat.bump = bump0;
	mat.density = density0;
	mat.refraction = refraction0;

	return	mat;
}


t_material	*CopyMaterial(t_material *mat0)
{
	t_material *mat;
	mat = (t_material *) malloc(sizeof (t_material));

	mat->type = mat0->type;
	mat->color = mat0->color;
	mat->specular = mat0->specular;
	mat->reflection = mat0->reflection;
	mat->power = mat0->power;
	mat->perlin = mat0->perlin;
	mat->bump = mat0->bump;
	mat->density = mat0->density;
	mat->refraction = mat0->refraction;

	return	mat;
}


void		PrintMaterial(t_material *mat)
{
	fprintf(stdout, "[Material] color = ");
	PrintfArray(mat->color, 3);
	fprintf(stdout, "type=%d refl = %f | spec = %f | rough = %f\n |",
		mat->type, mat->reflection, mat->specular, mat->power);
	fprintf(stdout, "perlin = %f | bump = %f | density = %f | refraction = %f",
		mat->perlin, mat->bump, mat->density, mat->refraction);
	fprintf(stdout, "\n");
}
