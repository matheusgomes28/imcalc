#include <imgui.h>
// needs to be after imgui
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GL/glew.h> // Initialize with glewInit()
// After GL
#include <GLFW/glfw3.h>

// Standard lib includes
#include <array>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    enum class TextButtonPurpose
    {
        AddToBuffer,
        ClearAll
    };

    struct TextButton
    {
        std::string text;
        int col;
        int row;
        TextButtonPurpose purpose;
    };

    struct TextBuffer
    {
        std::array<char, sizeof(char) * 48> buff;
        int curr_index;
    };

    constexpr std::string_view vertex_shader{
        R"(
#version 130
in mediump vec3 point;
in mediump vec2 texcoord;
out mediump vec2 UV;
void main()
{
    gl_Position = vec4(point, 1);
    UV          = texcoord;
}
    )"};

    constexpr std::string_view fragment_shader{
        R"(
#version 130
in mediump vec2 UV;
out mediump vec3 fragColor;
uniform sampler2D tex;
void main()
{
  fragColor = texture(tex, UV).rgb;
}
    )"};


    bool createShaders(std::string_view vertex, std::string_view fragment)
    {
        {
            GLuint vertexShader           = glCreateShader(GL_VERTEX_SHADER);
            GLint vertexShaderSize        = vertex.size();
            const char* vertexDataPointer = vertex.data();
            glShaderSource(vertexShader, 1, &vertexDataPointer, &vertexShaderSize);
            glCompileShader(vertexShader);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                std::cout << "vertex shader was probably not compiled\n";
                return false;
            }
        }

        {
            GLuint fragmentShader           = glCreateShader(GL_FRAGMENT_SHADER);
            GLint fragmentShaderSize        = fragment.size();
            const char* fragmentDataPointer = fragment.data();
            glShaderSource(fragmentShader, 1, &fragmentDataPointer, &fragmentShaderSize);
            glCompileShader(fragmentShader);

            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                std::cout << "fragment shader was probably not compiled\n";
                return false;
            }
        }

        return true;
    }


    void createCalculatorWindow(TextBuffer& text_buffer)
    {
        float const calc_width  = 300;
        float const calc_height = 300;
        int const rows          = 4;
        int const cols          = 4;
        ImGui::SetNextWindowSize(ImVec2{calc_width * 1.1f, calc_height * 1.23f});


        static bool should_open = true;

        if (should_open)
        {
            auto const flags =
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
            ImGui::Begin("Calculator Window", &should_open, flags);

            std::vector<TextButton> const buttons{{"1", 0, 0, TextButtonPurpose::AddToBuffer},
                {"2", 1, 0, TextButtonPurpose::AddToBuffer}, {"3", 2, 0, TextButtonPurpose::AddToBuffer},
                {"*", 3, 0, TextButtonPurpose::AddToBuffer}, {"4", 0, 1, TextButtonPurpose::AddToBuffer},
                {"5", 1, 1, TextButtonPurpose::AddToBuffer}, {"6", 2, 1, TextButtonPurpose::AddToBuffer},
                {"/", 3, 1, TextButtonPurpose::AddToBuffer}, {"7", 0, 2, TextButtonPurpose::AddToBuffer},
                {"8", 1, 2, TextButtonPurpose::AddToBuffer}, {"9", 2, 2, TextButtonPurpose::AddToBuffer},
                {"=", 3, 2, TextButtonPurpose::AddToBuffer}, {"+", 0, 3, TextButtonPurpose::AddToBuffer},
                {"0", 1, 3, TextButtonPurpose::AddToBuffer}, {"-", 2, 3, TextButtonPurpose::AddToBuffer},
                {"C", 3, 3, TextButtonPurpose::ClearAll}};

            ImVec2 const button_size{calc_width / cols, calc_height / rows};

            auto const tableFlags = ImGuiTableFlags_None;
            if (ImGui::BeginTable("calcTable", 1, tableFlags))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                auto const input_text_flags = ImGuiInputTextFlags_ReadOnly;
                ImGui::PushItemWidth(calc_width * 1.1);
                ImGui::InputText("", text_buffer.buff.data(), text_buffer.buff.size(), input_text_flags);
                ImGui::PopItemWidth();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::BeginTable("buttonTable", cols, tableFlags))
                {
                    for (auto const& button : buttons)
                    {
                        if (button.col == 0)
                        {
                            ImGui::TableNextRow();
                        }

                        ImGui::TableSetColumnIndex(button.col);
                        if (ImGui::Button(button.text.c_str(), button_size))
                        {
                            switch (button.purpose)
                            {
                            case TextButtonPurpose::AddToBuffer:
                                std::copy(begin(button.text), end(button.text),
                                    begin(text_buffer.buff) + text_buffer.curr_index);
                                text_buffer.curr_index += button.text.size();
                                break;
                            case TextButtonPurpose::ClearAll:
                                std::fill_n(begin(text_buffer.buff), text_buffer.curr_index, 0);
                                text_buffer.curr_index = 0;
                                break;
                            }
                        }
                    }
                    ImGui::EndTable(); // buttonTable
                }
                ImGui::EndTable(); // calcTable
            }


            ImGui::End();
        }
    }
} // namespace

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{
    //[setup Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    //]

    if (glewInit() != GLEW_OK)
    {
        std::cout << "error initialising glew\n";
        return -1;
    }

    bool err = false;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    //[setup_imgui Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // TextBuffer For Calc
    TextBuffer text_buffer{{}, 0};

    if (!createShaders(vertex_shader, fragment_shader))
    {
        std::cout << "error\n";
        return -1;
    }

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // Drawing a sphere
        {
            std::string vertex_shader = R"()";
            std::vector<float> const vertices;


            GLuint vertex_buffer = 0;
            GLsizeiptr data_size = vertices.size() * sizeof(float);
            glGenBuffers(1, &vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, data_size, 0, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, vertices.data());
        }

        createCalculatorWindow(text_buffer);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}