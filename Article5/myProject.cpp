//--------------------------------------------------------------------------------------
#pragma comment (lib,"opengl32.lib")
#include <windows.h> 
#include <glew.h>
#include <wglew.h>
#include <stdio.h>
#include <glm.hpp>
#include <ext.hpp>
#include "shaders.h"
#include <Windowsx.h>

HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
HGLRC					hRC = NULL;

UINT uiVBO[3];
UINT uiVAO[1];

//Установка матриц для камеры
glm::mat4	mWorld;
glm::mat4	mView;
glm::mat4	mProjection;

const int u=20;
const int v=20;

const int indicesCount=(u-1)*(v-1)*6;

unsigned int indices[indicesCount];

// определяем количество вершин
int verticesCount=u*v;

struct VERTEX{
	glm::vec3 pos[u*v];
	glm::vec3 normal[u*v];
} vertices;

HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitCompatibleContext();
HRESULT InitContext();
void InitGeometry();
void CleanupContext();
void CalculateNormal();
int TransformCursToVertIndx(LONG, LONG);
void TransformPos(float R, GLuint p0);
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

	if( FAILED( InitCompatibleContext() ) )
		return 0;

	if( FAILED( InitContext() ) )
    {
        CleanupContext();
        return 0;
    }

	InitGeometry();
	
    // Цикл обработки сообщений
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupContext();

    return ( int )msg.wParam;
}

HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0; // LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"MyProject";
    wcex.hIconSm = 0; // LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 533, 400 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"MyProject", L"My Project", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
		case WM_LBUTTONDOWN: {
			DWORD xPos = GET_X_LPARAM(lParam);  
			DWORD yPos = GET_Y_LPARAM(lParam); 

			int Index = TransformCursToVertIndx(xPos, yPos);

			if (Index >= 0) {

				TransformPos(0.5f, Index);
				CalculateNormal();

				glBindVertexArray(uiVAO[0]);

				glBindBufferARB(GL_ARRAY_BUFFER, uiVBO[0]); 
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices.pos), vertices.pos);

				glBindBufferARB(GL_ARRAY_BUFFER, uiVBO[1]); 
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices.normal), vertices.normal);
			}

			break;
							 }
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

HRESULT InitCompatibleContext()
{
	int iMajorVersion=0;
	int iMinorVersion=0;

	HDC hDC = GetDC(g_hWnd);

	// Устанавливаем параметры поверхности контекста
	
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize		= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA; //тип пикселей
	pfd.cColorBits = 32;		    //тип цвета
	pfd.cDepthBits = 24;            //тип z-буфера
	pfd.iLayerType = PFD_MAIN_PLANE;
 
	int iPixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (iPixelFormat == 0) return false;

	if(!SetPixelFormat(hDC, iPixelFormat, &pfd)) return false;

	// Создаем совместимый контекст

	HGLRC hRCCompatible = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRCCompatible);

	bool bResult = true;

	if(glewInit() != GLEW_OK)
	{
			MessageBoxA(g_hWnd, "Couldn't initialize GLEW!", "Fatal Error", MB_ICONERROR);
			bResult = false;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRCCompatible);
	//DestroyWindow(hWndFake);

	return bResult;
}

HRESULT InitContext()
{
	int iMajorVersion=3;
	int iMinorVersion=3;

	HDC hDC = GetDC(g_hWnd);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd,sizeof(pfd)); 

	bool bError=false;
	
	// Устанавливаем параметры поверхности контекста

	if(WGLEW_ARB_create_context && WGLEW_ARB_pixel_format)
	{
		const int iPixelFormatAttribList[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, //тип пикселей
			WGL_COLOR_BITS_ARB, 32,			       //тип цвета
			WGL_DEPTH_BITS_ARB, 24,                //тип z-буфера
			WGL_STENCIL_BITS_ARB, 8,
			0 // End of attributes list
		};
		int iContextAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, iMajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, iMinorVersion,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0 // End of attributes list
		};

		int iPixelFormat, iNumFormats;
		wglChoosePixelFormatARB(hDC, iPixelFormatAttribList, NULL, 1, &iPixelFormat, (UINT*)&iNumFormats);

		// Создаем основной контекст

		hRC = wglCreateContextAttribsARB(hDC, 0, iContextAttribs);
		// If everything went OK
		if(hRC) wglMakeCurrent(hDC, hRC);
		else bError = true;

	}
	else bError = true;
	
	if(bError)
	{
		// Generate error messages
		char sErrorMessage[255], sErrorTitle[255];
		sprintf(sErrorMessage, "OpenGL %d.%d is not supported! Please download latest GPU drivers!", iMajorVersion, iMinorVersion);
		sprintf(sErrorTitle, "OpenGL %d.%d Not Supported", iMajorVersion, iMinorVersion);
		MessageBoxA(g_hWnd, sErrorMessage, sErrorTitle, MB_ICONINFORMATION);
		return false;
	}

   return S_OK;
}



