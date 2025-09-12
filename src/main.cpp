#include <bits/stdc++.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// test function 
float test_function(float x, float x_opt = 2.0f) {
    return (x - x_opt) * (x - x_opt);
}

// Genetic Algorithm
struct GA {
    int population_size = 20;
    int bits = 16;
    float a = -5.0f, b = 5.0f;
    float crossover_prob = 0.7f;
    float mutation_prob = 0.01f;
    float x_opt = 2.0f;

    struct Individual {
        std::string code;
        float x;
        float fitness;
    };

    std::vector<Individual> population;
    std::mt19937 rng{std::random_device{}()};

    float decode(const std::string &code) {
        int val = 0;
        for (char c : code) val = (val << 1) | (c - '0');
        float x = (float)val / (float)((1 << bits) - 1);
        return x * (b - a) + a;
    }

    float fitness(float x) { return -test_function(x, x_opt); }

    void init_random() {
        std::uniform_int_distribution<int> bit(0, 1);
        population.clear();
        for (int i = 0; i < population_size; i++) {
            std::string code;
            for (int j = 0; j < bits; j++) code.push_back(bit(rng) ? '1' : '0');
            Individual ind;
            ind.code = code;
            ind.x = decode(code);
            ind.fitness = fitness(ind.x);
            population.push_back(ind);
        }
    }

    void step() {
        if (population.empty()) return;

        std::vector<Individual> new_pop;
        std::uniform_real_distribution<float> prob(0.0f, 1.0f);
        std::uniform_int_distribution<int> point(1, bits - 1);

        auto tournament = [&]() -> Individual {
            std::uniform_int_distribution<int> idx(0, population_size - 1);
            Individual &a = population[idx(rng)];
            Individual &b = population[idx(rng)];
            return a.fitness > b.fitness ? a : b;
        };

        while ((int)new_pop.size() < population_size) {
            Individual p1 = tournament();
            Individual p2 = tournament();
            std::string c1 = p1.code, c2 = p2.code;

            if (prob(rng) < crossover_prob) {
                int cp = point(rng);
                swap_ranges(c1.begin() + cp, c1.end(), c2.begin() + cp);
            }

            for (auto &c : {&c1, &c2}) {
                for (char &bit : *c) {
                    if (prob(rng) < mutation_prob)
                        bit = (bit == '0' ? '1' : '0');
                }
            }

            Individual i1{c1, decode(c1), 0};
            Individual i2{c2, decode(c2), 0};
            i1.fitness = fitness(i1.x);
            i2.fitness = fitness(i2.x);
            new_pop.push_back(i1);
            new_pop.push_back(i2);
        }

        population = std::move(new_pop);
    }
};

// Grey Wolf Optimizer 
struct GWO {
    int wolves_count = 20;
    float a = -5.0f, b = 5.0f;
    float x_opt = 2.0f;
    int max_iter = 100;
    int iter = 0;

    struct Wolf { float x, fitness; };

    std::vector<Wolf> wolves;
    std::mt19937 rng{std::random_device{}()};

    float fitness(float x) { return test_function(x, x_opt); }

    void init_random() {
        std::uniform_real_distribution<float> dist(a, b);
        wolves.clear();
        for (int i = 0; i < wolves_count; i++) {
            Wolf w{dist(rng), 0};
            w.fitness = fitness(w.x);
            wolves.push_back(w);
        }
        iter = 0;
    }

    void step() {
        if (wolves.empty()) return;

        sort(wolves.begin(), wolves.end(),
            [&](auto &u, auto &v) { return u.fitness < v.fitness; });

        Wolf alpha = wolves[0];
        Wolf beta  = wolves[1];
        Wolf delta = wolves[2];

        float a_param = 2.0f - (2.0f * iter / (float)max_iter);
        std::uniform_real_distribution<float> dist01(0, 1);

        for (auto &w : wolves) {
            float r1 = dist01(rng), r2 = dist01(rng);
            float A1 = 2 * a_param * r1 - a_param;
            float C1 = 2 * r2;
            float D_alpha = fabs(C1 * alpha.x - w.x);
            float X1 = alpha.x - A1 * D_alpha;

            r1 = dist01(rng); r2 = dist01(rng);
            float A2 = 2 * a_param * r1 - a_param;
            float C2 = 2 * r2;
            float D_beta = fabs(C2 * beta.x - w.x);
            float X2 = beta.x - A2 * D_beta;

            r1 = dist01(rng); r2 = dist01(rng);
            float A3 = 2 * a_param * r1 - a_param;
            float C3 = 2 * r2;
            float D_delta = fabs(C3 * delta.x - w.x);
            float X3 = delta.x - A3 * D_delta;

            w.x = (X1 + X2 + X3) / 3.0f;
            if (w.x < a) w.x = a;
            if (w.x > b) w.x = b;
            w.fitness = fitness(w.x);
        }
        iter++;
    }
};

// Drawing
void draw_scene(GA &ga, GWO &gwo) {
    float min_y = FLT_MAX, max_y = -FLT_MAX;

    for (int i = 0; i < 200; i++) {
        float t = (float)i / 199.0f;
        float x = ga.a + t * (ga.b - ga.a);
        float y = test_function(x, ga.x_opt);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // function
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 200; i++) {
        float t = (float)i / 199.0f;
        float x = ga.a + t * (ga.b - ga.a);
        float y = test_function(x, ga.x_opt);
        float nx = -1.0f + 2.0f * t;
        float ny = -1.0f + 2.0f * (y - min_y) / (max_y - min_y);
        glVertex2f(nx, ny);
    }
    glEnd();

    // GA dots (red)
    glColor3f(1, 0, 0);
    glPointSize(6);
    glBegin(GL_POINTS);
    for (auto &ind : ga.population) {
        float nx = -1.0f + 2.0f * (ind.x - ga.a) / (ga.b - ga.a);
        float ny = -1.0f + 2.0f * (test_function(ind.x, ga.x_opt) - min_y) / (max_y - min_y);
        glVertex2f(nx, ny);
    }
    glEnd();

    // GWO dots (blue)
    glColor3f(0, 0, 1);
    glBegin(GL_POINTS);
    for (auto &w : gwo.wolves) {
        float nx = -1.0f + 2.0f * (w.x - gwo.a) / (gwo.b - gwo.a);
        float ny = -1.0f + 2.0f * (test_function(w.x, gwo.x_opt) - min_y) / (max_y - min_y);
        glVertex2f(nx, ny);
    }
    glEnd();
}

// Main
int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1000, 600, "GA + GWO Demo", NULL, NULL);
    if(!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    GA ga;
    GWO gwo;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_scene(ga, gwo);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(280, 150));
        ImGui::SetNextWindowPos(ImVec2(720, 0));
        ImGui::Begin("GA + GWO Controls", nullptr, ImGuiWindowFlags_NoResize);

        ImGui::SliderFloat("Interval a", &ga.a, -10.0f, 0.0f);
        ImGui::SliderFloat("Interval b", &ga.b, 0.0f, 10.0f);
        ImGui::SliderFloat("Optimum x", &ga.x_opt, -5.0f, 5.0f);

        if (ImGui::Button("Init GA")) { ga.init_random(); } 
        ImGui::SameLine();
        if (ImGui::Button("Step GA")) { ga.step(); }

        if (ImGui::Button("Init GWO")) { gwo.init_random(); } 
        ImGui::SameLine();
        if (ImGui::Button("Step GWO")) { gwo.step(); }

        ImGui::End();

        ImGui::Render();
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

