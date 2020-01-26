using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SDL2;

namespace NeuroEvolution
{
	class WorldScene : BaseScene
	{
		EvoEngine engine;
		SpriteBatch spriteBatch;

		List<IEntity> world = new List<IEntity>();
		public List<IEntity> World { get => world; }

		SpriteFont Arial;


		public GeneticAlgorithm<double> GenePool;

		public bool DrawSensors = false;

		public WorldScene(EvoEngine engine) : base(0, false, false, false)
		{
			this.engine = engine;
		}

		public override bool Draw()
		{
			engine.GraphicsDevice.Clear(Color.CornflowerBlue);

			spriteBatch.Begin();

			foreach (IEntity entity in world)
			{
				spriteBatch.Draw(
					entity.Texture,
					entity.Location,
					null,
					entity.Color, 
					entity.VelocityRotation, 
					entity.Center, 
					entity.Scale, 
					SpriteEffects.None, 
					0f);

				if (DrawSensors && entity.GetType() == typeof(Snake))
				{
					Snake snake = (Snake)entity;
					if (snake.Energy > 0)
					{
						float[] activation = snake.GetFoodSensorsActivation();

						for (int i = 0; i < Snake.FoodSensorsCount; i++)
						{
							float[] sensorrange = snake.GetFoodSensorRange(i);
							float from = sensorrange[0] + snake.VelocityRotation;
							float to = sensorrange[1] + snake.VelocityRotation;
							Primitives2D.DrawArc(
								spriteBatch, 
								entity.Location, 
								20, 
								360, 
								NormalizeAngle(from), 
								NormalizeAngle(to - from), 
								Extensions.MixColor(Color.Red, Color.AliceBlue, 1-activation[i]), 
								3);
						}
					}
				}
			}

			//spriteBatch.DrawString(Arial, $"Generation {GenePool.Generation}", new Vector2(10, 10), Color.White);

			spriteBatch.End();

			return base.Draw();
		}

		private float NormalizeAngle(float deg)
		{
			float normalizedDeg = deg;
			float twopi = (float)Math.PI * 2;
			if (normalizedDeg < 0)
				normalizedDeg += twopi;
			if (normalizedDeg > twopi)
				normalizedDeg -= twopi;

			return normalizedDeg;
		}

		public override void Update(TimeSpan elapsed)
		{
			CheckAlive();

			if (engine.KeyState.IsKeyDown(Keys.S) && engine.OldKeyState.IsKeyUp(Keys.S))
				DrawSensors = !DrawSensors;

			if (engine.KeyState.IsKeyDown(Keys.Escape) && engine.OldKeyState.IsKeyUp(Keys.Escape))
				engine.Exit();

			Console.Clear();
			Console.WriteLine($"Generation {GenePool.Generation}");
			Console.WriteLine($"Mean Fit: {GenePool.MeanPopulationFitness}");
			foreach (IEntity entity in world)
			{
				if (entity.GetType() == typeof(Snake))
				{
					Snake snake = (Snake)entity;
					if (snake.Brain.GetType() != typeof(SnakeBrainGenetic)) continue;
					if (snake.Energy < 0)
						Console.ForegroundColor = ConsoleColor.Black;
					Console.WriteLine($"Snake Energy: {snake.Energy:00.00} - DNA Fit: {GenePool.Population[((SnakeBrainGenetic)snake.Brain).CurrentDNA].Fitness:00.00} - DNA OldFit: {GenePool.Population[((SnakeBrainGenetic)snake.Brain).CurrentDNA].OldFitness:00.00}");
					Console.ResetColor();
				}
			}

			foreach (IEntity entity in world)
			{
				entity.Update(elapsed);

				//if (entity.GetType() == typeof(Snake))
				//{
				//	Snake snake = (Snake)entity;
				//	ControlSnake(snake);
				//}
			}
			base.Update(elapsed);
		}

		private void CheckAlive()
		{
			// Pess enter to force evolution
			if (!(engine.KeyState.IsKeyDown(Keys.Space) && engine.OldKeyState.IsKeyUp(Keys.Space)))
			{
				foreach (IEntity entity in world)
				{
					if (entity.GetType() == typeof(Snake))
					{
						Snake snake = (Snake)entity;
						if (snake.Energy > 0)
							return; //At least one snake alive... continue simulation
					}
				}
			}

			// No Snakes alive, evolving DNAs
			GenePool.NewGeneration(2);
			PopulateSnakes(GenePool.Population.Count); 
		}

		private void ControlSnake(Snake snake)
		{
			//float[] activation = snake.GetFoodSensorsActivation();

			if (engine.KeyState.IsKeyDown(Keys.Up))
				snake.SetVelocity(new Vector2(snake.Velocity.X, snake.Velocity.Y - 0.1f));
			if (engine.KeyState.IsKeyDown(Keys.Down))
				snake.SetVelocity(new Vector2(snake.Velocity.X, snake.Velocity.Y + 0.1f));
			if (engine.KeyState.IsKeyDown(Keys.Left))
				snake.SetVelocity(new Vector2(snake.Velocity.X - 0.1f, snake.Velocity.Y));
			if (engine.KeyState.IsKeyDown(Keys.Right))
				snake.SetVelocity(new Vector2(snake.Velocity.X + 0.1f, snake.Velocity.Y));
		}

		public override void Load()
		{
			for (int i = 0; i < 30; i++)
			{
				Food f = new Food(engine.Content);
				f.SetRandomLocation();
				f.SetScale(0.2f);
				world.Add(f);
			}

			GenePool = new GeneticAlgorithm<double>(
				10, //Population size
				68, //DNA size, calculated by the network weights
				Extensions.Random,
				getRandomGene,
				5, //Elitism
				0.05
				);

			PopulateSnakes(GenePool.Population.Count);
			spriteBatch = new SpriteBatch(engine.GraphicsDevice);
			//Arial = engine.Content.Load<SpriteFont>("ArialSprite");


			base.Load();
		}

		private void PopulateSnakes(int n)
		{
			world.RemoveAll(x => x.GetType() == typeof(Snake));

			for (int i = 0; i < GenePool.Population.Count; i++)
			{
				Snake f = new Snake(engine.Content);
				f.SetRandomLocation();
				if (f.Brain.GetType() == typeof(SnakeBrainGenetic))
					((SnakeBrainGenetic)f.Brain).AssignDNA(i);
				world.Add(f);
			}
		}

		private double getRandomGene()
		{
			return Extensions.GetRandom(-1, 1);
		}
	}
}
