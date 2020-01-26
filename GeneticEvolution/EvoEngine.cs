using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

using SDL2;
using Microsoft.Xna.Framework.Graphics;

namespace NeuroEvolution
{
	class EvoEngine : Game
	{
		public readonly static double FPS = 250;

		private BaseScene scene;
		public BaseScene Scene => scene;

		private readonly GraphicsDeviceManager graphicDeviceManager;
		public GraphicsDeviceManager GraphicsDeviceManager => graphicDeviceManager;

		public KeyboardState KeyState;
		public KeyboardState OldKeyState;

		public EvoEngine()
		{
			graphicDeviceManager = new GraphicsDeviceManager(this);
			Content.RootDirectory = "Content";

			graphicDeviceManager.PreferredBackBufferWidth = 1280;
			graphicDeviceManager.PreferredBackBufferHeight = 900;
			//graphicDeviceManager.ToggleFullScreen();
			graphicDeviceManager.ApplyChanges();
		}

		protected override void Initialize()
		{
			Window.Title = "Evolution Engine";
			IsMouseVisible = true;
			IsFixedTimeStep = true;
			TargetElapsedTime = TimeSpan.FromMilliseconds(1000.0f / FPS);

			base.Initialize();
		}

		protected override void LoadContent()
		{
			SetScene(new WorldScene(this));
			base.LoadContent();
		}

		protected override void Update(GameTime gameTime)
		{
			OldKeyState = KeyState;
			KeyState = Keyboard.GetState();

			scene?.Update(gameTime.ElapsedGameTime);
			base.Update(gameTime);
		}

		protected override void Draw(GameTime gameTime)
		{
			scene?.Draw();
			base.Draw(gameTime);
		}

		public void SetScene(BaseScene scene)
		{
			this.scene?.Dispose();
			this.scene = scene;

			if (scene != null)
			{
				Window.AllowUserResizing = scene.CanResize;
				scene.Load();
			}
		}


	}
}
