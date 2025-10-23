#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <functional>
#include <iostream>
#include <vector>
#include <cmath>
#include "GA.cpp"
#include "GWO.cpp"
#include "DrawScene.cpp"

// Глобальні змінні
GLFWwindow* window;
const char* glsl_version = "#version 130";
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

GeneticAlgorithm ga;
GreyWolfOptimizer gwo;
FunctionDrawer drawer;

// Налаштування GUI
int selectedAlgorithm = 0;
int populationSize = 50;
int maxGenerations = 100;
float crossoverRate = 0.8f;
float mutationRate = 0.1f;
int chromosomeLength = 16;
float searchMin = -10.0f;
float searchMax = 10.0f;
bool isRunning = false;
int currentGeneration = 0;
std::vector<float> bestPositions;
std::vector<float> bestFitnessHistory;

// Тестові функції
const char* testFunctions[] = { "Sphere", "Rastrigin", "Custom Function" };
int selectedFunction = 0;

float TestFunction(float x) {
    const float pi = 3.14159265359f;
    switch (selectedFunction) {
        case 0: // Sphere
            return x * x;
        case 1: // Rastrigin
            return x * x - 10.0f * cosf(2.0f * pi * x) + 10.0f;
        case 2: // custom 
            return (x*x)-2;
        default:
            return x * x;
    }
}

void Initialize() {
    // Ініціалізація GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Optimization Algorithms - GA & GWO", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Ініціалізація ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Ініціалізація алгоритмів
    ga.Initialize(static_cast<size_t>(populationSize), static_cast<size_t>(chromosomeLength),searchMin, searchMax, crossoverRate, mutationRate);
    ga.EvaluateFitness(TestFunction);
    gwo.Initialize(static_cast<size_t>(populationSize), searchMin, searchMax);
    drawer.Initialize(searchMin, searchMax, TestFunction);
}

void Update() {
    if (!isRunning) return;

    if (selectedAlgorithm == 0) { // GA
        ga.RunGeneration(TestFunction);
        currentGeneration = ga.GetCurrentGeneration();
        bestPositions = ga.GetBestPositions();
    } else { // GWO
        gwo.RunGeneration(TestFunction);
        currentGeneration = gwo.GetCurrentGeneration();
        bestPositions = gwo.GetBestPositions();
    }

    if (currentGeneration >= maxGenerations) {
        isRunning = false;
    }
} 

void Render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags controlPanelFlags = ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse;
    
    ImGuiWindowFlags visualizationFlags = ImGuiWindowFlags_NoResize | 
                                         ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_NoCollapse;

    // Головне вікно
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(400, WINDOW_HEIGHT));
    ImGui::Begin("Control Panel", nullptr, controlPanelFlags);

    // Вибір алгоритму
    ImGui::Text("Optimization Algorithm");
    ImGui::RadioButton("Genetic Algorithm", &selectedAlgorithm, 0);
    ImGui::RadioButton("Grey Wolf Optimizer", &selectedAlgorithm, 1);
    ImGui::Separator();

    // Вибір функції
    ImGui::Text("Test Function");
    bool functionChanged = ImGui::Combo("Function", &selectedFunction, testFunctions, IM_ARRAYSIZE(testFunctions));
    ImGui::Separator();

    ImGui::Text("Common Parameters");
    ImGui::SliderInt("Population Size", &populationSize, 10, 200);
    ImGui::SliderInt("Max Generations", &maxGenerations, 10, 500);
    bool rangeChanged = false;
    rangeChanged |= ImGui::SliderFloat("Search Min", &searchMin, -10.0f, 0.0f);
    rangeChanged |= ImGui::SliderFloat("Search Max", &searchMax, 0.0f, 10.0f);

    if (selectedAlgorithm == 0) {
        ImGui::Separator();
        ImGui::Text("Genetic Algorithm Parameters");
        ImGui::SliderFloat("Crossover Rate", &crossoverRate, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Mutation Rate", &mutationRate, 0.0f, 1.0f, "%.2f");
        ImGui::SliderInt("Chromosome Length", &chromosomeLength, 8, 32);
    }
        
    if (rangeChanged || functionChanged) {
        drawer.Initialize(searchMin, searchMax, TestFunction);
    }

    // Керування симуляцією
    ImGui::Text("Simulation Control");
    if (ImGui::Button(isRunning ? "Pause" : "Start")) {
        isRunning = !isRunning;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        isRunning = false;
        currentGeneration = 0;
        if (selectedAlgorithm == 0) {
            ga.Initialize(static_cast<size_t>(populationSize), static_cast<size_t>(chromosomeLength), searchMin, searchMax, crossoverRate, mutationRate);
            ga.EvaluateFitness(TestFunction);
        } else {
            gwo.Initialize(static_cast<size_t>(populationSize), searchMin, searchMax);
        }
        drawer.Initialize(searchMin, searchMax, TestFunction);
        bestPositions.clear();
    }

    ImGui::SameLine();
    if (ImGui::Button("Step")) {
        if (selectedAlgorithm == 0) {
            ga.RunGeneration(TestFunction);
            bestPositions = ga.GetBestPositions();
            currentGeneration = ga.GetCurrentGeneration();
        } else {
            gwo.RunGeneration(TestFunction);
            bestPositions = gwo.GetBestPositions();
            currentGeneration = gwo.GetCurrentGeneration();
        }
    }

    ImGui::Text("Generation: %d/%d", currentGeneration, maxGenerations);
    
    // Результати
    if (!bestPositions.empty()) {
        float bestX = bestPositions[0];
        float bestFitness = TestFunction(bestX);
        ImGui::Text("Best Solution: x = %.4f", bestX);
        ImGui::Text("Best Fitness: %.6f", bestFitness);
    }

    ImGui::End();

    // Вікно візуалізації
    ImGui::SetNextWindowPos(ImVec2(400, 0));
    ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH - 400, WINDOW_HEIGHT));
    ImGui::Begin("Visualization", nullptr, visualizationFlags);
    
    drawer.DrawFunction();
    if (!bestPositions.empty()) {
        drawer.DrawSolutions(bestPositions);
    }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

void Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    Initialize();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Update();
        Render();
    }

    Cleanup();
    return 0;
}
