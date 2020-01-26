using NeuroEvolution;
using System;

public class DNA<T>
{
	public T[] Genes { get; private set; }
	public double Fitness { get; set; }
	public double OldFitness { get; set; }

	private Random random;
	private Func<T> getRandomGene;

	public DNA(int size, Random random, Func<T> getRandomGene, bool shouldInitGenes = true)
	{
		Genes = new T[size];
		this.random = random;
		this.getRandomGene = getRandomGene;

		if (shouldInitGenes)
		{
			for (int i = 0; i < Genes.Length; i++)
			{
				Genes[i] = getRandomGene();
			}
		}
	}

	public override string ToString()
	{
		return $"Current Fitness: {Fitness}";
	}

	public DNA<T> Crossover(DNA<T> otherParent)
	{
		DNA<T> child = new DNA<T>(Genes.Length, random, getRandomGene, shouldInitGenes: false);

		for (int i = 0; i < Genes.Length; i++)
		{
			child.Genes[i] = random.NextDouble() < 0.9 ? Genes[i] : otherParent.Genes[i];
		}

		return child;
	}

	public void Mutate(double mutationRate)
	{
		for (int i = 0; i < Genes.Length; i++)
		{
			if (random.NextDouble() < mutationRate)
			{
				int tipo = random.Next(3);

				switch (tipo)
				{
					case 0: Genes[i] = getRandomGene(); break;
					case 1:
						{
							double rnd = Extensions.GetRandom(-1.5, 1.5);
							Genes[i] = MultiplyGene(Genes[i], rnd);
						} break;
					case 2:
						{
							double rnd = Extensions.GetRandom(-1.5, 1.5);
							Genes[i] = SumGene(Genes[i], rnd);
						} break;
				}
			}
		}
	}

	private T SumGene(T t, double rnd)
	{
		if (typeof(T) == typeof(double))
			return BoxingSafeConverter<double, T>.Instance.Convert(BoxingSafeConverter<T, double>.Instance.Convert(t) + rnd);
		else
			return default(T);
	}

	private T MultiplyGene(T t, double rnd)
	{
		if (typeof(T) == typeof(double))
			return BoxingSafeConverter<double, T>.Instance.Convert(BoxingSafeConverter<T, double>.Instance.Convert(t) * rnd);
		else
			return default(T);
	}
}