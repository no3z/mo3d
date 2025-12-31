#pragma once

// Cross-platform OpenGL header
// Handles differences between macOS and Linux OpenGL includes

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

// Common OpenGL error checking
#ifndef NDEBUG
    #define GL_CHECK(call) \
        do { \
            call; \
            GLenum err; \
            while ((err = glGetError()) != GL_NO_ERROR) { \
                fprintf(stderr, "OpenGL error 0x%x at %s:%d\n", err, __FILE__, __LINE__); \
            } \
        } while (0)
#else
    #define GL_CHECK(call) call
#endif