void CalculateNormal()
{
	int v1, v2, v3;
	int i, j;
	glm::vec3 a, b, c, p, q, n, normals[indicesCount];
	for (j = 0; j < indicesCount; j += 3)
	{
		v1 = indices[j + 0];
		v2 = indices[j + 1];
		v3 = indices[j + 2];

		a = vertices.pos[v1];
		b = vertices.pos[v2];
		c = vertices.pos[v3];

		p = b - a;
		q = c - a;

		n = glm::normalize( glm::cross(p, q) );

		normals[j + 0] = normals[j + 1] = normals[j + 2] = n;
	}

	for (j = 0; j < verticesCount; ++j) {
		n = glm::vec3(0,0,0);
		for (i = 0; i < indicesCount; ++i)
			if (j == indices[i])
				n += normals[i];

		vertices.normal[j] = glm::normalize(n);
	}
}

int TransformCursToVertIndx(LONG mx, LONG my) {

	GLint viewport[4];
	GLdouble mvm[16];
	GLdouble projm[16];

	GLdouble wx, wy, wz;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
    glGetDoublev(GL_PROJECTION_MATRIX, projm);

	my = viewport[3] - my - 1;

	//glReadPixels( mx, my, 1.0f, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mz );

	gluUnProject(mx, my, 0.0f, mvm, projm, viewport, &wx, &wy, &wz);
	glm::vec3 p(wx, wy, wz);

	GLfloat d1, d2, d3;
	GLint i, v1, v2, v3;
	glm::vec4 pos1, pos2, pos3;

	for (i = 0; i < indicesCount; i += 3) {

		v1 = indices[i+0];
		v2 = indices[i+1];
		v3 = indices[i+2];

		pos1 = mProjection * mView * glm::vec4(vertices.pos[v1], 1.0f);
		pos1 /= pos1.w;

		pos2 = mProjection * mView * glm::vec4(vertices.pos[v2], 1.0f);
		pos2 /= pos2.w;

		pos3 = mProjection * mView * glm::vec4(vertices.pos[v3], 1.0f);
		pos3 /= pos3.w;

		//векторный метод определения принадлежности точки треугольнику

		GLfloat Bx, By, Cx, Cy, Px, Py, m, l;

		Bx = pos2.x - pos1.x;	By = pos2.y - pos1.y;
		Cx = pos3.x - pos1.x;	Cy = pos3.y - pos1.y;
		Px = p.x - pos1.x;		Py = p.y - pos1.y;

		m = (Px*By - Bx*Py) / (Cx*By - Bx*Cy);

		if ( (m >= 0) && (m <= 1)) {
			l = (Px - m*Cx) / Bx;
			if ( (l >= 0) && (m + l <= 1) )
				break;
		}
	}

	if (i != indicesCount) {

		//вычисляем три расстояния между точкой и вершинами треугольника
		d1 = glm::length(glm::vec2(p.x - pos1.x, p.y - pos1.y));
		d2 = glm::length(glm::vec2(p.x - pos2.x, p.y - pos2.y));
		d3 = glm::length(glm::vec2(p.x - pos3.x, p.y - pos3.y));

		//выбираем минимальное расстояние
		if ((d1 < d2) && (d1 < d3))
			return v1;
		else if (d2 < d3)
			return v2;
		else
			return v3;
	}

	//если точка не попала ни в одни треугольник - возращаем -1
	return -1;
}

void TransformPos(float R, GLuint p0) {
	GLuint i, j;
	float x, d;

	i = p0;
	while ( (i >= 0) && (glm::length(vertices.pos[i] - vertices.pos[p0]) < R) ) {
		j = i - 1;
		while ( (j + 1) % u) {
			d = glm::length(vertices.pos[p0] - vertices.pos[j]);
			if (d >= R)
				break;
			x = glm::sqrt(R*R - d*d);
			vertices.pos[j] += vertices.normal[j] * x;
			--j;
		}
		if (i != p0) {
			d = glm::length(vertices.pos[p0] - vertices.pos[i]);
			x = glm::sqrt(R*R - d*d);
			vertices.pos[i] += vertices.normal[i] * x;
		}
		i -= v;
	}

	i = p0;
	while ( (i < verticesCount) && (glm::length(vertices.pos[i] - vertices.pos[p0]) < R) ) {
		j = i + 1;
		while (j % u) {
			d = glm::length(vertices.pos[p0] - vertices.pos[j]);
			if (d >= R)
				break;
			x = glm::sqrt(R*R - d*d);
			vertices.pos[j] += vertices.normal[j] * x;
			++j;
		}
		if (i != p0) {
			d = glm::length(vertices.pos[p0] - vertices.pos[i]);
			x = glm::sqrt(R*R - d*d);
			vertices.pos[i] += vertices.normal[i] * x;
		}
		i += v;
	}

	vertices.pos[p0] += vertices.normal[p0] * R;
}

