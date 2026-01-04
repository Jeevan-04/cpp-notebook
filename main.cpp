#include "src/ui/notebook.hpp"  // Updated include path
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>
#include <glad/glad.h>
#include <iostream>

int main(int, char**) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Configure OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "C++ Notebook",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create GL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigMacOSXBehaviors = true; // Enable macOS keyboard shortcuts (Cmd instead of Ctrl)

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load larger fonts for better visibility
    ImFontConfig fontConfig;
    fontConfig.SizePixels = 18.0f; // Default font size (18pt)
    io.Fonts->AddFontDefault(&fontConfig);
    
    // Create much larger font for code (22pt)
    fontConfig.SizePixels = 22.0f;
    ImFont* codeFont = io.Fonts->AddFontDefault(&fontConfig);
    
    // Create heading fonts
    fontConfig.SizePixels = 28.0f; // H1 - 28pt
    ImFont* h1Font = io.Fonts->AddFontDefault(&fontConfig);
    
    fontConfig.SizePixels = 24.0f; // H2 - 24pt
    ImFont* h2Font = io.Fonts->AddFontDefault(&fontConfig);
    
    fontConfig.SizePixels = 20.0f; // H3 - 20pt
    ImFont* h3Font = io.Fonts->AddFontDefault(&fontConfig);
    
    io.Fonts->Build();

    // Style
    ImGui::StyleColorsDark();

    // Notebook state
    std::vector<NotebookUtils::Cell> cells;
    bool darkMode = true;
    std::string selectedFont = "Default";

    // Main loop
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Start frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Draw notebook
        NotebookUtils::drawNotebook(cells, darkMode, selectedFont);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}