#pragma once

#include "ai/neural_network.h"

#include <cstdint>
#include <random>
#include <span>
#include <vector>

namespace game::ai {

struct Genome {
    uint32_t id = 0;
    std::vector<float> weights{};
    float fitness = 0.0f;
};

// Genetic algorithm managing selection, crossover, and mutation for neuroevolution.
class GeneticAlgorithm {
public:
    explicit GeneticAlgorithm(std::size_t population_size = 60);

    void reset(uint32_t seed = 42);
    void evolve();
    void mutate_all(float strength);

    [[nodiscard]] std::span<Genome> population() { return population_; }
    [[nodiscard]] std::span<const Genome> population() const { return population_; }

    [[nodiscard]] const Genome& best_genome() const;
    [[nodiscard]] float best_fitness() const { return best_fitness_; }
    [[nodiscard]] float all_time_best_fitness() const { return all_time_best_fitness_; }

    void set_fitness(std::size_t index, float fitness);

private:
    [[nodiscard]] const Genome& tournament_select(int k = 3);
    [[nodiscard]] Genome crossover(const Genome& parent1, const Genome& parent2);
    void mutate(Genome& genome);

    std::size_t population_size_;
    float mutation_rate_ = 0.12f;
    float mutation_strength_ = 0.25f;
    float elite_ratio_ = 0.10f;

    std::vector<Genome> population_;
    float best_fitness_ = 0.0f;
    float all_time_best_fitness_ = 0.0f;
    uint32_t next_genome_id_ = 1;
    std::mt19937 rng_{42};
};

} // namespace game::ai
