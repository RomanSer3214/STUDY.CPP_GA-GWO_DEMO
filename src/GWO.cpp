#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <cmath>
#include <limits>

class GreyWolfOptimizer {
private:
    struct Wolf {
        float position;
        float fitness;
        
        Wolf() : position(0.0f), fitness(-std::numeric_limits<float>::max()) {}
        Wolf(float pos, float fit) : position(pos), fitness(fit) {}
    };

    std::vector<Wolf> wolves;
    Wolf alpha, beta, delta;
    size_t populationSize;
    float searchMin, searchMax;
    int currentGeneration;
    std::mt19937 rng;

public:
    GreyWolfOptimizer() : populationSize(0), currentGeneration(0) {
        rng.seed(std::random_device{}());
    }

    void Initialize(size_t popSize, float min, float max) {
        populationSize = popSize;
        searchMin = min;
        searchMax = max;
        currentGeneration = 0;

        wolves.clear();
        wolves.resize(populationSize);
        std::uniform_real_distribution<float> dist(min, max);
        
        for (auto& wolf : wolves) {
            wolf.position = dist(rng);
            wolf.fitness = -std::numeric_limits<float>::max();
        }

        alpha = Wolf(0, -std::numeric_limits<float>::max());
        beta = Wolf(0, -std::numeric_limits<float>::max());
        delta = Wolf(0, -std::numeric_limits<float>::max());
    }

    void EvaluateFitness(std::function<float(float)> fitnessFunction) {
        // Оновлюємо alpha, beta, delta
        for (const auto& wolf : wolves) {
            float fitness = -fitnessFunction(wolf.position); // Мінімізація
            
            if (fitness > alpha.fitness) {
                delta = beta;
                beta = alpha;
                alpha = Wolf(wolf.position, fitness);
            } else if (fitness > beta.fitness) {
                delta = beta;
                beta = Wolf(wolf.position, fitness);
            } else if (fitness > delta.fitness) {
                delta = Wolf(wolf.position, fitness);
            }
        }
    }

    void RunGeneration(std::function<float(float)> fitnessFunction) {
        EvaluateFitness(fitnessFunction);

        // Параметр a лінійно зменшується від 2 до 0
        float a = 2.0f - (2.0f * currentGeneration) / 100.0f;
        
        for (auto& wolf : wolves) {
            float A1 = 2.0f * a * std::uniform_real_distribution<float>(0, 1)(rng) - a;
            float C1 = 2.0f * std::uniform_real_distribution<float>(0, 1)(rng);
            float D_alpha = std::abs(C1 * alpha.position - wolf.position);
            float X1 = alpha.position - A1 * D_alpha;

            float A2 = 2.0f * a * std::uniform_real_distribution<float>(0, 1)(rng) - a;
            float C2 = 2.0f * std::uniform_real_distribution<float>(0, 1)(rng);
            float D_beta = std::abs(C2 * beta.position - wolf.position);
            float X2 = beta.position - A2 * D_beta;

            float A3 = 2.0f * a * std::uniform_real_distribution<float>(0, 1)(rng) - a;
            float C3 = 2.0f * std::uniform_real_distribution<float>(0, 1)(rng);
            float D_delta = std::abs(C3 * delta.position - wolf.position);
            float X3 = delta.position - A3 * D_delta;
            float newPosition = (X1 + X2 + X3) / 3.0f;
            wolf.position = std::max(searchMin, std::min(searchMax, newPosition));
        }

        currentGeneration++;
    }

    std::vector<float> GetBestPositions() {
        std::vector<float> positions;
        positions.push_back(alpha.position);
        positions.push_back(beta.position);
        positions.push_back(delta.position);
        return positions;
    }

    int GetCurrentGeneration() const { return currentGeneration; }
};
