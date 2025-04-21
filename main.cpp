#include <cmath>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "render.h"
#include "simulation.h"
#include "HashContainer.h"
#include "BinaryPartitionContainer.h"
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

    // Grid Overlay toggle
    bool show_grid = false;

    // Particle debug toggle
    bool show_debug_panel = false;


    // Simulation settings struct
    Simulation sim;

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

            ImGui::SliderFloat("Smoothing Radius", &sim.smoothing_radius, 0.0, 1.0);
            ImGui::InputFloat("Timestep", &sim.timestep);
            ImGui::InputFloat("Gas Constant", &sim.gas_constant);
            ImGui::InputFloat("Gravity", &sim.gravity);
            ImGui::InputFloat("Target Density", &sim.target_density);
            ImGui::SliderFloat("Viscosity", &sim.viscosity, 0.0, 1.0);
            ImGui::DragInt("Particle Count", &sim.particle_count);

            // Particle pattern dropdown
            const char* patterns[] = { "Grid", "Circle", "Random" };
            int current_pattern = static_cast<int>(sim.spawn_pattern);
            if (ImGui::Combo("Spawn Pattern", &current_pattern, patterns, IM_ARRAYSIZE(patterns))) {
                sim.spawn_pattern = static_cast<Simulation::Pattern>(current_pattern);
            }

            if (ImGui::Button(sim.paused ? "Resume" : "Pause")) {
                sim.paused = !sim.paused;
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                sim.get_particles().vec().clear();

                float r = sim.smoothing_radius;
                int count = sim.particle_count;

                switch (sim.spawn_pattern) {
                    case Simulation::Pattern::Grid: {
                        int grid_dim = static_cast<int>(std::sqrt(count));
                        float spacing = 1.5 / grid_dim;

                        for (int i = 0; i < grid_dim; ++i) {
                            for (int j = 0; j < grid_dim && sim.get_particles().vec().size() < count; ++j) {
                                float x = (i - grid_dim / 2) * spacing;
                                float y = (j - grid_dim / 2) * spacing;
                                sim.get_particles().vec().emplace_back(x, y, 1.0f);
                            }
                        }
                        break;
                    }

                    case Simulation::Pattern::Circle: {
                        float angle_step = 2 * 3.14159f / count;
                        float radius = r * count * 0.1f;

                        for (int i = 0; i < count; ++i) {
                            float angle = i * angle_step;
                            float x = std::cos(angle) * radius;
                            float y = std::sin(angle) * radius;
                            sim.get_particles().vec().emplace_back(x, y, 1.0f);
                        }
                        break;
                    }

                    case Simulation::Pattern::Random: {
                        for (int i = 0; i < count; ++i) {
                            float x = ((rand() % 2000) / 1000.0f - 1.0f) * 0.8f;
                            float y = ((rand() % 2000) / 1000.0f - 1.0f) * 0.8f;
                            sim.get_particles().vec().emplace_back(x, y, 1.0f);
                        }
                        break;
                    }
                }
            }


            ImGui::Checkbox("Show Grid Overlay", &show_grid);

            ImGui::Checkbox("Show Debug Panel", &show_debug_panel);

            ImGui::End();
        }

        // Show simulation space
        {
            ImGui::Begin("Simulation");

            // Draw simulation in child window
            ImGui::BeginChild("SimRender");
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            unsigned int texture = render_particles(sim.get_particles().vec(), render_info, window_size.x, window_size.y);
            ImGui::GetWindowDrawList()->AddImage(
                    (ImTextureID)texture,
                    pos,
                    ImVec2(pos.x+window_size.x, pos.y+window_size.y),
                    ImVec2(0, 1),
                    ImVec2(1, 0));

            // Grid Overlay Display for subdivision
            if (show_grid) {
                int num_cols = (2.0 / (sim.smoothing_radius*3)) + 2;
                int num_rows = (2.0 / (sim.smoothing_radius*3)) + 2;
                float scaling_x = window_size.x / 2.0;
                float scaling_y = window_size.y / 2.0;

                ImU32 grid_color = IM_COL32(255, 255, 255, 40); // light white, semi-transparent

                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Draw vertical grid lines
                for (int i = 0; i <= num_cols; ++i) {
                    float x = pos.x + scaling_x*(i * sim.smoothing_radius*3 - 1.0);
                    draw_list->AddLine(ImVec2(x, pos.y), ImVec2(x, pos.y + window_size.y), grid_color);
                }

                // Draw horizontal grid lines
                for (int j = 0; j <= num_rows; ++j) {
                    float y = pos.y + scaling_y*(j * sim.smoothing_radius*3 - 1.0);
                    draw_list->AddLine(ImVec2(pos.x, y), ImVec2(pos.x + window_size.x, y), grid_color);
                }
            }
            ImGui::EndChild();

            ImGui::End();
        }

        // Debug Tools Panel
        if (show_debug_panel) {
            ImGui::Begin("Debug Tools");

            const auto& particle_vec = sim.get_particles().vec();
            ImGui::Text("Total Particles: %zu", particle_vec.size());

            if (!particle_vec.empty()) {
                const auto& p = particle_vec[0];
                ImGui::Text("P[0] Position: (%.3f, %.3f)", p.px, p.py);
                ImGui::Text("P[0] Velocity: (%.3f, %.3f)", p.vx, p.vy);
            }

            ImGui::End();
        }

        // Perform a physics update on the particles using physics wrapper
    if (!sim.paused) {
        // do several physics updates per frame
        for (int i=0; i<2; i++)
        {
            sim.phys_update();
        }
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
