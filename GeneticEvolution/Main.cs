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
		/// <summary>
		/// Store the Engine so it can be accessed anywhere
		/// </summary>
		public static EvoEngine Engine;

		/// <summary>
		/// Simulation entry point
		/// </summary>
		/// <param name="args">not used</param>
		public static void Main(string[] args)
		{
			// Create and run the simulation engine
			Engine = new EvoEngine();
			Engine.Run();
			Engine.Dispose();
		}
	}
}
