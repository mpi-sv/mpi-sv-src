 /*
 ** KF-Ray, raytracer parallèle
 **  image.c
 **
 ** Copyright 2009 Karin AIT-SI-AMER & Florian DANG
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
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "misc.h"
#include "image.h"


void	SetPixel(unsigned char *img, int w, int x, int y, char r, char g, char b)
{
	img[y * w * 3 + x * 3] = r;
	img[y * w * 3 + x * 3 + 1] = g;
	img[y * w * 3 + x * 3 + 2] = b;
}


void	SaveImage(unsigned char *image, char *img_name, int w, int h)
{

	char *ptr;
	ptr = (char *) malloc(MAX_FILE_LENGTH * sizeof(char));

	if (img_name[0] == '/')
	{
		perror("Erreur : Le programme n'accepte pas les chemins complets");
		return;
	}

	//sprintf(ptr, "../scenes/images/%s", img_name);
        sprintf(ptr, "../scenes/%s", img_name);

	FILE* file_img = fopen(ptr, "w");
	fprintf(file_img, "P6\n%d %d\n%d\n", w, h, 255);
	const size_t bytes = fwrite(image, sizeof(char), w * h * 3, file_img);
	if (bytes) { }		// Pour que codeblocks arrête de warning
	fclose(file_img);

	free(image);
	free(ptr);
}


int	ShowImage(char *img_name)
{
	char *ptr;
	ptr = (char *) malloc(MAX_FILE_LENGTH * sizeof(char));

	sprintf(ptr, "../scenes/images/%s", img_name);
	execlp("display","display", ptr, NULL);
	free(ptr);

	return 0;
}


int	ShowImageSys(char *img_name)
{
	char *ptr;
	ptr = (char *) malloc(MAX_FILE_LENGTH * sizeof(char));

	sprintf(ptr, "display ../scenes/images/%s", img_name);
	if (system(ptr))
		return -1;
	free(ptr);

	return 0;
}


void SaveImage2(unsigned char *image, char *img_name, int w, int h)
{

	char *buf;
	char *ptr;

	if ((buf = (char *)malloc((size_t) MAX_FILE_LENGTH)) != NULL)
		ptr = getcwd(buf, (size_t) MAX_FILE_LENGTH);

	fprintf(stdout, "%s\n", ptr);
	if ((int) ptr == -1)
		perror("!Erreur : ");

	if (img_name[0] != '/')
	{
		strcat(ptr, "/../scenes/images/");
		strcat(ptr, img_name);
	}
	else
		perror("!Erreur : Le programme n'accepte pas les chemins complets");

	FILE* file_img = fopen(ptr, "w");
	fprintf(file_img, "P6\n%d %d\n%d\n", w, h, 255);
	const size_t bytes = fwrite(image, sizeof(char), w * h * 3, file_img);
	if (bytes) { }
	fclose(file_img);

	free(image);
	free(ptr);
}

