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
		/// <summary>
		/// How many fps the FNA engine will run
		/// </summary>
		public readonly static double FPS = 250;

		/// <summary>
		/// The simulation can have multiple scenes, but only one active at any given time
		/// </summary>
		private BaseScene scene;
		public BaseScene Scene => scene;

		/// <summary>
		/// The graphics device manager, so we can set some video settings to the simulation
		/// </summary>
		private readonly GraphicsDeviceManager graphicDeviceManager;
		public GraphicsDeviceManager GraphicsDeviceManager => graphicDeviceManager;

		/// <summary>
		/// Current frame keyboard state
		/// </summary>
		public KeyboardState KeyState;

		/// <summary>
		/// Last frame keyboard state
		/// </summary>
		public KeyboardState OldKeyState;

		/// <summary>
		/// Evolution engine constructor
		/// </summary>
		public EvoEngine()
		{
			graphicDeviceManager = new GraphicsDeviceManager(this);
			Content.RootDirectory = "Content";

			graphicDeviceManager.PreferredBackBufferWidth = 1280;
			graphicDeviceManager.PreferredBackBufferHeight = 900;
			//graphicDeviceManager.ToggleFullScreen();
			graphicDeviceManager.ApplyChanges();
		}

		/// <summary>
		/// This method is called once right after the constructor, so we can set some FNA parameters
		/// </summary>
		protected override void Initialize()
		{
			Window.Title = "Evolution Engine";
			IsMouseVisible = true;
			IsFixedTimeStep = true;
			TargetElapsedTime = TimeSpan.FromMilliseconds(1000.0f / FPS);

			base.Initialize();
		}

		/// <summary>
		/// Called when the graphics engine request the content load, so we can have as many resources ready as we can
		/// </summary>
		protected override void LoadContent()
		{
			SetScene(new WorldScene(this));
			base.LoadContent();
		}

		/// <summary>
		/// Called right before every frame so we can update/process the engine mechanics
		/// </summary>
		/// <param name="gameTime"></param>
		protected override void Update(GameTime gameTime)
		{
			OldKeyState = KeyState;
			KeyState = Keyboard.GetState();

			scene?.Update(gameTime.ElapsedGameTime);
			base.Update(gameTime);
		}

		/// <summary>
		/// Called after Update() method, so we can draw everything inside our scene
		/// </summary>
		/// <param name="gameTime"></param>
		protected override void Draw(GameTime gameTime)
		{
			scene?.Draw();
			base.Draw(gameTime);
		}

		/// <summary>
		/// Set current scene and dispose the old one
		/// </summary>
		/// <param name="scene"></param>
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
