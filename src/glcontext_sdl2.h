/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#ifndef BGFX_GLCONTEXT_SDL2_H_HEADER_GUARD
#define BGFX_GLCONTEXT_SDL2_H_HEADER_GUARD

#if BGFX_USE_SDL2

#include <SDL2/sdl.h>

#if defined(Success)
// X11 defines Success
#	undef Success
#endif // defined(Success)

namespace bgfx { namespace gl
{
	struct SwapChainGL;

	struct GlContext
	{
		GlContext()
			: m_current(NULL)
			, m_context(NULL)
			, m_window(NULL)
		{
		}

		void create(uint32_t _width, uint32_t _height);
		void destroy();
		void resize(uint32_t _width, uint32_t _height, uint32_t _flags);

		uint64_t getCaps() const;
		SwapChainGL* createSwapChain(void* _nwh);
		void destroySwapChain(SwapChainGL*  _swapChain);
		void swap(SwapChainGL* _swapChain = NULL);
		void makeCurrent(SwapChainGL* _swapChain = NULL);

		void import();

		bool isValid() const
		{
			return NULL != m_context;
		}

		SwapChainGL* m_current;
		SDL_GLContext m_context;
		SDL_Window  m_window;
	};
} /* namespace gl */ } // namespace bgfx

#endif // BGFX_USE_SDL2

#endif // BGFX_GLCONTEXT_SDL2_H_HEADER_GUARD
