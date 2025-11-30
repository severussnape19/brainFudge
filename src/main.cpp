#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>

int main()
{
    // Talks to os, loads X11/ Wayland libraries
    // Sets up event handling
    // sets up window handling
    // prepares everything before creating a window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_INIT error: " << SDL_GetError() << "\n";
        return -1;
    }

    // Telling SDL which OpenGL version we want to use
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


    // Creates a window
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 + OpenGL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL // window can hold an OpenGL 'context'
    );

    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << "\n";
        return -1;
    }

    // A context is like a private OpenGL universe tied to this window
    /*
    universe has :
    # Shaders
    # VBOs (vertex buffer object)
    # VAOs (vertex array object)
    # Textures
    # draw calls
    # GPU state
    */
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "GL Context Error: " << SDL_GetError() << "\n";
        return -1;
    }

    if (!glContext) {
        std::cerr << "GL Context Error: " << SDL_GetError() << "\n";
        return -1;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    bool running = true;
    SDL_Event event;

    float vertices[] = {
        0.0f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

/*
There is no direct connection function.
When the VAO is bound and the VBO is bound,
then calling glVertexAttribPointer links them
because both are active at that moment.
*/

    // VAO (stores the vertex layout / attribute layout)
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); // Binds it (makes it active)
    // becomes the active configuration container <--------------------@
    //                                                                 |
    // VBO (stores the vertex data on the gpu) (raw data)              |
    GLuint vbo;             //                                         |
    glGenBuffers(1, &vbo);  //                                         |
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // current active array buffer |
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 
    // Allocate GPU memory for this buffer

    // Describes the data format to openGL
    // CONNECTS VBO to VAO
    glVertexAttribPointer(
        0,                // attribute index in shader (layout(location = 0))
        2,                // how many floats per vertex
        GL_FLOAT,         // data type
        GL_FALSE,         // Do not normalize
        2 * sizeof(float),// stride (distance between each vertex)
        (void*)0          // offset in buffer (starting at the beginning of the VBO)
        // nullptr of type void* representing "offset = 0 bytes into the buffer"
    );

    glEnableVertexAttribArray(0); 
    // attribute is a piece of data that belongs to each vertex
    // eg : position, color, texture coord etc
    // now we only have position (single attribute)
    // at idx 0, we have the vertices (positions)
    // Now, VAO marks slot 0 as "Active" and ready for the shader

    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.8, 0.1, 0.2, 1.0);
        }
    )";

    // the parameters are placeholders for future values. GLuint is the return type.
    // src -> pointer to a C-style string containing GLSL shader source.
    // GLenum type -> an OpenGL enum saying what kind of shader. (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc).
    // So basically, compileShader is a small function that takes shader source + shader type and returns a compiled shader handle
    auto compileShader = [](const char* src, GLenum type) -> GLuint
    {
        /*
        # In OpenGL we do not hold actual shader objects in code.
        # The shader lives inside the openGL driver / GPU, not in the process memory as a c++ object
        # So, instead, openGL gives an inter ID that refers to that shader.

        -> GLuint is just a typedef for an unsigned int from openGL headers.
        -> Shader is not the shader itself. Its just a number the driver uses as akey to find the shader internally.
        */
       // "type" is GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
       // Asks openGL to create a shader of a specific type
       // This creates a new Shader object of that type internally and returns a GLuint (an ID)
       // The handle is stored in shader var
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr); // attaches the GLSL source code to the shader object
        /*
        Args:
             Shader  : the handle
               1    : number of strings you are passing (multiple can be passed)
             &src   : Address of the const char*
            nullptr : optional array of string lengths. Its null here so gl configures with strlen
        */
        glCompileShader(shader); // compiles from the given data

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // queries properties of the shader
        // GL_COMPILE_SHADER -> returs 1 or 0 and write it into success var
        if (!success) {
            // Buffer is created to store the error message
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log); // Returns the compilation log string for this shader.
            // nullptr for length of out parameter (we don't care)
            // buffer where gl writes the message
            std::cerr << "Shader compilation Error: \n" << log << "\n";
        }
        return shader;
        // returned shader handle can be attached to a program (glAttachShader)
        // Later Deleted after linking (glDeleteShader)
    };

    GLuint vertexShader = compileShader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    /*
    # creates an empty program object
    # Returns a GLuint handle again
    # More like an empty .exe before linking object files into it.
    */
    glAttachShader(shaderProgram, vertexShader); 
    glAttachShader(shaderProgram, fragmentShader);
    // The above two lines do not link just yet! (Does NOT validate or compile anything)
    // It's like putting them into the linker's input box so it could link them LATER
    glLinkProgram(shaderProgram);
    // Now links both the vertex shader and fragment shader
    /*
    # Takes compiled machine code from both the shaders
    # Checks that the vertex shader outputs match fragment shader inputs
    # Builds GPU's internal pipeline (vertex -> Rasterize -> Fragment)
    # Optimizes the combined shader program
    # Produces a findal GPU-executable shader pipeline
    */

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // These lines do NOT delete the actual shader code used by the program
    /*
    # When the shaders are linked + attatched, the program carries its own full copy
    # The shaders become a dead weight so, they are deleted. Frees memory and avoids memory leaks
    */


    // Usual event loop
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
Vertex Shaders and Fragment shaders

# Vertex shader becomes an input to the fragment shader yeah.
# But only the pre-vertex outputs you explicitly pass become inputs to the fragment shader.

# The triangles are made by the fixed-function rasterizer, not shader
*/