#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <sstream>
#include <fstream>
#include <vector>

// !!!! Keep checking the top of the Terminal you dumbassssssssssssssss

// Global vars
SDL_Window *gWindow = nullptr;
int gScreenW = 800;
int gScreenH = 600;
SDL_GLContext gSDLcontext = 0;

GLuint gShaderProgram = 0;

// VAO and VBO
GLuint gVertexArrayObject = 0, gVertexbufferObject = 0, gVertexBufferObject2 = 0;

// Index Buffer object(IBO) / Element Buffer object(EBO)
// Used to store the array of indices that we want to draw from when we do indexed drawing 
GLuint gIndexBufferObject = 0;

// Shaders 
GLuint gVertexShader = 0;
GLuint gFragmentShader = 0;

bool gRunning = true;
SDL_Event e;
//-----------------------------------------//
// ^^^^^^^^^^^^^^^^^ ERROR HANDLING ^^^^^^^^^^^^^^ //

static void GLClearAllErrors() {
    while (glGetError() != GL_NO_ERROR) {}
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL Error: " << error
                  << "\tLine: "       << line 
                  << "\tFunction: "   << function <<  std::endl;
        return true;
    }
    return false; 
}

// GLCheckErrorStatus("glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)", 123(whatever line of code));
#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

SDL_Window* initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Could not init SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    gWindow = SDL_CreateWindow(
        "SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gScreenW, gScreenH,
        SDL_WINDOW_OPENGL
    );

    if (!gWindow) {
        std::cerr << "Error creating a window: " << SDL_GetError() << "\n";
        exit(1);
    }

    gSDLcontext = SDL_GL_CreateContext(gWindow);
    if (!gSDLcontext) {
        std::cerr << "Error creating a context: " << SDL_GetError() << "\n";
        exit(1);
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n" << "\n";
        exit(1);
    }
    return gWindow;
}

void VendorInfo() {
    std::cout << "Vendor version:    " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer version:  " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL version:        " << glGetString(GL_VERSION) << "\n";
    std::cout << "Shading Language version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}

void GenVertexObjects_seperateVBO(std::vector<float>& vertices, std::vector<float>& vert_colors) {
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    glGenBuffers(1, &gVertexbufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexbufferObject);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0, 
        2, 
        GL_FLOAT, 
        GL_FALSE,
        0,//2 * sizeof(float),
        (void*)0
    );

    glGenBuffers(1, &gVertexBufferObject2);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject2);
    glBufferData(
        GL_ARRAY_BUFFER,
        vert_colors.size() * sizeof(float),
        vert_colors.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,//3 * sizeof(float),
        (void*)0
    );

    glBindVertexArray(0); // Unbind vao (we used it so, we no longer need it to be active)


   // glDisableVertexAttribArray(0);
   // glDisableVertexAttribArray(1);
}

void GenVertexObjects_sameVBO(std::vector<float> vertexData) {
    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    glGenBuffers(1, &gVertexbufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexbufferObject);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(float),
        vertexData.data(),
        GL_STATIC_DRAW
    );

    const std::vector<GLuint> indexBufferData {2, 0, 1, 3, 2, 1};

    glGenBuffers(1, &gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indexBufferData.size() * sizeof(GLuint),
        indexBufferData.data(),
        GL_STATIC_DRAW
    );
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 
        3, 
        GL_FLOAT, 
        GL_FALSE,
        6 * sizeof(float),
        (GLvoid*)0
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (GLvoid*)(sizeof(float) * 3)
    );

    glBindVertexArray(0);
}

std::string ShaderRetriver(const std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "could not open the file!: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Shader requires a char* type param to be passed.0
// Type -> type of data GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
GLuint ShaderCompiler(const char* src, GLenum type) {
    
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);

    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compilation error: " <<  log << "\n";
    }
    return shader;
}

GLuint ShaderProgramInit() {
    gShaderProgram = glCreateProgram();

    gVertexShader = ShaderCompiler(ShaderRetriver("../src/Basics/Shaders/vertex_shader.glsl").c_str(), GL_VERTEX_SHADER);
    gFragmentShader = ShaderCompiler(ShaderRetriver("../src/Basics/Shaders/fragment_shader.glsl").c_str(), GL_FRAGMENT_SHADER);

    int linked;

    glGetProgramiv(gShaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(gShaderProgram, 512, nullptr, log);
        std::cout << "Program Link error: " << log << "\n";
    }

    glAttachShader(gShaderProgram, gVertexShader);
    glAttachShader(gShaderProgram, gFragmentShader);

    glLinkProgram(gShaderProgram);

    glDeleteShader(gVertexShader);
    glDeleteShader(gFragmentShader);

    return gShaderProgram;
}

void mainLoop() {
    while (gRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                gRunning = false;
        }

        glClearColor(0.f, 0.f, 0.f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(gShaderProgram);
        glBindVertexArray(gVertexArrayObject);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

        SDL_GL_SwapWindow(gWindow);
    }

    SDL_GL_DeleteContext(gSDLcontext);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

struct vertexData {
        std::vector<GLfloat> vertices = {
//       x       y
         0.0f,  0.5f,
        -0.5f, -0.5f,
         0.5f, -0.5f
    };

    std::vector<GLfloat> vertexColors = {
//       r    g    b
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f,
    };

     std::vector<GLfloat> vertexData_withRepeat = {
          // Vertex 3                           // bottom left vertex
          -0.5f,  -0.5f,  0.f,  // location
          1.f,   0.f,  0.f, // color

          // Vertex 2                          // bottom right vertex
         0.5f, -0.5f, 0.f,  // location
          0.f,   1.f,  0.f, // color

          // Vertex 1                           // top left  vertex
          -0.5f,  0.5f, 0.f,  //location  
          0.f,   0.f,  1.f, //color

          // Second Triangle //

          // Vertex 3                           // bottom right vertex
          0.5f,  -0.5f,  0.f,  // location
          0.f,   1.f,  0.f, // color

          // Vertex 2                          // top - right vertex
         0.5f,   0.5f, 0.f,  // location
          0.f,   1.f,  0.f, // color

          // Vertex 1                           // top left vertex
          -0.5f,  0.5f, 0.f,  //location  
          0.f,   0.f,  1.f, //color
    };

    std::vector<GLfloat> vertexData_withoutRepeat = {
          // Vertex 0 | bottom left vertex
          -0.5f,  -0.5f,  0.f,  // location
          1.f,   0.f,  0.f, // color

          // Vertex 1  | bottom right vertex
         0.5f, -0.5f, 0.f,  // location
          0.f,   1.f,  0.f, // color

          // Vertex 2 | top left  vertex
          -0.5f,  0.5f, 0.f,  //location  
          0.f,   0.f,  1.f, //color

          // Vertex 3 | top - right vertex
         0.5f,   0.5f, 0.f,  // location
          0.f,   1.f,  0.f, // color
    };
};

int main() {
    initializeSDL();

    vertexData data;
    //BindVertexBufferVerterArray(vertices, vertexColors);
    GenVertexObjects_sameVBO(data.vertexData_withoutRepeat);

    ShaderProgramInit();

    VendorInfo();

    mainLoop();

    return 0;
}

