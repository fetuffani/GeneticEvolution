using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NeuroEvolution
{
	class SnakeBrain
	{
		public Snake Snake => snake;
		private Snake snake;

		public SnakeBrain(Snake snake)
		{
			this.snake = snake;
		}

		internal virtual void Think(EvoEngine engine)
		{
			Food food = GetClosestFood(engine);

			if (food == null) return; // Do nothing if no food is found

			if (food.Location.X > snake.Location.X)
				snake.AccelerateRight();
			else
				snake.AccelerateLeft();

			if (food.Location.Y > snake.Location.Y)
				snake.AccelerateDown();
			else
				snake.AccelerateUp();
		}

		private Food GetClosestFood(EvoEngine engine)
		{
			WorldScene scene = Extensions.GetWorldScene();
			Food food = null;
			float dist = float.MaxValue;

			foreach (IEntity entity in scene.World)
			{
				if (entity.GetType() == typeof(Food))
				{
					float fooddist = Extensions.GetVectorDistance(snake.Location, entity.Location);
					if (fooddist < dist && dist > 50.0f) //Ensure we do not circle current food
					{
						food = (Food)entity;
						dist = fooddist;
					}
				}
			}

			return food;
		}
	}
}