using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace NeuroEvolution
{
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

	class Extensions
	{
		public static WorldScene GetWorldScene()
		{
			if (GeneticEvolution.Engine.Scene.GetType() != typeof(WorldScene))
				throw new Exception("The current running scene is not WorldScene. Cannot capture the food on the world");

			WorldScene scene = (WorldScene)GeneticEvolution.Engine.Scene;
			return scene;
		}

		public static Random Random = new Random();

		public static Vector2 ScaleVector(Vector2 v, float d)
		{
			return new Vector2(v.X * d, v.Y * d);
		}

		public static Vector2 RotateVector(Vector2 v, float rad)
		{
			return new Vector2(
				(float)Math.Cos(rad) * v.X - (float)Math.Sin(rad) * v.Y,
				(float)Math.Sin(rad) * v.X + (float)Math.Cos(rad) * v.Y
				);
		}

		public static double GetRandom(double min = 0, double max = 1)
		{
			return Random.NextDouble() * (max - min) + min;
		}

		public static float GetRandomFloat(float min = 0, float max = 0)
		{
			return (float)GetRandom(min, max);
		}

		public static float GetUnitRandomFloat()
		{
			return (float)GetRandom();
		}

		public static float GetVectorDistance(Vector2 v1, Vector2 v2)
		{
			return (float)Math.Sqrt(Math.Pow((float)(v1.X - v2.X), 2) + Math.Pow((float)(v1.Y - v2.Y), 2));
		}

		public static Rectangle ScaleRectangle(Rectangle rectange, double scale)
		{
			return new Rectangle(rectange.X, rectange.Y, (int)(rectange.Width * scale), (int)(rectange.Height * scale));
		}

		public static float SigmoidFunction(float x)
		{
			return 1.0f / (1 + (float)Math.Exp((float)-x));
		}

		public static readonly float SigmoidFunctionDiffMax = SigmoidFunctionDiff(0.0f);
		public static float SigmoidFunctionDiff(float x)
		{
			float sigmoid = SigmoidFunction(x);
			return sigmoid * (1.0f - sigmoid);
		}

		public static float SigmoidFunctionDiffNormalized(float x)
		{
			return SigmoidFunctionDiff(x) / SigmoidFunctionDiffMax;
		}

		internal static Color MixColor(Color basecolor, Color mixcolor, float v)
		{
			return new Color(
				(int)(basecolor.R * (1.0f - v)) + (int)(mixcolor.R * (v)),
				(int)(basecolor.G * (1.0f - v)) + (int)(mixcolor.G * (v)),
				(int)(basecolor.B * (1.0f - v)) + (int)(mixcolor.B * (v))
				);
		}
	}
}
