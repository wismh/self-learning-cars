#include <gtest/gtest.h>

#include "ai/genetic_algorithm.h"

TEST(GeneticAlgorithmTest, InitialPopulation) {
    game::ai::GeneticAlgorithm ga(40);
    EXPECT_EQ(ga.population().size(), 40);

    for (const auto& genome : ga.population()) {
        EXPECT_EQ(genome.weights.size(), game::ai::NeuralNetwork::kTotalWeights);
        EXPECT_EQ(genome.fitness, 0.0f);
        EXPECT_GT(genome.id, 0u);
    }
}

TEST(GeneticAlgorithmTest, ResetDeterminism) {
    game::ai::GeneticAlgorithm ga1(20);
    game::ai::GeneticAlgorithm ga2(20);

    ga1.reset(12345);
    ga2.reset(12345);

    for (std::size_t i = 0; i < 20; ++i) {
        EXPECT_EQ(ga1.population()[i].id, ga2.population()[i].id);
        EXPECT_EQ(ga1.population()[i].weights, ga2.population()[i].weights);
    }
}

TEST(GeneticAlgorithmTest, DifferentSeedProducesDifferentWeights) {
    game::ai::GeneticAlgorithm ga1(20);
    game::ai::GeneticAlgorithm ga2(20);

    ga1.reset(100);
    ga2.reset(200);

    EXPECT_NE(ga1.population()[0].weights, ga2.population()[0].weights);
}

TEST(GeneticAlgorithmTest, SetFitnessAndBestGenome) {
    game::ai::GeneticAlgorithm ga(10);

    ga.set_fitness(3, 45.0f);
    ga.set_fitness(7, 120.0f);
    ga.set_fitness(2, 85.0f);
    ga.set_fitness(999, 500.0f); // Out of bounds, should be ignored safely

    const auto& best = ga.best_genome();
    EXPECT_FLOAT_EQ(best.fitness, 120.0f);
    EXPECT_EQ(best.id, ga.population()[7].id);
}

TEST(GeneticAlgorithmTest, ElitismPreservesBestFitness) {
    game::ai::GeneticAlgorithm ga(20);

    for (std::size_t i = 0; i < ga.population().size(); ++i) {
        ga.set_fitness(i, static_cast<float>(i * 10));
    }

    const float highest = ga.population().back().fitness; // 190.0
    ga.evolve();

    EXPECT_FLOAT_EQ(ga.all_time_best_fitness(), highest);
    EXPECT_FLOAT_EQ(ga.best_fitness(), highest);
    EXPECT_EQ(ga.population().size(), 20);
}

TEST(GeneticAlgorithmTest, MutateAllAltersWeightsWithinClamp) {
    game::ai::GeneticAlgorithm ga(10);
    const auto original_weights = ga.population()[0].weights;

    ga.mutate_all(1.0f);

    const auto& mutated = ga.population()[0].weights;
    EXPECT_NE(original_weights, mutated);

    for (const auto& genome : ga.population()) {
        for (float w : genome.weights) {
            EXPECT_GE(w, -2.5f);
            EXPECT_LE(w, 2.5f);
        }
    }
}
