using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Accord.Neuro;

namespace NeuroEvolution
{
	class GeneticEvolution
	{
		public static EvoEngine Engine;
		public static void Main(string[] args)
		{
			Engine = new EvoEngine();
			Engine.Run();
			Engine.Dispose();
		}

		//public static void Main()
		//{
		//	ActivationNetwork Network = new ActivationNetwork(new SigmoidFunction(), Snake.FoodSensorsCount, 1);
		//	double[] weights = GetNetworkWeights(Network);

		//	foreach (double d in weights)
		//	{
		//		Console.Write($"{d:0.00} ");
		//	}

		//	Console.WriteLine();

		//	ActivationNetwork Network2 = new ActivationNetwork(new SigmoidFunction(), Snake.FoodSensorsCount, 1);
		//	SetNetworkWeights(Network2, weights);
		//	weights = GetNetworkWeights(Network2);

		//	foreach (double d in weights)
		//	{
		//		Console.Write($"{d:0.00} ");
		//	}

		//	Console.WriteLine();

		//	ActivationNetwork Network3 = new ActivationNetwork(new SigmoidFunction(), Snake.FoodSensorsCount, 1);
		//	SetNetworkWeights(Network3, weights);
		//	weights = GetNetworkWeights(Network3);

		//	foreach (double d in weights)
		//	{
		//		Console.Write($"{d:0.00} ");
		//	}

		//	Console.WriteLine();



		//	Console.Read();
		//}

		//private static double[] GetNetworkWeights(ActivationNetwork Network)
		//{
		//	List<double> coefs = new List<double>();

		//	foreach (var layer in Network.Layers)
		//	{
		//		foreach (var neur in layer.Neurons)
		//		{
		//			foreach (var weig in neur.Weights)
		//			{
		//				coefs.Add(weig);
		//			}
		//		}
		//	}

		//	return coefs.ToArray();
		//}

		//private static void SetNetworkWeights(ActivationNetwork Network, double[] dna)
		//{
		//	int index = 0;
		//	foreach (var layer in Network.Layers)
		//	{
		//		foreach (var neur in layer.Neurons)
		//		{
		//			for (int i = 0; i < neur.Weights.Length; i++)
		//			{
		//				neur.Weights[i] = dna[index];
		//				index++;
		//			}
		//		}
		//	}
		//}
	}
}
