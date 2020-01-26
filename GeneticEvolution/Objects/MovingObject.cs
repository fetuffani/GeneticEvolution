using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Content;

namespace NeuroEvolution
{
	class MovingObject : IEntity
	{
		private Vector2 location;
		public Vector2 Location => location;

		private Vector2 velocity;
		public Vector2 Velocity => velocity;

		public Texture2D texture;
		public virtual Texture2D Texture { get { return texture; } set { texture = value; center = new Vector2((float)texture.Width/2, (float)texture.Height/2); } }
		public float Scale { get; set; }

		public virtual Color Color { get; set; }

		public float VelocityRotation => (float)Math.Atan2(velocity.Y, velocity.X);

		private Vector2 center;
		public Vector2 Center { get { return center; } }

		public virtual void Update(TimeSpan elapsed)
		{
			location += Extensions.ScaleVector(velocity, (float)elapsed.TotalSeconds);
		}

		public virtual void SetLocation(Vector2 location)
		{
			this.location = location;
		}

		public virtual void SetVelocity(Vector2 velocity)
		{
			this.velocity = velocity;
		}

		public virtual void SetScale(float scale)
		{
			Scale = scale;
		}

		public virtual void RotateVelocity(float rad)
		{
			velocity = Extensions.RotateVector(velocity, rad);
		}

		public virtual void SetRandomLocation()
		{
			SetLocation(new Vector2(
					Extensions.GetUnitRandomFloat() * GeneticEvolution.Engine.GraphicsDeviceManager.PreferredBackBufferWidth,
					Extensions.GetUnitRandomFloat() * GeneticEvolution.Engine.GraphicsDeviceManager.PreferredBackBufferHeight
					));
		}

		public MovingObject(ContentManager content)
		{
			location = new Vector2(0f, 0f);
			velocity = new Vector2(0f, 0f);
			Scale = 1.0f;
			Color = Color.White;
		}
	}
}
