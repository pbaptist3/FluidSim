#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "render.h"
#include "simulation.h"
#include "HashContainer.h"
#include "GraphContainer.h"
#include "SimulationSettings.h"
#include "simulation_extensions.h"

#define IMGUI_ENABLE_FREETYPE

constexpr ImVec4 clear_color = ImVec4(0.00f, 0.02f, 0.10f, 1.00f);

/// Handler for any glfw errors
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main()
{
    // OpenGL/imgui initialization code taken from imgui examples at
    // https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "FluidSim", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup our particle rendering with OpenGL
    GLRenderInfo render_info = init_render();

    // State
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_grid = false;

    // Simulation settings struct
    SimulationSettings settings;

    HashContainer particles;
    particles.vec().emplace_back(0.0, 0.5, 1.0);
    particles.vec().emplace_back(-0.5, -0.5, 1.0);
    particles.vec().emplace_back(0.5, -0.5, 1.0);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show config window
        {
            ImGui::Begin("Config");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            ImGui::SliderFloat("Timestep", &settings.timestep, 0.001f, 0.1f);
            ImGui::SliderFloat("Gravity", &settings.gravity, 0.0f, 20.0f);
            ImGui::SliderFloat("Viscosity", &settings.viscosity, 0.0f, 2.0f);

            // Particle pattern dropdown
            const char* patterns[] = { "Grid", "Circle", "Random" };
            int current_pattern = static_cast<int>(settings.spawn_pattern);
            if (ImGui::Combo("Spawn Pattern", &current_pattern, patterns, IM_ARRAYSIZE(patterns))) {
                settings.spawn_pattern = static_cast<SimulationSettings::Pattern>(current_pattern);
            }

            if (ImGui::Button(settings.paused ? "Resume" : "Pause")) {
                settings.paused = !settings.paused;
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                particles.vec().clear();

                float r = settings.particle_radius;
                int count = settings.particle_count;

                switch (settings.spawn_pattern) {
                    case SimulationSettings::Pattern::Grid: {
                        int grid_dim = static_cast<int>(std::sqrt(count));
                        float spacing = r * 2.5f;

                        for (int i = 0; i < grid_dim; ++i) {
                            for (int j = 0; j < grid_dim && particles.vec().size() < count; ++j) {
                                float x = (i - grid_dim / 2) * spacing;
                                float y = (j - grid_dim / 2) * spacing;
                                particles.vec().emplace_back(x, y, 1.0f);
                            }
                        }
                        break;
                    }

                    case SimulationSettings::Pattern::Circle: {
                        float angle_step = 2 * 3.14159f / count;
                        float radius = r * count * 0.1f;

                        for (int i = 0; i < count; ++i) {
                            float angle = i * angle_step;
                            float x = std::cos(angle) * radius;
                            float y = std::sin(angle) * radius;
                            particles.vec().emplace_back(x, y, 1.0f);
                        }
                        break;
                    }

                    case SimulationSettings::Pattern::Random: {
                        for (int i = 0; i < count; ++i) {
                            float x = ((rand() % 2000) / 1000.0f - 1.0f) * 0.8f;
                            float y = ((rand() % 2000) / 1000.0f - 1.0f) * 0.8f;
                            particles.vec().emplace_back(x, y, 1.0f);
                        }
                        break;
                    }
                }
            }


            ImGui::Checkbox("Show Grid Overlay", &show_grid);

            ImGui::End();
        }

        // Show simulation space
        {
            ImGui::Begin("Simulation", &show_another_window);

            // Draw simulation in child window
            ImGui::BeginChild("SimRender");
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            unsigned int texture = render_particles(particles.vec(), render_info, window_size.x, window_size.y);
            ImGui::GetWindowDrawList()->AddImage(
                    (ImTextureID)texture,
                    pos,
                    ImVec2(pos.x+window_size.x, pos.y+window_size.y),
                    ImVec2(0, 1),
                    ImVec2(1, 0));

            // Grid Overlay Display for subdivision
            if (show_grid) {
                int num_cols = 20;
                int num_rows = 20;

                ImU32 grid_color = IM_COL32(255, 255, 255, 40); // light white, semi-transparent

                float cell_width  = window_size.x / num_cols;
                float cell_height = window_size.y / num_rows;

                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Draw vertical grid lines
                for (int i = 0; i <= num_cols; ++i) {
                    float x = pos.x + i * cell_width;
                    draw_list->AddLine(ImVec2(x, pos.y), ImVec2(x, pos.y + window_size.y), grid_color);
                }

                // Draw horizontal grid lines
                for (int j = 0; j <= num_rows; ++j) {
                    float y = pos.y + j * cell_height;
                    draw_list->AddLine(ImVec2(pos.x, y), ImVec2(pos.x + window_size.x, y), grid_color);
                }
            }
            ImGui::EndChild();

            ImGui::End();
        }

        // Perform a physics update on the particles using physics wrapper
    if (!settings.paused) {
        apply_extended_physics(particles, settings);
    }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
