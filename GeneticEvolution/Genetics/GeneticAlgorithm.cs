using System;
using System.Collections.Generic;
using System.Linq;

class GeneticAlgorithm<T>
{
	public List<DNA<T>> Population { get; private set; }
	public int Generation { get; private set; }
	public double BestFitness { get; private set; }
	public double[] PopulationFitness { get; private set; }
	public double MeanPopulationFitness { get { return PopulationFitness.Sum() / PopulationFitness.Length; } }
	public T[] BestGenes { get; private set; }

	public int Elitism;
	public double MutationRate;

	private List<DNA<T>> newPopulation;
	private Random random;
	private double fitnessSum;
	private int dnaSize;
	private Func<T> getRandomGene;

	public GeneticAlgorithm(int populationSize, int dnaSize, Random random, Func<T> getRandomGene,
		int elitism, double mutationRate = 0.01f)
	{
		Generation = 1;
		Elitism = elitism;
		MutationRate = mutationRate;
		Population = new List<DNA<T>>(populationSize);
		newPopulation = new List<DNA<T>>(populationSize);
		this.random = random;
		this.dnaSize = dnaSize;
		this.getRandomGene = getRandomGene;
		PopulationFitness = new double[populationSize];

		BestGenes = new T[dnaSize];

		for (int i = 0; i < populationSize; i++)
		{
			Population.Add(new DNA<T>(dnaSize, random, getRandomGene, shouldInitGenes: true));
		}
	}

	public void NewGeneration(int numNewDNA = 0, bool crossoverNewDNA = false)
	{
		int finalCount = Population.Count + numNewDNA;

		if (finalCount <= 0)
		{
			return;
		}

		if (Population.Count > 0)
		{
			CalculateFitness();
			Population.Sort(CompareDNA);
			//Population.Reverse();
		}
		newPopulation.Clear();

		for (int i = 0; i < Population.Count; i++)
		{
			if (i < Elitism && i < Population.Count)
			{
				newPopulation.Add(Population[i]);
			}
			else if (i < Population.Count || crossoverNewDNA)
			{
				DNA<T> parent1 = ChooseParent();
				DNA<T> parent2 = ChooseParent();

				DNA<T> child = parent1.Crossover(parent2);

				// Just for visual comparisson, no effects on the algorithm
				// as it will be replaced below
				child.Fitness = (parent1.Fitness + parent2.Fitness) / 2; 

				child.Mutate(MutationRate);

				newPopulation.Add(child);
			}
			else
			{
				newPopulation.Add(new DNA<T>(dnaSize, random, getRandomGene, shouldInitGenes: true));
			}
		}

		List<DNA<T>> tmpList = Population;
		Population = newPopulation;
		newPopulation = tmpList;

		foreach (var dna in Population)
		{
			dna.OldFitness = dna.Fitness;
			dna.Fitness = 0;
		}

		Generation++;
	}

	public int CompareDNA(DNA<T> a, DNA<T> b)
	{
		if (a.Fitness > b.Fitness)
		{
			return -1;
		}
		else if (a.Fitness < b.Fitness)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	public void CalculateFitness()
	{
		fitnessSum = 0;
		DNA<T> best = Population[0];

		if (PopulationFitness == null)
			PopulationFitness = new double[Population.Count];

		for (int i = 0; i < Population.Count; i++)
		{
			fitnessSum += Population[i].Fitness;
			PopulationFitness[i] = Population[i].Fitness;

			if (Population[i].Fitness > best.Fitness)
			{
				best = Population[i];
			}
		}

		BestFitness = best.Fitness;
		best.Genes.CopyTo(BestGenes, 0);
	}

	private DNA<T> ChooseParent()
	{
		double randomNumber = random.NextDouble() * fitnessSum;

		for (int i = 0; i < Population.Count; i++)
		{
			if (randomNumber < Population[i].Fitness)
			{
				return Population[i];
			}

			randomNumber -= Population[i].Fitness;
		}

		return Population[random.Next(Population.Count-1)];
	}
}
