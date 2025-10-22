#include <vector>
#include <random>
#include <algorithm>
#include <functional>

class GeneticAlgorithm {
private:
    struct Chromosome {
        std::vector<bool> genes;
        float fitness;
        float position;
        
        Chromosome(size_t length) : genes(length), fitness(0.0f), position(0.0f) {}
    };

    std::vector<Chromosome> population;
    size_t populationSize;
    size_t chromosomeLength;
    float crossoverRate;
    float mutationRate;
    float searchMin, searchMax;
    int currentGeneration;
    std::mt19937 rng;

public:
    GeneticAlgorithm() : populationSize(0), chromosomeLength(0), currentGeneration(0) {
        rng.seed(std::random_device{}());
    }

    void Initialize(size_t popSize, size_t chromLength, float min, float max, float crossRate, float mutRate) {
        populationSize = popSize;
        chromosomeLength = chromLength;
        searchMin = min;
        searchMax = max;
        crossoverRate = crossRate;
        mutationRate = mutRate;
        currentGeneration = 0;

        population.clear();
        for (size_t i = 0; i < populationSize; ++i) {
            Chromosome chrom(chromosomeLength);
            for (size_t j = 0; j < chromosomeLength; ++j) {
                chrom.genes[j] = std::uniform_real_distribution<float>(0, 1)(rng) > 0.5f;
            }
            population.push_back(chrom);
        }
    }

    float BinaryToFloat(const std::vector<bool>& binary) {
        unsigned long long decimal = 0;
        for (size_t i = 0; i < chromosomeLength; ++i) {
            decimal = (decimal << 1) | (binary[i] ? 1 : 0);
        }
        
        // ПЕРЕВІР ЦЕ - може бути помилка з типами
        double maxDecimal = (1ULL << chromosomeLength) - 1;
        return static_cast<float>((decimal / maxDecimal) * (searchMax - searchMin) + searchMin);
    }

    void EvaluateFitness(std::function<float(float)> fitnessFunction) {
        for (auto& chrom : population) {
            chrom.position = BinaryToFloat(chrom.genes);
            chrom.fitness = -fitnessFunction(chrom.position); // Мінімізація
        }
    }

    std::vector<bool> Crossover(const std::vector<bool>& parent1, const std::vector<bool>& parent2) {
        std::vector<bool> child(chromosomeLength);
        if (std::uniform_real_distribution<float>(0, 1)(rng) < crossoverRate) {
            size_t crossoverPoint = std::uniform_int_distribution<size_t>(1, chromosomeLength - 1)(rng);
            for (size_t i = 0; i < chromosomeLength; ++i) {
                child[i] = (i < crossoverPoint) ? parent1[i] : parent2[i];
            }
        } else {
            child = parent1;
        }
        return child;
    }

    void Mutate(std::vector<bool>& chromosome) {
        for (size_t i = 0; i < chromosomeLength; ++i) {
            if (std::uniform_real_distribution<float>(0, 1)(rng) < mutationRate) {
                chromosome[i] = !chromosome[i];
            }
        }
    }

    Chromosome TournamentSelection() {
        int tournamentSize = 3;
        Chromosome* best = &population[std::uniform_int_distribution<int>(0, populationSize - 1)(rng)];
        
        for (int i = 1; i < tournamentSize; ++i) {
            Chromosome* candidate = &population[std::uniform_int_distribution<int>(0, populationSize - 1)(rng)];
            if (candidate->fitness > best->fitness) {  // Перевір, що це правильне порівняння
                best = candidate;
            }
        }
        return *best;
    }

    void RunGeneration(std::function<float(float)> fitnessFunction) {
        std::vector<Chromosome> newPopulation;
        
        auto bestChromosome = *std::max_element(population.begin(), population.end(),
            [](const Chromosome& a, const Chromosome& b) { return a.fitness < b.fitness; });
        newPopulation.push_back(bestChromosome);

        // Створюємо нову популяцію
        while (newPopulation.size() < populationSize) {
            Chromosome parent1 = TournamentSelection();
            Chromosome parent2 = TournamentSelection();
            
            Chromosome child1(chromosomeLength);
            child1.genes = Crossover(parent1.genes, parent2.genes);
            Mutate(child1.genes);
            
            // ДОДАЙ ЦЕ: оцінити дитину
            child1.position = BinaryToFloat(child1.genes);
            child1.fitness = -fitnessFunction(child1.position);
            
            newPopulation.push_back(child1);
        }

        population = newPopulation;
        currentGeneration++;
    }

    std::vector<float> GetBestPositions() {
        std::vector<float> positions;
        if (population.empty()) return positions;
        
        auto best = std::max_element(population.begin(), population.end(),
            [](const Chromosome& a, const Chromosome& b) { return a.fitness < b.fitness; });
        
        if (best != population.end()) {
            positions.push_back(best->position);
        }
        return positions;
    } 

    int GetCurrentGeneration() const { return currentGeneration; }
};
