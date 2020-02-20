/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     FLOAT = 259,
     INTEGER = 260,
     WIDTH = 261,
     HEIGHT = 262,
     MATERIALS = 263,
     PLANES = 264,
     SPHERES = 265,
     LIGHTS = 266,
     CAMERA = 267,
     DISTVIEW = 268,
     MAT = 269,
     ID = 270,
     TYPE = 271,
     NORMAL = 272,
     TURBULENCE = 273,
     MARBEL = 274,
     WOOD = 275,
     CHECKER = 276,
     RVB = 277,
     REFLECTION = 278,
     SPECULAR = 279,
     ROUGHNESS = 280,
     PERLIN = 281,
     BUMP = 282,
     REFRACTION = 283,
     DENSITY = 284,
     PLANE = 285,
     NORMALE = 286,
     DISTANCE = 287,
     MATERIAL = 288,
     SPHERE = 289,
     CENTER = 290,
     RADIUS = 291,
     LIGHT = 292,
     POSITION = 293,
     INTENSITY = 294,
     EOL = 295
   };
#endif
/* Tokens.  */
#define STRING 258
#define FLOAT 259
#define INTEGER 260
#define WIDTH 261
#define HEIGHT 262
#define MATERIALS 263
#define PLANES 264
#define SPHERES 265
#define LIGHTS 266
#define CAMERA 267
#define DISTVIEW 268
#define MAT 269
#define ID 270
#define TYPE 271
#define NORMAL 272
#define TURBULENCE 273
#define MARBEL 274
#define WOOD 275
#define CHECKER 276
#define RVB 277
#define REFLECTION 278
#define SPECULAR 279
#define ROUGHNESS 280
#define PERLIN 281
#define BUMP 282
#define REFRACTION 283
#define DENSITY 284
#define PLANE 285
#define NORMALE 286
#define DISTANCE 287
#define MATERIAL 288
#define SPHERE 289
#define CENTER 290
#define RADIUS 291
#define LIGHT 292
#define POSITION 293
#define INTENSITY 294
#define EOL 295




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 51 "parser_yy.y"

	float	nb_float;
	char	*string;
	int	nb_int;



/* Line 2068 of yacc.c  */
#line 138 "parser_yy.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


