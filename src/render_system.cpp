#include "render_system.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// glfw (OpenGL)
#define NOMINMAX
#include <GLFW/glfw3.h>

// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>				// map of character textures

#include <iostream>
#include <assert.h>
#include <fstream>			// for ifstream
#include <sstream>			// for ostringstream

#include "world_system.hpp"

RenderSystem& renderSystem = RenderSystem::instance();


RenderSystem::RenderSystem()
    : window(nullptr), shaderProgram(0), VAO(0), VBO(0), EBO(0)
{
}

RenderSystem::~RenderSystem()
{
    cleanup();
}

bool RenderSystem::initOpenGL(int width, int height, const std::string& title)
{
    // Initialize the window width and height based on the parameters
    windowWidth = width;
    windowHeight = height;

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Error: GLFW initialization failed" << std::endl;
        return false;
    }

    // Allow the window to be resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Set GLFW context version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window with the provided size
    window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Error: Window creation failed" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's OpenGL context current
    glfwMakeContextCurrent(window);
# ifdef __APPLE__
    glfwSwapInterval(0);  // Disable vsync on macOS for better performance
# endif

    // Initialize OpenGL (GL3W)
    if (gl3w_init())
    {
        std::cerr << "Error: gl3w initialization failed" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set the viewport to match the initial window size
    int frame_buffer_width_px, frame_buffer_height_px;
    glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px);
    glViewport(0, 0, frame_buffer_width_px, frame_buffer_height_px);

    // Adjust the projection matrix based on the logical window size (not framebuffer size)
    projection = glm::ortho(
            0.0f, static_cast<float>(windowWidth),   // Left and right
            static_cast<float>(windowHeight), 0.0f,  // Bottom and top
            0.0f, 1.0f  // Near and far planes
    );

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load shaders and set up vertices
    loadShaders("textured", shaderProgram);
    loadShaders("glass", glassShader);
    setupVertices();
    setupGlassVertices();
    setupFrameBuffer();

    // Set GLFW callbacks for input handling
    glfwSetKeyCallback(window, keyCallbackRedirect);
    glfwSetCursorPosCallback(window, mouseMoveCallbackRedirect);
    glfwSetMouseButtonCallback(window, mouseClickCallbackRedirect);
    glfwSetWindowUserPointer(window, this);

    // Handle window resizing dynamically
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int fb_width, int fb_height) {
        // Update the viewport based on the framebuffer size (actual pixels)
        glViewport(0, 0, fb_width, fb_height);

        // Get the window size (logical size, not pixel size)
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);

        // Update the projection matrix based on the logical window size (not framebuffer size)
        glm::mat4 projection = glm::ortho(
                0.0f, static_cast<float>(window_width),   // Left and right
                static_cast<float>(window_height), 0.0f,  // Bottom and top
                0.0f, 1.0f  // Near and far planes
        );

        // Set the updated projection matrix (bind shader program and set it here)
        // glUniformMatrix4fv(projection_location, 1, GL_FALSE, &projection[0][0]);
    });

    // Initialize SDL audio subsystem
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Error: SDL initialization failed (" << SDL_GetError() << ")" << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        std::cerr << "Error: SDL_mixer initialization failed (" << Mix_GetError() << ")" << std::endl;
        return false;
    }
    std::cout << "Audio channels: " << Mix_AllocateChannels(-1) << std::endl;

    return true;
}

