/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "bgfx_p.h"

#if (BGFX_CONFIG_RENDERER_OPENGLES || BGFX_CONFIG_RENDERER_OPENGL)
#	include "renderer_gl.h"

#	if BGFX_USE_SDL2

namespace bgfx { namespace gl
{
	void* eglOpen()
	{
		return NULL;
	}

	void eglClose(void* /*_handle*/)
	{
	}

#	define GL_IMPORT(_optional, _proto, _func, _import) _proto _func = NULL
#	include "glimports.h"

	static EGLint s_contextAttrs[16];

	struct SwapChainGL
	{
		SwapChainGL(SDL_Window* _window, SDL_GLContext _context)
			: m_window(_window)
		{
            		//EGLSurface defaultSurface = eglGetCurrentSurface(EGL_DRAW);

			//m_surface = eglCreateWindowSurface(m_display, _config, _nwh, NULL);
			//BGFX_FATAL(m_surface != EGL_NO_SURFACE, Fatal::UnableToInitialize, "Failed to create surface.");

			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1)
			m_context = SDL_GL_CreateContext(m_window); //eglCreateContext(m_display, _config, _context, s_contextAttrs);
			SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0)
			BX_ASSERT(NULL != m_context, "Create swap chain failed: %s", SDL_GetError() );

