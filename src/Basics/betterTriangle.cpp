#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <array>
#include <fstream>
#include <sstream>
#include <string>

// Globals
int gScreenHeight = 600;
int gScreenWidth = 800;
SDL_Window *gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Main loop flag
bool gQuit = false; // if true, we quit

// Shader
// The following stores a unique ID for the graphics pipeline
// Program object that will be ised for out OpenGL draw calls
GLuint gGraphicsPipelineShaderProgram = 0;

/*
// OpenGL objects
// Vertex Array Object (VAO)
// Vertex array objects encapsulate all of the items needed to rended an object.
# For example, we may have multiple vertex buffer objects (VBO) related to render one object.
The VAO allows us to setup the OpenGL state to render that object using the correct layout
and correct buffers with one call after being setup.
*/
GLuint gVertexArrayObject = 0;
// Vertex Buffer object (VBO)
// Vertex Buffer Objects store information relating to the vertices (eg. Positions, normals, textures)
// VBOs are our mechanism for arranging geometry on the GPU
GLuint gVertexBufferObject = 0;

// Shaders
/*
# Here we setup two shaders, a vertex shader and a fragment shader.
# At a minimum, every modern OpenGL program needs a vertex and a fragment shader.

# OpenGL provides functions that will compile the shader source code at run-time.
# (These are generally stored as strings)
*/

// Vertex shader executes once per vertex and will be in charge of the final position of the vertex. 
std::string loadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file! " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Fragment Shader
// The fragment shader executes once per fragment (i.e roughly for every pixelthat will be rasterized),
// and in part determines the final color that will be sent to the screen.
const std::string gVertexShaderSource =
    loadFile("../src/Basics/Shaders/vertex_shader.glsl");

const std::string gFragmentShaderSource =
    loadFile("../src/Basics/Shaders/fragment_shader.glsl");
GLuint CompileShader(GLuint type, const std::string& source) {
    GLuint shaderObject;
    if (type == GL_VERTEX_SHADER) {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    } else if (type == GL_FRAGMENT_SHADER) {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    // Source of our shader
    glShaderSource(shaderObject, 1, &src, nullptr);
    // Now compile the shader
    glCompileShader(shaderObject);

    // retrieve the result of our compilation
    int result;
    // Our goal with glGetShaderiv is to retrieve the compilation status
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length];
        glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

        if (type == GL_VERTEX_SHADER) {
            std::cout << "ERROR: GL_VERTEX_SHADER compilation failed\n" << errorMessages << "\n";
        } else if (type == GL_FRAGMENT_SHADER) {
            std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed\n" << errorMessages << "\n";
        }

        delete[] errorMessages;

        // Delete the broken shader
        glDeleteShader(shaderObject);
    }

    return shaderObject;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, 
                         const std::string& fragmentShaderSource) {
        GLuint programObject = glCreateProgram();
        
        GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
        
        glAttachShader(programObject, myVertexShader);
        glAttachShader(programObject, myFragmentShader);

        glLinkProgram(programObject);

        // Validate Program
        glValidateProgram(programObject);
        //
        return programObject;
}

void CreateGraphicsPipeline() {
    gGraphicsPipelineShaderProgram = createShaderProgram(gVertexShaderSource, gFragmentShaderSource);
}

