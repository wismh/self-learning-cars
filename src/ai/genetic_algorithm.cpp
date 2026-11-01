#include "ai/genetic_algorithm.h"

#include <algorithm>

namespace game::ai {

GeneticAlgorithm::GeneticAlgorithm(std::size_t population_size)
    : population_size_(population_size) {
    reset();
}

void GeneticAlgorithm::reset(uint32_t seed) {
    rng_.seed(seed);
    next_genome_id_ = 1;
    best_fitness_ = 0.0f;
    all_time_best_fitness_ = 0.0f;
    population_.clear();
    population_.resize(population_size_);

    std::uniform_real_distribution<float> dist(-0.8f, 0.8f);
    constexpr std::size_t kThrottleBiasIdx = NeuralNetwork::kWeights1 + NeuralNetwork::kBiases1 + NeuralNetwork::kWeights2 + 1;
    for (std::size_t i = 0; i < population_size_; ++i) {
        population_[i].id = next_genome_id_++;
        population_[i].weights.resize(NeuralNetwork::kTotalWeights);
        for (float& w : population_[i].weights) {
            w = dist(rng_);
        }
        population_[i].weights[kThrottleBiasIdx] = 0.5f; // Initial forward driving bias
        population_[i].fitness = 0.0f;
    }
}

void GeneticAlgorithm::set_fitness(std::size_t index, float fitness) {
    if (index < population_.size()) {
        population_[index].fitness = fitness;
    }
}

const Genome& GeneticAlgorithm::best_genome() const {
    return *std::max_element(population_.begin(), population_.end(), [](const Genome& a, const Genome& b) {
        return a.fitness < b.fitness;
    });
}

const Genome& GeneticAlgorithm::tournament_select(int k) {
    std::uniform_int_distribution<std::size_t> dist(0, population_.size() - 1);
    std::size_t best_idx = dist(rng_);
    for (int i = 1; i < k; ++i) {
        const std::size_t candidate = dist(rng_);
        if (population_[candidate].fitness > population_[best_idx].fitness) {
            best_idx = candidate;
        }
    }
    return population_[best_idx];
}

Genome GeneticAlgorithm::crossover(const Genome& parent1, const Genome& parent2) {
    Genome child;
    child.id = next_genome_id_++;
    child.weights.resize(NeuralNetwork::kTotalWeights);
    child.fitness = 0.0f;

    std::uniform_int_distribution<int> coin(0, 1);
    for (std::size_t i = 0; i < child.weights.size(); ++i) {
        child.weights[i] = (coin(rng_) == 0) ? parent1.weights[i] : parent2.weights[i];
    }
    return child;
}

void GeneticAlgorithm::mutate(Genome& genome) {
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    std::normal_distribution<float> delta(0.0f, mutation_strength_);

    for (float& w : genome.weights) {
        if (prob(rng_) < mutation_rate_) {
            w += delta(rng_);
            w = std::clamp(w, -2.5f, 2.5f);
        }
    }
}

void GeneticAlgorithm::mutate_all(float strength) {
    std::normal_distribution<float> delta(0.0f, strength);
    for (auto& g : population_) {
        for (float& w : g.weights) {
            w += delta(rng_);
            w = std::clamp(w, -2.5f, 2.5f);
        }
    }
}

void GeneticAlgorithm::evolve() {
    std::sort(population_.begin(), population_.end(), [](const Genome& a, const Genome& b) {
        return a.fitness > b.fitness;
    });

    best_fitness_ = population_.front().fitness;
    if (best_fitness_ > all_time_best_fitness_) {
        all_time_best_fitness_ = best_fitness_;
    }

    const std::size_t elite_count = std::max<std::size_t>(1, static_cast<std::size_t>(population_size_ * elite_ratio_));
    std::vector<Genome> next_generation;
    next_generation.reserve(population_size_);

    // 1. Keep elites untouched
    for (std::size_t i = 0; i < elite_count; ++i) {
        Genome elite = population_[i];
        elite.fitness = 0.0f;
        next_generation.push_back(std::move(elite));
    }

    // 2. Breed remaining slots
    while (next_generation.size() < population_size_) {
        const Genome& p1 = tournament_select();
        const Genome& p2 = tournament_select();
        Genome child = crossover(p1, p2);
        mutate(child);
        next_generation.push_back(std::move(child));
    }

    population_ = std::move(next_generation);
}

} // namespace game::ai
