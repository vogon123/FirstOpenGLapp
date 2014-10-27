#include <glew.h>
#include <wglew.h>
#include <stdio.h>
#include <stdlib.h>

#include "shaders.h"

CShader::CShader()
{
	bLoaded = false;
}

/*-----------------------------------------------

Name:		loadShader

Params:	sFile - path to a file
			a_iType - type of shader (fragment, vertex, geometry)

Result:	Loads and compiles shader.

/*---------------------------------------------*/

bool CShader::loadShader(char * sFile, int a_iType)
{
	FILE* fp = fopen(sFile, "rt");
	if(!fp)return false;

	//Загрузка построчно шейдера из файла в массив 

	GLchar * strings[128];
	int stringsI=0;

	char sLine[255];
	while(fgets(sLine, 255, fp))
	{
		strings[stringsI]=(GLchar*)malloc(255);
		strcpy(strings[stringsI],sLine); 
		stringsI++;
	}
	fclose(fp);

	uiShader = glCreateShader(a_iType);

	glShaderSource(uiShader, stringsI, (const GLchar**)strings, NULL);
	glCompileShader(uiShader);

	int iCompilationStatus;
	glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iCompilationStatus);

	if(iCompilationStatus == GL_FALSE)return false;
	iType = a_iType;
	bLoaded = true;

	return 1;
}

/*-----------------------------------------------

Name:		isLoaded

Params:	none

Result:	True if shader was loaded and compiled.

/*---------------------------------------------*/

bool CShader::isLoaded()
{
	return bLoaded;
}

/*-----------------------------------------------

Name:		getShaderID

Params:	none

Result:	Returns ID of a generated shader.

/*---------------------------------------------*/

UINT CShader::getShaderID()
{
	return uiShader;
}

/*-----------------------------------------------

Name:		deleteShader

Params:	none

Result:	Deletes shader and frees memory in GPU.

/*---------------------------------------------*/

void CShader::deleteShader()
{
	if(!isLoaded())return;
	bLoaded = false;
	glDeleteShader(uiShader);
}

CShaderProgram::CShaderProgram()
{
	bLinked = false;
}

/*-----------------------------------------------

Name:		createProgram

Params:	none

Result:	Creates a new program.

/*---------------------------------------------*/

void CShaderProgram::createProgram()
{
	uiProgram = glCreateProgram();
}

/*-----------------------------------------------

Name:		addShaderToProgram

Params:	sShader - shader to add

Result:	Adds a shader (like source file) to
			a program, but only compiled one.

/*---------------------------------------------*/

bool CShaderProgram::addShaderToProgram(CShader* shShader)
{
	if(!shShader->isLoaded())return false;

	glAttachShader(uiProgram, shShader->getShaderID());

	return true;
}

/*-----------------------------------------------

Name:		linkProgram

Params:	none

Result:	Performs final linkage of OpenGL program.

/*---------------------------------------------*/

bool CShaderProgram::linkProgram()
{
	glLinkProgram(uiProgram);
	int iLinkStatus;
	glGetProgramiv(uiProgram, GL_LINK_STATUS, &iLinkStatus);
	bLinked = iLinkStatus == GL_TRUE;
	return bLinked;
}

/*-----------------------------------------------

Name:		deleteProgram

Params:	none

Result:	Deletes program and frees memory on GPU.

/*---------------------------------------------*/

void CShaderProgram::deleteProgram()
{
	if(!bLinked)return;
	bLinked = false;
	glDeleteProgram(uiProgram);
}

/*-----------------------------------------------

Name:		useProgram

Params:	none

Result:	Tells OpenGL to use this program.

/*---------------------------------------------*/

void CShaderProgram::useProgram()
{
	if(bLinked)glUseProgram(uiProgram);
}

/*-----------------------------------------------

Name:	getProgramID

Params:	none

Result:	Returns OpenGL generated shader program ID.

/*---------------------------------------------*/

UINT CShaderProgram::getProgramID()
{
	return uiProgram;
}