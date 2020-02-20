 /*
 ** KF-Ray, raytracer parallèle
 **  parser_lex.l
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
 
%option nounput
%option noyywrap

%{

#include "parser_yy.h"

%}

separator	[\t ]+
scene		[Ss]cene
material	[Mm]aterial
plane		[Pp]lane
sphere		[Ss]phere
light		[Ll]ight

%%

	/* Règle des instructions */

{separator}			;
";"				;
","				;
"="				;
"}"				;

^[ \t]*"/*".*"*/"[ \t]*\n	;
^[ \t]*"//".*[ \t]*\n		;

^{scene}([ \n\t]*\{)		;

^{material}([ \n\t]*\{)		{ 
	return MAT;
} 

^{plane}([ \n\t]*\{) 		{
	return PLANE;
} 

^{sphere}([ \n\t]*\{)		{ 
	return SPHERE;
} 

^{light}([ \n\t]*\{) 		{
	return LIGHT;
} 

^{separator}*[Ww]idth		{
	yylval.string = yytext; 
	return WIDTH;
}

^{separator}*[Hh]eight		{
	yylval.string = yytext;
	return HEIGHT;
}

^{separator}*[Mm]aterials	{
	yylval.string = yytext;
	return MATERIALS;
}

^{separator}*[Pp]lanes		{
	yylval.string = yytext; 
	return PLANES;
}

^{separator}*[Ss]pheres		{
	yylval.string = yytext;
	return SPHERES;
}

^{separator}*[Ll]ights		{
	yylval.string = yytext;
	return LIGHTS;
}

^{separator}*[Cc]amera		{
	yylval.string = yytext;
	return CAMERA;
}

^{separator}*[Dd]istview	{
	yylval.string = yytext;
	return DISTVIEW;
}

^{separator}*[Ii]d		{
	yylval.string = yytext;
	return ID;
}

^{separator}*[Tt]ype		{
	yylval.string = yytext;
	return TYPE;
}

^{separator}*[Rr][Gg][Bb]	{
	yylval.string = yytext;
	return RVB;
}


^{separator}*[Rr]eflection	{
	yylval.string = yytext;
	return REFLECTION;
}

^{separator}*[Ss]pecular	{
	yylval.string = yytext;
	return SPECULAR;
}

^{separator}*[Rr]oughness	{
	yylval.string = yytext;
	return ROUGHNESS;
}


^{separator}*[Pp]erlin		{
	yylval.string = yytext;
	return PERLIN;
}

^{separator}*[Bb]ump		{
	yylval.string = yytext;
	return BUMP;
}


^{separator}*[Rr]efraction	{
	yylval.string = yytext;
	return REFRACTION;
}

^{separator}*[Dd]ensity	{
	yylval.string = yytext;
	return DENSITY;
}


^{separator}*[Nn]ormal		{
	yylval.string = yytext;
	return NORMALE;
}

^{separator}*[Dd]istance	{
	yylval.string = yytext;
	return DISTANCE;
}

^{separator}*[Mm]aterial	{
	yylval.string = yytext;
	return MATERIAL;
}

^{separator}*[Cc]enter		{
	yylval.string = yytext;
	return CENTER;
}

^{separator}*[Rr]adius		{
	yylval.string = yytext;
	return RADIUS;
}

^{separator}*[Pp]osition	{ 
	yylval.string = yytext;
	return POSITION;
}

^{separator}*[Ii]ntensity	{
	yylval.string = yytext;
	return INTENSITY;
}

[Nn]ormal			{ 
	yylval.string = yytext;
	return NORMAL;
}
	
[Tt]urbulence			{
	yylval.string = yytext;
	return TURBULENCE;
}

[Mm]arbel			{
	yylval.string = yytext;
	return MARBEL;
}

[Ww]ood				{
	yylval.string = yytext;
	return WOOD;
}

[Cc]hecker			{
	yylval.string = yytext;
	return CHECKER;
}

-?[0-9]+			{
	yylval.nb_int = atoi(yytext);
	return INTEGER;
}

-?([0-9]*\.[a-z0-9]+)		{
	yylval.nb_float = atof(yytext);
	return FLOAT;
} 

^{separator}*\n			;

{separator}*\n			{
	return EOL; 
}
<<EOF>>				{
	return 0 ;
}

%%


