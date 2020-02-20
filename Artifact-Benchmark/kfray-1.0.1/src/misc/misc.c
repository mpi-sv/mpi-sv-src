 /*
 ** KF-Ray, raytracer parallèle
 **  misc.c
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
#include "misc.h"
#include <sys/time.h>


double my_gettimeofday()
{
	struct timeval tmp_time;
	gettimeofday(&tmp_time, NULL);
	return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}

float	*CopyfArray(float *Tab0, int length)
{
	float	*Tab;
	Tab = (float*) malloc(length * sizeof(float));

	int i;
	for (i=0; i<length; i++)
		Tab[i] = Tab0[i];

	return	Tab;
}

void	PrintfArray(float *Tab, int length)
{
	int i;
	fprintf(stdout, "(");
	for (i=0; i<length; i++)
		fprintf(stdout, "%f ",Tab[i]);

	fprintf(stdout, ")");
}

int	*CopyiArray(int *Tab0, int length)
{
	int	*Tab;
	Tab = (int*) malloc(length * sizeof(int));

	int i;
	for (i=0; i<length; i++)
		Tab[i] = Tab0[i];

	return	Tab;
}

void	PrintiArray(int *Tab, int length)
{
	int i;
	fprintf(stdout, "(");
	for (i=0; i<length; i++)
		fprintf(stdout, "%d ",Tab[i]);
	printf(")");
}

void	PrintcArray(unsigned char *Tab, int length)
{
	int i;
	fprintf(stdout, "(");
	for (i=0; i<length; i++)
		fprintf(stdout, "%d ",Tab[i]);
	fprintf(stdout, ")");
}

float	*CreateArrayColor(float red0, float green0, float blue0)
{
	float	*TabColor;
	TabColor = (float*) malloc (3*sizeof (float));
	TabColor[0] = MAX_COLOR * (red0 / 255.0f);
	TabColor[1] = MAX_COLOR * (green0 / 255.0f);
	TabColor[2] = MAX_COLOR * (blue0 / 255.0f);

	return	TabColor;
}

float	*CreateArrayIntensity(float red0, float green0, float blue0)
{
	float	*TabIntensity;
	TabIntensity = (float*) malloc (3*sizeof (float));
	TabIntensity[0] = red0;
	TabIntensity[1] = green0;
	TabIntensity[2] = blue0;

	return	TabIntensity;
}
