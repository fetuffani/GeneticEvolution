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
				Network = new ActivationNetwork(new BipolarSigmoidFunction(), WorldScene.NeuralNetworkShape[0], WorldScene.NeuralNetworkShape.Skip(1).ToArray());
				// Nw = (I+1)*H1 +(H1+1)*H2 +(H2+1)*O
				// I = inputs
				// H1 = neurons in hidden layer 1
				// H2 = neurons in hidden layer 2
				// O = Number of outputs
				// Nw = (5+1)*0 + (0+1)*0 + (5+1)*4 = 24 // 5in 4out
				// Nw = (5+1)*4 + (4+1)*0 + (4+1)*4 = 44 // 5in 1hl4 4out
				// Nw = (11+1)*4 + (4+1)*0 + (4+1)*4 = 68 // 11in 1hl4 4out

				var dna = GetCurrentGene();
				SetNetworkWeights(dna);
				//float scale = (float)dna.Genes[scene.NeuralNetworkWeightsCount - 1 + 0];
				//if (scale < 0.4) scale = 0.4f;
				//if (scale > 0.6) scale = 0.6f;
				//Snake.SetScale(scale);

				//int div = scene.NeuralNetworkWeightsCount / 3;
				//Color c = new Color(
				//	(int)Math.Max((dna.Genes[div*0] * 255), 128),
				//	(int)Math.Max((dna.Genes[div*1] * 255), 128),
				//	(int)Math.Max((dna.Genes[div*2] * 255), 128)
				//	);
				//Snake.Color = c;
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

					coefs.Add(((ActivationNeuron)neur).Threshold);
				}
			}

			return coefs.ToArray();
		}

		private void SetNetworkWeights(DNA<double> dna)
		{
			WorldScene scene = Extensions.GetWorldScene();

			// Get only the network weights
			double[] genes = dna.Genes.Take(scene.NeuralNetworkWeightsCount).ToArray();

			int index = 0;
			foreach (var layer in Network.Layers)
			{
				foreach (var neur in layer.Neurons)
				{
					for (int i = 0; i < neur.Weights.Length; i++)
					{
						neur.Weights[i] = genes[index++];
					}

					((ActivationNeuron)neur).Threshold = genes[index++];
				}
			}

			if (index != scene.NeuralNetworkWeightsCount)
				throw new Exception("An error ocurred while retrieving the network weights!");
		}

		private DNA<double> GetCurrentGene()
		{
			WorldScene scene = Extensions.GetWorldScene();
			DNA<double> dna = scene.GenePool.Population[CurrentDNA];

			return dna;
		}
	}
}