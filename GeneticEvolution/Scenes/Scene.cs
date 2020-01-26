using System;
using SDL2;

namespace NeuroEvolution
{
	abstract class BaseScene : IDisposable
	{
		protected BaseScene(int sceneID, bool canresize, bool maximized, bool loadaudio)
		{
			CanResize = canresize;
			CanBeMaximized = maximized;
		}

		public readonly bool CanResize, CanBeMaximized, CanLoadAudio;
		public readonly int ID;

		public bool IsDestroyed { get; private set; }

		public bool IsLoaded { get; private set; }

		public int RenderedObjectsCount { get; protected set; }

		public virtual void Update(TimeSpan elapsed)
		{
		}

		public virtual void FixedUpdate(double totalMS, double frameMS)
		{

		}

		public virtual void Dispose()
		{
			if (IsDestroyed)
				return;

			IsDestroyed = true;
			Unload();
		}


		public virtual void Load()
		{
			IsLoaded = true;
		}

		public virtual void Unload()
		{
		}

		public virtual bool Draw()
		{
			return true;
		}


		internal virtual void OnLeftMouseUp() { }
		internal virtual void OnLeftMouseDown() { }

		internal virtual void OnRightMouseUp() { }
		internal virtual void OnRightMouseDown() { }

		internal virtual void OnMiddleMouseUp() { }
		internal virtual void OnMiddleMouseDown() { }


		internal virtual bool OnLeftMouseDoubleClick() => false;
		internal virtual bool OnRightMouseDoubleClick() => false;
		internal virtual bool OnMiddleMouseDoubleClick() => false;
		internal virtual void OnMouseWheel(bool up) { }
		internal virtual void OnMouseDragging() { }
		internal virtual void OnTextInput(string text) { }
		internal virtual void OnKeyDown(SDL.SDL_KeyboardEvent e) { }
		internal virtual void OnKeyUp(SDL.SDL_KeyboardEvent e) { }
	}
}