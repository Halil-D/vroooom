#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <windows.h>
#include "include/glad/glad.h"
#include "bmpr.h"
#include <GL/gl.h>

#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CLOSE) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

// WINDOW SETUP
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GLWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
    0,
    L"GLWindow",
    L"My Engine",
    WS_POPUP | WS_VISIBLE,
    0, 0,
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN),
    NULL, NULL, hInstance, NULL
    );
    ShowWindow(hwnd, nCmdShow);
    HDC hdc = GetDC(hwnd);

// PIXEL FORMAT
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

// TEMP CONTEXT
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

// LOAD MODERN CONTEXT
    typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
    auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");

    int attribs[] = {
        0x2091, 3,
        0x2092, 3,
        0x9126, 0x00000001,
        0
    };

    HGLRC modernContext = wglCreateContextAttribsARB(hdc, 0, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(hdc, modernContext);
    

// GLAD
    if (!gladLoadGL()) {
        MessageBoxA(NULL, "GLAD failed!", "Error", MB_OK);
        return -1;
    }
// VIEWPORT SETUP
    glViewport(0, 0,
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN));
    
    
// SHADERS
    const char* bgVS =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aUV;\n"
        "out vec2 uv;\n"
        "void main() {\n"
        "   uv = aUV;\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "}";

    const char* bgFS =
        "#version 330 core\n"
        "in vec2 uv;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "   FragColor = texture(tex, uv);\n"
        "}";

    const char* vs =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform vec2 offset;\n"
        "uniform float scale;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.xy * scale + offset, aPos.z, 1.0);\n"
        "}";

    const char* fs =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
        "}";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vs, NULL);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fs, NULL);
    glCompileShader(fShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    GLuint bgVShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(bgVShader, 1, &bgVS, NULL);
    glCompileShader(bgVShader);

    GLuint bgFShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(bgFShader, 1, &bgFS, NULL);
    glCompileShader(bgFShader);

    GLuint bgProgram = glCreateProgram();
    glAttachShader(bgProgram, bgVShader);
    glAttachShader(bgProgram, bgFShader);
    glLinkProgram(bgProgram);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

// TRIANGLE
    float vertices[] = {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
    float bgVertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
        1.0f, -1.0f, 0.0f,  1.0f, 1.0f,

        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        1.0f,  1.0f, 0.0f,  1.0f, 0.0f
};

    GLuint bgVAO, bgVBO;
    glGenVertexArrays(1, &bgVAO);
    glGenBuffers(1, &bgVBO);

    glBindVertexArray(bgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);

// position attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

// uv attrib
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER bmiHeader;

// Open BMP file
    FILE* bmp = fopen("blis.bmp", "rb");

    if (!bmp) {
        printf("Failed to open blis.bmp\n");
        return -1;
    }

// Read BMP file header
    fread(
        &fileHeader,
        sizeof(BITMAPFILEHEADER),
        1,
        bmp
    );

// Verify BMP signature
    if (fileHeader.bfType != 0x4D42) {
        printf("Not a valid BMP file\n");
        fclose(bmp);
        return -1;
    }

// Read BMP info header
    fread(
        &bmiHeader,
        sizeof(BITMAPINFOHEADER),
        1,
        bmp
    );

// Verify 24-bit BMP
    if (bmiHeader.biBitCount != 24) {
        printf("Only 24-bit BMP supported\n");
        fclose(bmp);
        return -1;
    }

// Allocate pixel buffer
    RGBTRIPLE* pixels =
        (RGBTRIPLE*)malloc(
            bmiHeader.biWidth *
            bmiHeader.biHeight *
            sizeof(RGBTRIPLE)
        );

    if (!pixels) {
        printf("Failed to allocate pixel buffer\n");
        fclose(bmp);
        return -1;
    }

// Read pixel data
    civi_bmp_read_pixels(
        pixels,
        &bmiHeader,
        bmp
    );

// Create OpenGL texture
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

// Texture settings
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

// Important for BMP row alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

// Upload texture to GPU
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        bmiHeader.biWidth,
        bmiHeader.biHeight,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        pixels
    );

// Cleanup CPU-side memory
    free(pixels);
    fclose(bmp);
    
// CACHE UNIFORMS
    int offsetLoc = glGetUniformLocation(program, "offset");
    int scaleLoc  = glGetUniformLocation(program, "scale");

// PLAYER
    float playerX = 0.0f, playerY = 0.0f;
    float velocityX = 0.0f, velocityY = 0.0f;
    float testspd = 1.5f;
    float gravity = -3.0f;
    float jumpForce = 1.5f;
    float accel = 3.0f;

    bool onGround = true;

// DELTA TIME
    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);

    MSG msg = {};

    while (true) {

        // DELTA TIME
        QueryPerformanceCounter(&now);
        float dt = (now.QuadPart - last.QuadPart) / (float)freq.QuadPart;
        last = now;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                return 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // INPUT
        if (GetAsyncKeyState('A')){
            velocityX = -testspd;}
        else if (GetAsyncKeyState('D')){
            velocityX = testspd;}
        else{
            velocityX *= 0.5f; }

        if ((GetAsyncKeyState(VK_SPACE)) && onGround) {
            velocityY = jumpForce;
            onGround = false;
           
        }
        // QUIT
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        PostQuitMessage(0);
}
        

        // PHYSICS
        if (!onGround){
            velocityY += gravity * dt;
        }
        playerX += velocityX * dt ;
        playerY += velocityY * dt ;
        
        

        if (playerY <= -0.5f) {
            playerY = -0.5f;
            velocityY = 0;
            onGround = true;
        }

        // --- RENDER ---
        glClearColor(0.1f, 0.2f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(bgProgram);
        glBindTexture(GL_TEXTURE_2D, texture);  // your blis texture
        glBindVertexArray(bgVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(program);
        glUniform2f(offsetLoc, playerX, playerY);
        glUniform1f(scaleLoc, 0.2f);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SwapBuffers(hdc);
        Sleep(1);
    }
}