void GenerateLandscape()
{
	// Генерация сетки вершин для вершинного буфера
	for (int i=0; i<u; i++)
	for (int j=0; j<v; j++)
	{
		float x=(float)i/(float)u-0.5f;
		float y=(float)j/(float)v-0.5f;
		vertices.pos[j*u+i]=glm::vec3(x,0,y)*7.5f;
	}

	// Генерация  индексного буфера
	for (int i=0; i<(u-1); i++)
	for (int j=0; j<(v-1); j++)
	{
		unsigned int indexa=j*(u-1)+i;
		unsigned int indexb=j*u+i;
		indices[indexa*6+0]=indexb;
		indices[indexa*6+1]=indexb+1+u;
		indices[indexa*6+2]=indexb+1;

		indices[indexa*6+3]=indexb;
		indices[indexa*6+4]=indexb+u;
		indices[indexa*6+5]=indexb+u+1;
	}

   //Расчёт нормалей вершин
   CalculateNormal();

   glGenVertexArrays(1, uiVAO); // Обобщенный буфер
   glGenBuffers(3, uiVBO);		// Два буфера для position, normal и один иднексный буфер

   // Копирование данных буферов в обобщенный буфер
   glBindVertexArray(uiVAO[0]); 

   glBindBufferARB(GL_ARRAY_BUFFER, uiVBO[0]); 
   glBufferDataARB(GL_ARRAY_BUFFER, 3*verticesCount*sizeof(float), vertices.pos, GL_DYNAMIC_DRAW); 
   glEnableVertexAttribArrayARB(0); 
   glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

   glBindBufferARB(GL_ARRAY_BUFFER, uiVBO[1]); 
   glBufferDataARB(GL_ARRAY_BUFFER, 3*verticesCount*sizeof(float), vertices.normal, GL_DYNAMIC_DRAW); 
   glEnableVertexAttribArrayARB(1); 
   glVertexAttribPointerARB(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 

   // Копирование индексного буфера
   glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, uiVBO[2]); 
   glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, indicesCount*sizeof(unsigned int), indices, GL_DYNAMIC_DRAW); 
}

CShader shVertex, shFragment; 
CShaderProgram spMain; 

//--------------------------------------------------------------------------------------
// Инициализация геометрии
//--------------------------------------------------------------------------------------
void InitGeometry() 
{ 
   // Создаем процедурно сгенерированный объект
   GenerateLandscape();

   // Load shaders and create shader program

   shVertex.loadShader("data\\shaders\\shader.vert", GL_VERTEX_SHADER); 
   shFragment.loadShader("data\\shaders\\shader.frag", GL_FRAGMENT_SHADER); 

   spMain.createProgram(); 
   spMain.addShaderToProgram(&shVertex); 
   spMain.addShaderToProgram(&shFragment); 

   spMain.linkProgram(); 
   spMain.useProgram(); 

   wglSwapIntervalEXT(1);

   //Включаем Zбуфер
   glEnable(GL_DEPTH_TEST);
   glClearDepth(1.0f);

}   

void Render()
{

    // Очистка рендер-таргета
	HDC hDC = GetDC(g_hWnd);

	// Очистка поверхности цветом
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 //Инициализация орбитальных данных координат камеры
    glm::vec3   Eye( 0.0f, 3.0f, 5.0f );

	////Установка матриц для камеры
	//glm::mat4	mWorld;
	//glm::mat4	mView;
	//glm::mat4	mProjection;
	mWorld=glm::mat4(1.0f,0,0,0, 0,1.0f,0,0, 0,0,1.0f,0, 0,0,0,1.0f);

	mWorld=		glm::translate(0,0,0);

	mView=		glm::lookAt(Eye,glm::vec3(0,0,0),glm::vec3(0.0f,1.0f,0.0f));
	mProjection=glm::perspectiveFov(87.0f,533.0f,400.0f,0.002f,1000.0f);

	//Установка констант шейдера матриц
	int iWorld=	glGetUniformLocation(spMain.getProgramID(),"mWorld");
	int iView=	glGetUniformLocation(spMain.getProgramID(),"mView");
	int iProjection=	glGetUniformLocation(spMain.getProgramID(),"mProjection");

	glUniformMatrix4fv(iWorld,1,GL_FALSE,glm::value_ptr(mWorld));
	glUniformMatrix4fv(iView,1,GL_FALSE,glm::value_ptr(mView));
	glUniformMatrix4fv(iProjection,1,GL_FALSE,glm::value_ptr(mProjection));

	// Рендер
	glBindVertexArray(uiVAO[0]);
	glUniformMatrix4fv(iWorld,1,GL_FALSE,glm::value_ptr(mWorld));
	glDrawElements(GL_TRIANGLES,indicesCount,GL_UNSIGNED_INT,0);

	SwapBuffers(hDC);
	DeleteObject(hDC);

}


//--------------------------------------------------------------------------------------
// Очистка
//--------------------------------------------------------------------------------------
void CleanupContext()
{
	spMain.deleteProgram();
	shVertex.deleteShader();
	shFragment.deleteShader();


	HDC hDC = GetDC(g_hWnd);
	wglMakeCurrent(NULL, NULL);
	//wglDeleteContext(hRC);
	ReleaseDC(g_hWnd, hDC);

	g_hWnd = NULL;
}


