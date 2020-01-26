using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework.Content;

namespace NeuroEvolution
{
	class Food : MovingObject
	{
		public Food(ContentManager content) : base(content)
		{
			Texture = TextureLoader.Load("apple", content);
		}
	}
}
