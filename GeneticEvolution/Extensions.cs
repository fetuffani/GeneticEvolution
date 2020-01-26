using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace NeuroEvolution
{
	/// <summary>
	/// Used to safely convert generic types
	/// </summary>
	/// <typeparam name="TIn">input type</typeparam>
	/// <typeparam name="TOut">output type</typeparam>
	public sealed class BoxingSafeConverter<TIn, TOut>
	{
		public static readonly BoxingSafeConverter<TIn, TOut> Instance = new BoxingSafeConverter<TIn, TOut>();
		private readonly Func<TIn, TOut> convert;

		public Func<TIn, TOut> Convert
		{
			get { return convert; }
		}

		private BoxingSafeConverter()
		{
			if (typeof(TIn) != typeof(TOut))
			{
				throw new InvalidOperationException("Both generic type parameters must represent the same type.");
			}
			var paramExpr = Expression.Parameter(typeof(TIn));
			convert =
				Expression.Lambda<Func<TIn, TOut>>(paramExpr, // this conversion is legal as typeof(TIn) = typeof(TOut)
					paramExpr)
					.Compile();
		}
	}

	/// <summary>
	/// Extension methods
	/// </summary>
	class Extensions
	{
		/// <summary>
		/// Get current WorldScene from the simulation engine
		/// </summary>
		/// <returns>Current WorldScene</returns>
		/// <exception cref="Exception">If the current scene is not an WorldScene, and exception will be thrown</exception>
		public static WorldScene GetWorldScene()
		{
			if (GeneticEvolution.Engine.Scene.GetType() != typeof(WorldScene))
				throw new Exception("The current running scene is not WorldScene. Cannot capture the food on the world");

			WorldScene scene = (WorldScene)GeneticEvolution.Engine.Scene;
			return scene;
		}

		/// <summary>
		/// Our current Random class
		/// </summary>
		public static Random Random = new Random();

		/// <summary>
		/// Scale a Vector2 by d units
		/// </summary>
		/// <param name="v">the vector</param>
		/// <param name="d">how many units to scale</param>
		/// <returns></returns>
		public static Vector2 ScaleVector(Vector2 v, float d)
		{
			return new Vector2(v.X * d, v.Y * d);
		}

		/// <summary>
		/// Rotate a Vector2 by rad radians
		/// </summary>
		/// <param name="v">the vector</param>
		/// <param name="rad">how many radians to rotate</param>
		/// <returns></returns>
		public static Vector2 RotateVector(Vector2 v, float rad)
		{
			return new Vector2(
				(float)Math.Cos(rad) * v.X - (float)Math.Sin(rad) * v.Y,
				(float)Math.Sin(rad) * v.X + (float)Math.Cos(rad) * v.Y
				);
		}

		/// <summary>
		/// Get a random double number
		/// </summary>
		/// <param name="min">minum value</param>
		/// <param name="max">maximum value</param>
		/// <returns></returns>
		public static double GetRandom(double min = 0, double max = 1)
		{
			return Random.NextDouble() * (max - min) + min;
		}

		/// <summary>
		/// Get a random float number
		/// </summary>
		/// <param name="min">minum value</param>
		/// <param name="max">maximum value</param>
		/// <returns></returns>
		public static float GetRandomFloat(float min = 0, float max = 0)
		{
			return (float)GetRandom(min, max);
		}

		/// <summary>
		/// Get a random float number between zero and one
		/// </summary>
		/// <returns></returns>
		public static float GetUnitRandomFloat()
		{
			return (float)GetRandom();
		}

		/// <summary>
		/// Get the distance between two vectors
		/// </summary>
		/// <param name="v1">vector one</param>
		/// <param name="v2">vector two</param>
		/// <returns></returns>
		public static float GetVectorDistance(Vector2 v1, Vector2 v2)
		{
			return (float)Math.Sqrt(Math.Pow((float)(v1.X - v2.X), 2) + Math.Pow((float)(v1.Y - v2.Y), 2));
		}

		/// <summary>
		/// Scale a Rectangle
		/// </summary>
		/// <param name="rectange">the rectangle</param>
		/// <param name="scale">how many units to scale</param>
		/// <returns></returns>
		public static Rectangle ScaleRectangle(Rectangle rectange, double scale)
		{
			return new Rectangle(rectange.X, rectange.Y, (int)(rectange.Width * scale), (int)(rectange.Height * scale));
		}

		/// <summary>
		/// Sigmoid function
		/// </summary>
		/// <see cref="https://en.wikipedia.org/wiki/Sigmoid_function"/>
		/// <param name="x">the x parameter</param>
		/// <returns></returns>
		public static float SigmoidFunction(float x)
		{
			return 1.0f / (1 + (float)Math.Exp((float)-x));
		}

		/// <summary>
		/// Maximum value for the sigmoid function, so we can normalize the derivative
		/// </summary>
		public static readonly float SigmoidFunctionDiffMax = SigmoidFunctionDiff(0.0f);

		/// <summary>
		/// The derivative of the sigmoid function
		/// </summary>
		/// <param name="x">the x parameter</param>
		/// <returns></returns>
		public static float SigmoidFunctionDiff(float x)
		{
			float sigmoid = SigmoidFunction(x);
			return sigmoid * (1.0f - sigmoid);
		}

		/// <summary>
		/// Normalized sigmoid function (between zero and one)
		/// </summary>
		/// <param name="x"></param>
		/// <returns></returns>
		public static float SigmoidFunctionDiffNormalized(float x)
		{
			return SigmoidFunctionDiff(x) / SigmoidFunctionDiffMax;
		}

		/// <summary>
		/// Mix two colors
		/// </summary>
		/// <param name="basecolor">the base color</param>
		/// <param name="mixcolor">the mix color to blend</param>
		/// <param name="v">ratio between the colors, value between 0 and 1</param>
		/// <returns></returns>
		internal static Color MixColor(Color basecolor, Color mixcolor, float v)
		{
			return new Color(
				(int)(basecolor.R * (1.0f - v)) + (int)(mixcolor.R * (v)),
				(int)(basecolor.G * (1.0f - v)) + (int)(mixcolor.G * (v)),
				(int)(basecolor.B * (1.0f - v)) + (int)(mixcolor.B * (v))
				);
		}

		/// <summary>
		/// Calculate the number of weights in the Neural Network based on the shape
		/// </summary>
		/// <param name="shape">the shape array, including the inputs and outputs</param>
		/// <returns></returns>
		internal static int NeuralNetworkWeightCount(int[] shape)
		{
			// Nw = (I+1)*H1 +(H1+1)*H2 +(H2+1)*O
			// I = inputs
			// H1 = neurons in hidden layer 1
			// H2 = neurons in hidden layer 2
			// O = Number of outputs
			// Nw = (5+1)*0 + (0+1)*0 + (5+1)*4 = 24 // 5in 4out
			// Nw = (5+1)*4 + (4+1)*0 + (4+1)*4 = 44 // 5in 1hl4 4out
			// Nw = (11+1)*4 + (4+1)*0 + (4+1)*4 = 68 // 11in 1hl4 4out
			// Nw = (11+1)*5 + (5+1)*4 + (4+1)*4 = 104// 11in 1hl5 2hl4 4out
			int count = 0;

			for (int i = 0; i < shape.Length - 2; i++)
			{
				count += (shape[i] + 1) * shape[i + 1];
			}

			count += (shape[shape.Length - 2] + 1) * shape[shape.Length - 1];

			return count;
		}
	}
}