void RenderSystem::loadShaders(const std::string& program, GLuint& shader_program) {
    std::string vertexCode = readShaderFile(shader_path(program + ".vs.glsl"));
    std::string fragmentCode = readShaderFile(shader_path(program + ".fs.glsl"));

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    char infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Error: Vertex Shader compilation failed\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Error: Fragment Shader compilation failed\n" << infoLog << std::endl;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertexShader);
    glAttachShader(shader_program, fragmentShader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, nullptr, infoLog);
        std::cerr << "Error: Shader Program linking failed\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

std::string RenderSystem::readShaderFile(const std::string& filePath)
{
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open())
    {
        std::cerr << "Error: Failed to open shader file " << filePath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

// TODO
bool RenderSystem::fontInit(const std::string& font_filename, unsigned int font_default_size) {
    
    // read in our shader files
    std::string vertexShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.vs.glsl"));
    std::string fragmentShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.fs.glsl"));
    const char* vertexShaderSource_c = vertexShaderSource.c_str();
    const char* fragmentShaderSource_c = fragmentShaderSource.c_str();

    // enable blending or you will just get solid boxes instead of text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // font buffer setup
    glGenVertexArrays(1, &m_font_VAO);
    glGenBuffers(1, &m_font_VBO);

    // font vertex shader
    unsigned int font_vertexShader;
    font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(font_vertexShader, 1, &vertexShaderSource_c, NULL);
    glCompileShader(font_vertexShader);

    // font fragement shader
    unsigned int font_fragmentShader;
    font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(font_fragmentShader, 1, &fragmentShaderSource_c, NULL);
    glCompileShader(font_fragmentShader);

    // font shader program
    m_font_shaderProgram = glCreateProgram();
    glAttachShader(m_font_shaderProgram, font_vertexShader);
    glAttachShader(m_font_shaderProgram, font_fragmentShader);
    glLinkProgram(m_font_shaderProgram);

    // apply orthographic projection matrix for font, i.e., screen space
    glUseProgram(m_font_shaderProgram);
    glm::mat4 projection_ = glm::ortho(0.0f,static_cast<float>(getWindowWidth()), 0.0f, static_cast<float>(getWindowHeight()));
    GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
    assert(project_location > -1);
    std::cout << "project_location: " << project_location << std::endl;
    glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection_));

    // clean up shaders
    glDeleteShader(font_vertexShader);
    glDeleteShader(font_fragmentShader);

    // init FreeType fonts
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
        return false;
    }

    // extract a default size
    FT_Set_Pixel_Sizes(face, 0, font_default_size);

    // disable byte-alignment restriction in OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load each of the chars - note only first 128 ASCII chars
    for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // std::cout << "texture: " << c << " = " << texture << std::endl;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
            (char)c
        };
        m_ftCharacters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // clean up
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // bind buffers
    glBindVertexArray(m_font_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // release buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void RenderSystem::renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans)
{
    // use program, load variables, bind to VAO, then iterate thru chars

    // activate the shader program
    glUseProgram(m_font_shaderProgram);

    // get shader uniforms
    GLint textColor_location =
        glGetUniformLocation(m_font_shaderProgram, "textColor");
    glUniform3f(textColor_location, color.x, color.y, color.z);

    GLint transformLoc =
        glGetUniformLocation(m_font_shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

    glBindVertexArray(m_font_VAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = m_ftCharacters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::setupVertices()
{
    float vertices[] = {
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f 
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3 
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void RenderSystem::setupGlassVertices()
{
    float glassVertices[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f, 
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f  
    };

    unsigned int glassIndices[] = {
        0, 1, 2,
        2, 3, 0 
    };

    glGenVertexArrays(1, &glassVAO);
    glBindVertexArray(glassVAO);

    glGenBuffers(1, &glassVBO);
    glBindBuffer(GL_ARRAY_BUFFER, glassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassVertices), glassVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &glassEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glassEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glassIndices), glassIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    gl_has_errors();
}

void RenderSystem::closeWindow() {
    glfwSetWindowShouldClose(window, true);
}


void RenderSystem::renderLoop()
{
    float lastTime = static_cast<float>(glfwGetTime());

    // FPS stuff
    float FPS_Last_Time = 0;
    std::string prev_FPS_string = "";

    // setup fonts
    std::string font_filename = PROJECT_SOURCE_DIR +
        std::string("data/fonts/Share-Bold.ttf");
    unsigned int font_default_size = 48;
    fontInit(font_filename, font_default_size);
    glm::vec3 font_color = glm::vec3(1.0, 1.0, 1.0);
    glm::mat4 font_trans = glm::mat4(1.0f);
    font_trans = glm::scale(font_trans, glm::vec3(0.5, 0.5, 1.0));

    while (!glfwWindowShouldClose(window))
    {
        Uint32 startTicks = SDL_GetTicks();
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (gameStateManager)
        {
            gameStateManager->update(deltaTime);
           
            gameStateManager->render();
        }
        // FPS stuff
        calculateFPS();
        float FPS_Delta_Time = currentTime - FPS_Last_Time;
     
        if (FPS_Delta_Time >= 1.0)
        {
            std::string FPS = std::to_string(static_cast<int>(fps));
            std::string FPS_String = "FPS: " + FPS;
            prev_FPS_string = FPS_String;
            FPS_Last_Time = currentTime;
            if (Show_FPS) {
                renderText(FPS_String, static_cast<float>(getWindowWidth() * 1.90), static_cast<float>(getWindowHeight() * 1.95), 1.0f, font_color, font_trans);
            }
        }
        else {
            if (Show_FPS) {
                renderText(prev_FPS_string, static_cast<float>(getWindowWidth() * 1.90), static_cast<float>(getWindowHeight() * 1.95), 1.0f, font_color, font_trans);
            }
        }
        // FPS limiting 
        Uint32 frameTicks = SDL_GetTicks() - startTicks;
        if (1000 / maxFPS > frameTicks) {
            SDL_Delay((1000 / maxFPS) - frameTicks);
        }
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void RenderSystem::setupFrameBuffer() {
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width_px, window_height_px, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::captureScreen() {
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width_px, window_height_px, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::doGlassBreakTransition(int count, size_t total) {
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_width_px, window_height_px, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(glassShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glUniform1f(glGetUniformLocation(glassShader, "keyframe"), static_cast<float>(count) / total);
    glUniform1i(glGetUniformLocation(glassShader, "screenTexture"), 0);
    glBindVertexArray(glassVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(0);
    glfwSwapBuffers(renderSystem.getWindow());
    glfwPollEvents();
    gl_has_errors();
}

void RenderSystem::drawEntity(const Sprite& sprite, const TransformComponent& transform, float transparency) {
    glUseProgram(shaderProgram);

    // Create model matrix from transform component
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform.position);
    model = glm::rotate(model, glm::radians(transform.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, transform.scale * glm::vec3(1.f));

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform1f(glGetUniformLocation(shaderProgram, "visibility"), transparency);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, *sprite.textureID);

    // Bind VAO and draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

std::future<Image> loadImageData(const std::string& filePath, std::atomic<int>& count) {
    return std::async(std::launch::async, [filePath, &count]() {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(textures_path(filePath).c_str(), &width, &height, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load texture at path: " << filePath << std::endl;
            return Image{0, 0, 0, nullptr};
        }
        count++;
        return Image{ width, height, nrChannels, data };
        });
}

Sprite bindTexture(const Image& image) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width = image.width;
    int height = image.height;
    int nrChannels = image.nrChannels;
    unsigned char* data = image.data;
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1) {
            format = GL_RED;
        }
        else if (nrChannels == 3) {
            format = GL_RGB;
        }
        else if (nrChannels == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    return Sprite{ textureID, static_cast<float>(width), static_cast<float>(height) };
}

Sprite RenderSystem::loadTexture(const std::string& filePath)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(textures_path(filePath).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1) {
            format = GL_RED;
        }
        else if (nrChannels == 3) {
            format = GL_RGB;
        }
        else if (nrChannels == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cerr << "Failed to load texture at path: " << filePath << std::endl;
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    stbi_image_free(data);

    return Sprite{textureID, static_cast<float>(width), static_cast<float>(height)};
}

void RenderSystem::loadPlayerMeshes(Entity playerEntity) {
    const std::unordered_map<PlayerState, std::string> playerMeshPaths = {
            {PlayerState::IDLE, mesh_path("mesh_walk_3.obj")},
            {PlayerState::WALKING, mesh_path("mesh_walk_3.obj")},
            {PlayerState::JUMPING, mesh_path("mesh_jump_3.obj")},
            {PlayerState::ATTACKING, mesh_path("mesh_attack_3.obj")},
            {PlayerState::HIT, mesh_path("mesh_hit.obj")},
    };

    // Check if the player already has a PlayerMeshes component
    if (!registry.playerMeshes.has(playerEntity)) {
        registry.playerMeshes.emplace(playerEntity, PlayerMeshes{});
    }
    auto& playerMeshesComponent = registry.playerMeshes.get(playerEntity);

    // Load each mesh for the specified player states
    for (const auto& playerMeshPath : playerMeshPaths) {
        PlayerState state = playerMeshPath.first;
        std::string path = playerMeshPath.second;
        Mesh mesh;
        if (Mesh::loadFromOBJFile(path, mesh.vertices, mesh.vertex_indices, mesh.edges, mesh.original_size)) {
            playerMeshesComponent.stateMeshes.emplace(state, std::move(mesh));
        } else {
            std::cerr << "Error: Failed to load mesh for state " << static_cast<int>(state) << " from " << path << std::endl;
        }
    }
}

const Mesh& RenderSystem::getPlayerMesh(Entity playerEntity, PlayerState state) {
    static Mesh emptyMesh;
    if (registry.playerMeshes.has(playerEntity)) {
        const auto& playerMeshes = registry.playerMeshes.get(playerEntity).stateMeshes;
        auto it = playerMeshes.find(state);
        if (it != playerMeshes.end()) {
            return it->second;
        }
    }
    return emptyMesh;
}


void RenderSystem::cleanup()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void RenderSystem::keyCallbackRedirect(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
    RenderSystem* renderSystem_ = static_cast<RenderSystem*>(glfwGetWindowUserPointer(wnd));
    if (renderSystem_ && renderSystem_->gameStateManager)
    {
        renderSystem_->gameStateManager->on_key(key, scancode, action, mods);
    }
}

void RenderSystem::mouseMoveCallbackRedirect(GLFWwindow* wnd, double xpos, double ypos) {
    RenderSystem* renderSystem_ = static_cast<RenderSystem*>(glfwGetWindowUserPointer(wnd));
    if (renderSystem_ && renderSystem_->gameStateManager) {
        int actualWidth, actualHeight;
        glfwGetWindowSize(wnd, &actualWidth, &actualHeight);

        // Scale mouse position based on logical window size
        glm::vec2 scaledPos = glm::vec2(
                xpos * renderSystem_->windowWidth / actualWidth,
                ypos * renderSystem_->windowHeight / actualHeight
        );

        renderSystem_->gameStateManager->on_mouse_move(scaledPos);
    }
}

void RenderSystem::mouseClickCallbackRedirect(GLFWwindow* wnd, int button, int action, int mods) {
    RenderSystem* renderSystem_ = static_cast<RenderSystem*>(glfwGetWindowUserPointer(wnd));
    if (renderSystem_ && renderSystem_->gameStateManager) {
        double xpos, ypos;
        glfwGetCursorPos(wnd, &xpos, &ypos);
        renderSystem_->gameStateManager->on_mouse_click(button, action, glm::vec2(xpos, ypos), mods);
    }
}

void RenderSystem::setGameStateManager(GameStateManager* gsm) {
    gameStateManager = gsm;
}

GameStateManager* RenderSystem::getGameStateManager() const
{
    return gameStateManager;
}

GLFWwindow* RenderSystem::getWindow() const
{
    return window;
}

int RenderSystem::getWindowWidth() const
{
    return windowWidth;
}

int RenderSystem::getWindowHeight() const
{
    return windowHeight;
}

void RenderSystem::calculateFPS() {
    static const int NUM_SAMPLES = 10;
    static float frameTimes[NUM_SAMPLES];
    static int currentFrame = 0;

    static float prevTicks = static_cast<float>(SDL_GetTicks());
    float currentTicks = static_cast<float>(SDL_GetTicks());

    frameTime = currentTicks - prevTicks;
    frameTimes[currentFrame % NUM_SAMPLES] = frameTime;
    prevTicks = currentTicks;

    int count;
    currentFrame++;

    if (currentFrame < NUM_SAMPLES) {
        count = currentFrame;
    }
    else {
        count = NUM_SAMPLES;
    }

    float frameTimeAverage = 0;
    for (int i = 0; i < count; i++) {
        frameTimeAverage += frameTimes[i];
    }

    frameTimeAverage /= count;

    if (frameTimeAverage > 0) {
        fps = 1000.0f / frameTimeAverage;
    }
    else {
        fps = 60.0f;
    }
}