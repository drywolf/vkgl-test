// original GL sample code: https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/aebe72f254e91567e4c4fdfd01c840a576e831e3/src/4.advanced_opengl/5.1.framebuffers/framebuffers.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl_debug_output.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#undef STB_IMAGE_IMPLEMENTATION

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <vk-render.h>

#include <filesystem>
#include <iostream>
#include <map>

#include "vkgl_options.h"

void resize_window(int width, int height);
void update_window_title(GLFWwindow* window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// z-buffer position of the drawn Vulkan Quad (can be changed interactively via mouse-scroll)
float vk_quad_z = 0.95f;

// camera
//Camera camera(glm::vec3(0, +1.0f, +10.0f), glm::vec3(0, 1, 0)); // simple straight view
Camera camera(glm::vec3(4.74534, 0.958979, 4.76834), glm::vec3(0, 1, 0), -135.7, -15.1);

float lastX = 0;
float lastY = 0;
bool firstMouse = true;

glm::mat4 projection;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void gl_draw_mesh(GLuint vao_id, GLuint num_vertices, const glm::mat4& model_matrix, const Shader& shader, GLuint texture_id);
void print_gl_default_framebuffer_info();
bool check_gl_capability();

std::tm get_time_now()
{
    std::time_t t = std::time(0); // get time now
    std::tm now = *std::localtime(&t);
    return now;
}

int main(int argc, char* argv[])
{
    VkGlAppOptions options;

    // IMPORTANT: MSAA sample-count must be a power-of-two number !!!
    int msaa_sample_count =
        options.enable_msaa
        ? 4
        : 1;

    if (
        msaa_sample_count < 1
        || (msaa_sample_count & (msaa_sample_count - 1)) != 0
        )
    {
        std::cout << "FATAL ERROR: VkGlApp::initialize() msaa_sample_count must be a power-of-two number or '1' !!!" << std::endl;
        std::abort();
    }

    auto start_time = get_time_now();

    auto& logger = std::cout;
    logger << "--------------------------------------------------------------------------------" << std::endl;
    logger << " APP START - " << std::put_time(&start_time, "%a %b %d %H:%M:%S %Y") << std::endl;
    logger << "--------------------------------------------------------------------------------" << std::endl;

    lastX = (float)options.width / 2.0;
    lastY = (float)options.height / 2.0;

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // we do not handle window resizing in this prototype -> disable GLFW window resize/maximize

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_USE_DEBUG_CONTEXT);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(options.width, options.height, "Vulkan-GL Interop", NULL, NULL);
    if (window == NULL)
    {
        logger << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to not capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    const int glad_init_result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (!glad_init_result)
    {
        logger << "ERROR: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    if (!check_gl_capability())
    {
        logger << "ERROR: Insufficient GL version/extension-features" << std::endl;
        return -1;
    }

    print_gl_default_framebuffer_info();

    logger << "Enabling GL Debug Output..." << std::endl;
    if (!gl_enable_debug_output())
    {
        logger << "ERROR: Failed to initialize GL DEBUG OUTPUT" << std::endl;
    }

    GLuint gl_color_tex = 0, gl_depth_tex = 0;

    // initialize vulkan & interop
    if (!vk_init(
        options.width,
        options.height,
        msaa_sample_count, // NOTE: this value will be modified, if the GPU capabilities do not support the desired sample-count
        options.ENABLE_VULKAN_VALIDATION_LAYER,
        &gl_color_tex,
        &gl_depth_tex
    ))
    {
        vk_shutdown();
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader("5.1.framebuffers.vs", "5.1.framebuffers.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    // cube VAO
    GLuint cubeVAO = 0, cubeVBO = 0;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    // plane VAO
    GLuint planeVAO = 0, planeVBO = 0;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/container.jpg").c_str());
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/metal.png").c_str());

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    // framebuffer configuration
    // -------------------------
    GLuint vkgl_framebuffer;
    glGenFramebuffers(1, &vkgl_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, vkgl_framebuffer);

    const auto target = msaa_sample_count > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // attach interop COLOR render-texture to GL FBO
    GL_CHECK(glBindTexture(target, gl_color_tex));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, gl_color_tex, 0));
    //GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    //GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glBindTexture(target, 0));

    // attach interop DEPTH render-texture to GL FBO
    GL_CHECK(glBindTexture(target, gl_depth_tex));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, gl_depth_tex, 0));
    GL_CHECK(glBindTexture(target, 0));

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    const auto fbo_state = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fbo_state != GL_FRAMEBUFFER_COMPLETE)
    {
        logger << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        return 1;
    }

    // Call resize_window() manually once, to set up the camera projection matrix & GL viewport dimensions
    resize_window(options.width, options.height);
    update_window_title(window);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // clear the window framebuffer RED, just for potential debugging purposes
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // clear color & depth via vulkan
        vk_clear_fbo();

        // render
        // ------
        // bind vkgl interop framebuffer and draw scene as we normally would
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, vkgl_framebuffer));

        // clear color & depth via GL (this is just for testing)
        //glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw some GL
        gl_draw_mesh(cubeVAO, 36, glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, -3.0f)), shader, cubeTexture);
        gl_draw_mesh(cubeVAO, 36, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)), shader, cubeTexture);

        // then draw some VK
        vk_draw_quad(vk_quad_z);

        // then draw some GL again
        gl_draw_mesh(cubeVAO, 36, glm::translate(glm::mat4(1.0f), glm::vec3(+3.0f, 0.0f, +3.0f)), shader, cubeTexture);
        gl_draw_mesh(planeVAO, 6, glm::mat4(1.0f), shader, floorTexture);

        // now copy the VK-GL FBO results to the GLFW window framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, vkgl_framebuffer);   // vkgl interop FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);                  // window swapchain framebuffer

        // IMPORTANT: resolving MSAA Depth + Stencil buffer attachments can only work if the Vulkan and OpenGL formats for depth & stencil do match EXACTLY !!!
        // (otherwise you will get an OpenGL error here)
        // Currently the only generally available Depth-Stencil format for all IHVs [NVidia, AMD, Intel] might be VK_FORMAT_D32_SFLOAT_S8_UINT.
        // see also: https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/depth.adoc#depth-formats
        GL_CHECK(glBlitFramebuffer(0, 0, options.width, options.height, 0, 0, options.width, options.height,
            //GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        ));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteFramebuffers(1, &vkgl_framebuffer);

    vk_shutdown();

    glfwTerminate();

    auto shutdown_time = get_time_now();

    logger << "--------------------------------------------------------------------------------" << std::endl;
    logger << " GRACEFUL APP SHUTDOWN - " << std::put_time(&shutdown_time, "%a %b %d %H:%M:%S %Y") << std::endl;
    logger << "--------------------------------------------------------------------------------" << std::endl;

    return 0;
}

void resize_window(int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);

    projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    resize_window(width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void update_window_title(GLFWwindow* window)
{
    const char* gl_version = (const char*)glGetString(GL_VERSION);
    std::string title = std::string() + "[vkgl-test @ " + gl_version + "] vk_quad_z = " + std::to_string(vk_quad_z) + " ... change via mouse scroll-wheel";
    glfwSetWindowTitle(window, title.c_str());
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    vk_quad_z += yoffset * 0.001f;
    vk_quad_z = std::clamp(vk_quad_z, 0.0f, 1.0f);

    update_window_title(window);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // can be used to capture the current camera position (make it a new starting position)
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        std::cout << "Camera camera(glm::vec3(" << camera.Position.x << "," << camera.Position.y << "," << camera.Position.z << "), glm::vec3(0, 1, 0), " << camera.Yaw << ", " << camera.Pitch << ");" << std::endl;
    }
}

void gl_draw_mesh(GLuint vao_id, GLuint num_vertices, const glm::mat4& model_matrix, const Shader& shader, GLuint texture_id)
{
    shader.use();
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setMat4("model", model_matrix);

    glBindVertexArray(vao_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

bool check_gl_capability()
{
    const char* gl_version = (const char*)glGetString(GL_VERSION);
    std::cout << "GL_VERSION: " << gl_version << std::endl;

    const char* gl_renderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "GL_RENDERER: " << gl_renderer << std::endl;

#define gl_required_ext(ext_name) { (#ext_name + 5), ext_name }

    const std::map<const char*, bool> gl_required_extensions =
    {
        gl_required_ext(GLAD_GL_EXT_memory_object),
#if WIN32
        gl_required_ext(GLAD_GL_EXT_memory_object_win32),
#else
        gl_required_ext(GLAD_GL_EXT_memory_object_fd),
#endif
        gl_required_ext(GLAD_GL_EXT_semaphore),
#if WIN32
        gl_required_ext(GLAD_GL_EXT_semaphore_win32),
#else
        gl_required_ext(GLAD_GL_EXT_semaphore_fd),
#endif
    };

    bool all_supported = true;
    for (auto& [name, supported] : gl_required_extensions)
    {
        std::cout << name << ": " << supported << std::endl;
        all_supported &= supported;
    }

    return all_supported;
}

// Print some info about GL default framebuffer (framebuffer owned by the window/swapchain)
void print_gl_default_framebuffer_info()
{
    auto& logger = std::cout;
    logger << "----------------------------------------" << std::endl;
    logger << "Default GL Framebuffer format:" << std::endl;

    const auto target = GL_FRAMEBUFFER;
    glBindFramebuffer(target, 0);

    GLint rgba_bits[4] = {};
    const auto attachment = GL_BACK_LEFT;
    glGetFramebufferAttachmentParameteriv(target, attachment, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &rgba_bits[0]);
    glGetFramebufferAttachmentParameteriv(target, attachment, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &rgba_bits[1]);
    glGetFramebufferAttachmentParameteriv(target, attachment, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &rgba_bits[2]);
    glGetFramebufferAttachmentParameteriv(target, attachment, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &rgba_bits[3]);

    logger << "Red:     " << rgba_bits[0] << " bits" << "\n";
    logger << "Green:   " << rgba_bits[1] << " bits" << "\n";
    logger << "Blue:    " << rgba_bits[2] << " bits" << "\n";
    logger << "Alpha:   " << rgba_bits[3] << " bits" << "\n";

    GLint depth_bits = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);

    logger << "Depth:   " << depth_bits << " bits" << "\n";

    GLint stencil_bits = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_bits);

    logger << "Stencil: " << stencil_bits << " bits" << "\n";
    logger << "----------------------------------------" << std::endl;
}