void GetOpenGLVersionInfo() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void VertexSpecification() {
    /*
    // Geometry Data

    # Here we are going to store x, y and z position attributes within VertexPositions for the data
    # For now, this information is just stored in the CPU, and we are going to store this data on the GPU shortly.
    # In a call to glBufferData which will store this information into a vertex buffer object.
    */
    const std::array<GLfloat, 9> vertexPositions = {
        // x      y    z
        -0.8f,  -0.8, 0.f, // vertex 1
        0.8f, -0.8f, 0.0f, // vertex 2
        0.0f, 0.8f, 0.0f // vertex 3 
    };

    // Vertex Array Object (VAO) setup
    /*
    # we can think of VAOs as a 'wrap around' that encapsulates all of the vertex Buffer objects,\n
    in the sense that it encapsulates all VBO state that we setting up

    # Thus, it is also important that we glBindVertexArray (select the VAO we want to use) before our VBO operations
    */
    glGenVertexArrays(1, &gVertexArrayObject);
    // We bind to the VAO that we want to work with
    glBindVertexArray(gVertexArrayObject);

    // Start Generating our VBO
    glGenBuffers(1, &gVertexBufferObject);
    // Next we will glBindBuffer
    // Bind is equivalent to 'selecting the active buffer object' that we want to work witin OpenGL
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    // Now, in our currently binded buffer, we populate the data from our 'vertexPositions' (which is on the CPU),
    // onto a buffer that will live on the GPU
    glBufferData(
        GL_ARRAY_BUFFER,                          // Kind of buffer we are working with
        vertexPositions.size() * sizeof(GLfloat), // Size of the data in bytes
        vertexPositions.data(),                   // Raw array data
        GL_STATIC_DRAW                            // How we intend to use the data
    );

    // For our given vertex array object, We need to tell OpenGL 'how' the information in our buffer will be used
    glEnableVertexAttribArray(0);
    // For the specific attribute in our vertex specification, we use 'glVertexAttribPointer' to figure out how we are going to move through the data
    glVertexAttribPointer(
        0,        // Attribute 0 corresponds to the enabled glEnableVertexAttribArray
        3,        // Number of components
        GL_FLOAT, // Type
        GL_FALSE, // Is data normalized? 
        3 * sizeof(GLfloat),        // Stride
        (void*)0  // Offset 
    );

    // Unbind our currently bound vertex array object
    glBindVertexArray(0);
    // Disable any attributes we opened in our vertex attribute array,
    // As we do not want to leave themd in our vertex attribute array,
    glDisableVertexAttribArray(0);
}
/*
Initialization of the graphics application. Typically this will involve setting up a window and the OpenGL context
(With the appropriate version)
@ return void
*/
void InitializeProgram() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL2 could not initalize video sub-system" << "\n";
        exit(1);
    }

    // Setup the OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // core profile gets rid of the old stuff found in the compatibility profile
    
    // We want to request a double buffer for smooth updating.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Creates an application window using openGL that supports SDL
    gGraphicsApplicationWindow = SDL_CreateWindow(
        "OpenGL Window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gScreenWidth, gScreenHeight,
        SDL_WINDOW_OPENGL
    );

    // Check if window did not create
    if (!gGraphicsApplicationWindow) {
        std::cerr << "SDL window was not able to be created." << "\n";
        exit(1);
    }

    // Create an OpenGL Graphics context
    gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
    
    if (!gOpenGLContext) {
        std::cerr << "OpenGL context not available.\n";
        exit(1);
    }

    // Initialize the glad library 
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "Glad could not get initialized." << std::endl;
        exit(1);
    }
    
    GetOpenGLVersionInfo();
}

void Input() {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            std::cout << "Closing!" << std::endl;
            gQuit = true; 
        }
    }
}

void PreDraw() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw() {
    // Enable our attributes
    glBindVertexArray(gVertexArrayObject);

    // Select the vertex buffer object we want to enable
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Render data
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // // Stop using our current graphics pipeline
    glUseProgram(0);
}

void MainLoop() {
    
    while (!gQuit) {
        Input();

        PreDraw();

        Draw();
        
        // updates the screen
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}

void cleanUp() {
    SDL_DestroyWindow(gGraphicsApplicationWindow);

    SDL_Quit();
}

int main(int argc, char* []) {
    // 1) Setup the Graphics Program
    InitializeProgram();

    // 2) Setup our geometry
    VertexSpecification();

    // 3) Create our graphics pipeline
    // -At a minimum, this means the vertex and fragment shader
    CreateGraphicsPipeline();

    // Calls the main application loop
    MainLoop();

    // Call the cleanup function when our program terminates
    cleanUp();

    return 0;
}