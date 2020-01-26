using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NeuroEvolution
{
	interface IEntity
	{
		Vector2 Location { get; }
		Vector2 Velocity { get; }
		Vector2 Center { get; }

		Texture2D Texture { get; set; }
		//Rectangle Rectange { get; set; }
		Color Color { get; set; }

		float Scale { get; set; }
		float VelocityRotation { get; }

		void Update(TimeSpan elapsed);
		void SetLocation(Vector2 location);
		void SetRandomLocation();
		void SetScale(float scale);
		void RotateVelocity(float rad);
	}
}
