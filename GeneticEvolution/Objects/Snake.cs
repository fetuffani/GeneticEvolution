using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NeuroEvolution
{
	class Snake : MovingObject
	{
		/// <summary>
		/// How many food sensors the snake will have
		/// Sensors are not straight line but a wide sensor
		/// </summary>
		public static int FoodSensorsCount = WorldScene.NeuralNetworkShape[0];

		/// <summary>
		/// How wide the sensors will sense
		/// (in radians)
		/// </summary>
		public static float FoodSensorsSpread = ((float)Math.PI/180.0f) * 90.0f;

		public double Energy;

		public SnakeBrain Brain;

		private Color basecolor = Color.White;
		public override Color Color {
			get
			{
				return Extensions.MixColor(basecolor, Color.Brown, Extensions.SigmoidFunctionDiffNormalized((float)Energy));
			}

			set => basecolor = value;
		}

		public Snake(ContentManager content) : base(content)
		{
			Texture = TextureLoader.Load("snakeheadright", content);
			Scale = 0.5f;

			Brain = new SnakeBrainGenetic(this);
			Energy = 10;
		}

		public override void Update(TimeSpan elapsed)
		{
			WorldScene scene = Extensions.GetWorldScene();

			
			if (Energy < 0)
			{
				return; // We are dead now
			}
			if (Brain.GetType() == typeof(SnakeBrainGenetic))
				Energy -= 1 / EvoEngine.FPS;
			var windowengine = GeneticEvolution.Engine.GraphicsDeviceManager;

			float x = 0f, y = 0f;
			if (Location.X > windowengine.PreferredBackBufferWidth)
				x = Location.X % windowengine.PreferredBackBufferWidth;
			else if (Location.X < 0)
				x = Location.X + windowengine.PreferredBackBufferWidth;
			else
				x = Location.X;

			if (Location.Y > windowengine.PreferredBackBufferHeight)
				y = Location.Y % windowengine.PreferredBackBufferHeight;
			else if (Location.Y < 0)
				y = Location.Y + windowengine.PreferredBackBufferHeight;
			else
				y = Location.Y;
		
			SetLocation(new Vector2(x, y));

			CheckEatFood();


			if (Brain != null)
				Brain.Think(GeneticEvolution.Engine);

			base.Update(elapsed);
		}

		private void CheckEatFood()
		{
			WorldScene scene = Extensions.GetWorldScene();

			foreach (IEntity entity in scene.World)
			{
				if (entity == this) continue;
				if (entity.GetType() != typeof(Food)) continue;

				if (Extensions.GetVectorDistance(Location, entity.Location) < 20)
				{
					// in order to eat, the snake must face it
					float angletofood = (float)Math.Atan2(entity.Location.Y - Location.Y, entity.Location.X - Location.X) - VelocityRotation;
					float minangle = -FoodSensorsSpread;
					float maxangle = +FoodSensorsSpread;
					if (angletofood > minangle && angletofood < maxangle)
					{
						entity.SetRandomLocation();
						if (Brain.GetType() == typeof(SnakeBrainGenetic))
							scene.GenePool.Population[((SnakeBrainGenetic)Brain).CurrentDNA].Fitness++;
						Energy += 2;
					}
				}
			}
		}

		public float[] GetFoodSensorRange(int sensor)
		{
			if (sensor >= FoodSensorsCount)
				throw new Exception("Sensor not found");
			else if (sensor < 0)
				throw new Exception($"Sensor number must zero or less than {FoodSensorsCount}");
			else
			{
				float from = ((float)(sensor + 0) / (float)FoodSensorsCount) * FoodSensorsSpread - FoodSensorsSpread / 2.0f;
				float to =   ((float)(sensor + 1) / (float)FoodSensorsCount) * FoodSensorsSpread - FoodSensorsSpread / 2.0f;

				return new float[] { from, to };
			}
		}

		public void AccelerateUp()
		{
			SetVelocity(new Vector2(Velocity.X, Velocity.Y - 0.1f));
		}

		public void AccelerateDown()
		{
			SetVelocity(new Vector2(Velocity.X, Velocity.Y + 0.1f));
		}

		public void AccelerateLeft()
		{
			SetVelocity(new Vector2(Velocity.X - 0.1f, Velocity.Y));
		}

		public void AccelerateRight()
		{
			SetVelocity(new Vector2(Velocity.X + 0.1f, Velocity.Y));
		}

		public void Accelerate()
		{
			if (Velocity.Length() < 0.1)
				SetVelocity(new Vector2(0.1f, 0.1f));

			if (Velocity.Length() < 100)
				SetVelocity(Extensions.ScaleVector(Velocity, 1.1f));
		}

		public void Brake()
		{
			SetVelocity(Extensions.ScaleVector(Velocity, 0.9f));
		}

		public void RotateLeft()
		{
			SetVelocity(Extensions.RotateVector(Velocity, 0.05f));
		}

		public void RotateRight()
		{
			SetVelocity(Extensions.RotateVector(Velocity, -0.05f));
		}

		public float[] GetFoodSensorsActivation()
		{
			WorldScene scene = Extensions.GetWorldScene();

			float[] activation = new float[FoodSensorsCount];

			for (int i = 0; i < FoodSensorsCount; i++)
			{
				float[] sensorrange = GetFoodSensorRange(i);
				foreach (IEntity ent in scene.World)
				{
					if (ent.GetType() != typeof(Food)) continue;
					Food food = (Food)ent;

					float angletofood = (float)Math.Atan2(food.Location.Y - Location.Y, food.Location.X - Location.X) - VelocityRotation;

					if (angletofood > sensorrange[0] && angletofood < sensorrange[1])
					{
						float distance = Extensions.GetVectorDistance(Location, food.Location);
						float actv = (float)Math.Sqrt(Extensions.SigmoidFunctionDiffNormalized(distance / 100.0f));
						if (activation[i] < actv)
							activation[i] = actv;

					}
				}

			}

			return activation;

		}
	}
}
