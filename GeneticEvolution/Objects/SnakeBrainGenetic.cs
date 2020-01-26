using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Accord.Neuro;
using Accord.Neuro.Networks;

namespace NeuroEvolution
{
	class SnakeBrainGenetic : SnakeBrain
	{
		ActivationNetwork Network;
		public int CurrentDNA;

		public SnakeBrainGenetic(Snake snake) : base(snake)
		{
		}


		internal void AssignDNA(int i)
		{
			CurrentDNA = i;
		}

		internal override void Think(EvoEngine engine)
		{
			if (Network == null)
			{
				Network = new ActivationNetwork(new BipolarSigmoidFunction(), Snake.FoodSensorsCount, 4, 4);
				// Nw = (I+1)*H1 +(H1+1)*H2 +(H2+1)*O
				// I = inputs
				// H1 = neurons in hidden layer 1
				// H2 = neurons in hidden layer 2
				// O = Number of outputs
				// Nw = (5+1)*0 + (0+1)*0 + (5+1)*4 = 24 // 5in 4out
				// Nw = (5+1)*4 + (4+1)*0 + (4+1)*4 = 44 // 5in 1hl4 4out
				// Nw = (11+1)*4 + (4+1)*0 + (4+1)*4 = 68 // 68in 1hl4 4out

				SetNetworkWeights(GetCurrentGene(GeneticEvolution.Engine));
			}

			if (engine.KeyState.GetPressedKeys().Length > 0)
				return;

			double[] netout = Network.Compute(convertToDouble(Snake.GetFoodSensorsActivation()));

			//if (Math.Max(netout[0], netout[1]) > 0.5)
				if (netout[0] > netout[1])
					Snake.Accelerate();
				else
					Snake.Brake();

			//if (Math.Max(netout[2], netout[3]) > 0.5)
				if (netout[2] > netout[3])
					Snake.RotateLeft();
				else
					Snake.RotateRight();

		}

		public static double[] convertToDouble(float[] inputArray)
		{
			if (inputArray == null)
				return null;

			double[] output = new double[inputArray.Length];
			for (int i = 0; i < inputArray.Length; i++)
				output[i] = inputArray[i];

			return output;
		}

		private double[] GetNetworkWeights()
		{
			List<double> coefs = new List<double>();

			foreach (var layer in Network.Layers)
			{
				foreach (var neur in layer.Neurons)
				{
					foreach (var weig in neur.Weights)
					{
						coefs.Add(weig);
					}
				}
			}

			return coefs.ToArray();
		}

		private void SetNetworkWeights(DNA<double> dna)
		{
			int index = 0;
			foreach (var layer in Network.Layers)
			{
				foreach (var neur in layer.Neurons)
				{
					for (int i = 0; i < neur.Weights.Length; i++)
					{
						neur.Weights[i] = dna.Genes[index];
						index++;
					}
				}
			}
		}

		private DNA<double> GetCurrentGene(EvoEngine engine)
		{
			WorldScene scene = Extensions.GetWorldScene();
			DNA<double> dna = scene.GenePool.Population[CurrentDNA];

			return dna;
		}
	}
}