			makeCurrent();
			GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f) );
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT) );
			swapBuffers();
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT) );
			swapBuffers();
            		SDL_GL_MakeCurrent(m_window, _context);
		}

		~SwapChainGL()
		{
            		//EGLSurface defaultSurface = eglGetCurrentSurface(EGL_DRAW);
            		SDL_GLContext defaultContext = SDL_GL_GetCurrentContext(); //eglGetCurrentContext();
			SDL_GL_MakeCurrent(m_window, 0); //EGL_NO_CONTEXT);
			SDL_GL_DestroyContext(m_window, m_context);
			//eglDestroySurface(m_display, m_surface);
            		SDL_GL_MakeCurrent(m_window, defaultContext);
		}

		void makeCurrent()
		{
			SDL_GL_MakeCurrent(m_window, m_context);
		}

		void swapBuffers()
		{
			SDL_GL_SwapWindow(m_window);
		}

		SDL_GLContext m_context;
		SDL_Window* m_window;
	};

	void GlContext::create(uint32_t _width, uint32_t _height)
	{
		m_eglLibrary = eglOpen();

		if (NULL == g_platformData.context)
		{
			BX_UNUSED(_width, _height);
			//EGLNativeWindowType  nwh = (EGLNativeWindowType )g_platformData.nwh;

			m_window = SDL_GL_GetCurrentWindow();
			BGFX_FATAL(m_window != NULL, Fatal::UnableToInitialize, "Failed to retrieve SDL/GL window");

			/*EGLint major = 0;
			EGLint minor = 0;
			EGLBoolean success = eglInitialize(m_display, &major, &minor);
			BGFX_FATAL(success && major >= 1 && minor >= 3, Fatal::UnableToInitialize, "Failed to initialize %d.%d", major, minor);

			BX_TRACE("EGL info:");
			const char* clientApis = eglQueryString(m_display, EGL_CLIENT_APIS);
			BX_TRACE("   APIs: %s", clientApis); BX_UNUSED(clientApis);

			const char* vendor = eglQueryString(m_display, EGL_VENDOR);
			BX_TRACE(" Vendor: %s", vendor); BX_UNUSED(vendor);

			const char* version = eglQueryString(m_display, EGL_VERSION);
			BX_TRACE("Version: %s", version); BX_UNUSED(version);

			const char* extensions = eglQueryString(m_display, EGL_EXTENSIONS);
			BX_TRACE("Supported EGL extensions:");
			dumpExtensions(extensions);

			// https://www.khronos.org/registry/EGL/extensions/ANDROID/EGL_ANDROID_recordable.txt
			const bool hasEglAndroidRecordable = !bx::findIdentifierMatch(extensions, "EGL_ANDROID_recordable").isEmpty();*/

			const uint32_t gles = BGFX_CONFIG_RENDERER_OPENGLES;

			// TODO Create SDL_GLContext

			/*EGLint attrs[] =
			{
				EGL_RENDERABLE_TYPE, (gles >= 30) ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT,

				EGL_BLUE_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_RED_SIZE, 8,
				EGL_ALPHA_SIZE, 8,

#	if BX_PLATFORM_ANDROID
				EGL_DEPTH_SIZE, 16,
#	else
				EGL_DEPTH_SIZE, 24,
#	endif // BX_PLATFORM_
				EGL_STENCIL_SIZE, 8,

				// Android Recordable surface
				//hasEglAndroidRecordable ? 0x3142 : EGL_NONE,
				//hasEglAndroidRecordable ? 1      : EGL_NONE,

				EGL_NONE
			};

			EGLint numConfig = 0;
			success = eglChooseConfig(m_display, attrs, &m_config, 1, &numConfig);
			BGFX_FATAL(success, Fatal::UnableToInitialize, "eglChooseConfig");

#	if BX_PLATFORM_ANDROID

			EGLint format;
			eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry( (ANativeWindow*)g_platformData.nwh, _width, _height, format);

#	endif // BX_PLATFORM_ANDROID

			m_surface = eglCreateWindowSurface(m_display, m_config, nwh, NULL);
			BGFX_FATAL(m_surface != EGL_NO_SURFACE, Fatal::UnableToInitialize, "Failed to create surface.");

			const bool hasEglKhrCreateContext = !bx::findIdentifierMatch(extensions, "EGL_KHR_create_context").isEmpty();
			const bool hasEglKhrNoError       = !bx::findIdentifierMatch(extensions, "EGL_KHR_create_context_no_error").isEmpty();

			for (uint32_t ii = 0; ii < 2; ++ii)
			{
				bx::StaticMemoryBlockWriter writer(s_contextAttrs, sizeof(s_contextAttrs) );

				EGLint flags = 0;

				if (hasEglKhrCreateContext)
				{
					bx::write(&writer, EGLint(EGL_CONTEXT_MAJOR_VERSION_KHR) );
					bx::write(&writer, EGLint(gles / 10) );

					bx::write(&writer, EGLint(EGL_CONTEXT_MINOR_VERSION_KHR) );
					bx::write(&writer, EGLint(gles % 10) );

					flags |= BGFX_CONFIG_DEBUG && hasEglKhrNoError ? 0
						| EGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR
						: 0
						;

					if (0 == ii)
					{
						flags |= BGFX_CONFIG_DEBUG ? 0
							| EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
//							| EGL_OPENGL_ES3_BIT_KHR
							: 0
							;

						bx::write(&writer, EGLint(EGL_CONTEXT_FLAGS_KHR) );
						bx::write(&writer, flags);
					}
				}
				else
				{
					bx::write(&writer, EGLint(EGL_CONTEXT_CLIENT_VERSION) );
					bx::write(&writer, 2);
				}

				bx::write(&writer, EGLint(EGL_NONE) );

				SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0)
				m_context = SDL_GL_CreateContext(m_window); //, m_config, EGL_NO_CONTEXT, s_contextAttrs);
				if (NULL != m_context)
				{
					break;
				}

				BX_TRACE("Failed to create GL context with EGL_CONTEXT_FLAGS_KHR (%08x).", flags);
			}*/

			BGFX_FATAL(m_context != NULL, Fatal::UnableToInitialize, "Failed to create context.");

			success = SDL_GL_MakeCurrent(m_window, m_context);
			BGFX_FATAL(success, Fatal::UnableToInitialize, "Failed to set context.");
			m_current = NULL;

			SDL_GL_SetSwapInterval(0);
		}

		import();

		g_internalData.context = m_context;
	}

	void GlContext::destroy()
	{
		if (NULL != m_window)
		{
			SDL_GL_MakeCurrent(m_window, 0); //, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			SDL_GL_DestroyContext(m_window, m_context);
			//eglDestroySurface(m_display, m_surface);
			//eglTerminate(m_display);
			m_context = NULL;
		}

		eglClose(m_eglLibrary);

	}

	void GlContext::resize(uint32_t _width, uint32_t _height, uint32_t _flags)
	{
#	if BX_PLATFORM_EMSCRIPTEN
		EMSCRIPTEN_CHECK(emscripten_set_canvas_element_size(HTML5_TARGET_CANVAS_SELECTOR, _width, _height) );
#	else
		if (NULL != m_window)
		{
			SDL_SetWindowSize(m_window, _width, _height);
		}
		BX_UNUSED(_width, _height);
#	endif // BX_PLATFORM_*

		if (NULL != m_window)
		{
			bool vsync = !!(_flags&BGFX_RESET_VSYNC);
			SDL_GL_SetSwapInterval(vsync ? 1 : 0);
		}
	}

	uint64_t GlContext::getCaps() const
	{
		return BX_ENABLED(0
						| BX_PLATFORM_LINUX
						| BX_PLATFORM_WINDOWS
						| BX_PLATFORM_ANDROID
						)
			? BGFX_CAPS_SWAP_CHAIN
			: 0
			;
	}

	SwapChainGL* GlContext::createSwapChain(void* _nwh)
	{
		return BX_NEW(g_allocator, SwapChainGL)(m_window, m_context);
	}

	void GlContext::destroySwapChain(SwapChainGL* _swapChain)
	{
		BX_DELETE(g_allocator, _swapChain);
	}

	void GlContext::swap(SwapChainGL* _swapChain)
	{
		makeCurrent(_swapChain);

		if (NULL == _swapChain)
		{
			if (NULL != m_window)
			{
				SDL_GL_SwapWindow(m_window);
			}
		}
		else
		{
			_swapChain->swapBuffers();
		}
	}

	void GlContext::makeCurrent(SwapChainGL* _swapChain)
	{
		if (m_current != _swapChain)
		{
			m_current = _swapChain;

			if (NULL == _swapChain)
			{
				if (NULL != m_window)
				{
					SDL_GL_MakeCurrent(m_window, m_context);
				}
			}
			else
			{
				_swapChain->makeCurrent();
			}
		}
	}

	void GlContext::import()
	{
		BX_TRACE("Import:");

#	if BX_PLATFORM_WINDOWS || BX_PLATFORM_LINUX
		void* glesv2 = bx::dlopen("libGLESv2." BX_DL_EXT);

#		define GL_EXTENSION(_optional, _proto, _func, _import)                           \
			{                                                                            \
				if (NULL == _func)                                                       \
				{                                                                        \
					_func = bx::dlsym<_proto>(glesv2, #_import);                         \
					BX_TRACE("\t%p " #_func " (" #_import ")", _func);                   \
					BGFX_FATAL(_optional || NULL != _func                                \
						, Fatal::UnableToInitialize                                      \
						, "Failed to create OpenGLES context. eglGetProcAddress(\"%s\")" \
						, #_import);                                                     \
				}                                                                        \
			}
#	else
#		define GL_EXTENSION(_optional, _proto, _func, _import)                           \
			{                                                                            \
				if (NULL == _func)                                                       \
				{                                                                        \
					_func = reinterpret_cast<_proto>(SDL_GL_GetProcAddress(#_import) );      \
					BX_TRACE("\t%p " #_func " (" #_import ")", _func);                   \
					BGFX_FATAL(_optional || NULL != _func                                \
						, Fatal::UnableToInitialize                                      \
						, "Failed to create OpenGLES context. SDL2_GL_GetProcAddress(\"%s\")" \
						, #_import);                                                     \
				}                                                                        \
			}

#	endif // BX_PLATFORM_

#	include "glimports.h"

#	undef GL_EXTENSION
	}

} /* namespace gl */ } // namespace bgfx

#	endif // BGFX_USE_SDL2
#endif // (BGFX_CONFIG_RENDERER_OPENGLES || BGFX_CONFIG_RENDERER_OPENGL)
