/*
 *    rendertarget.h    --    header for declaring render targets
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 *
 *    Included is functionality for creating render targets.
 *    Render targets are individual surfaces that can be rendered to.
 */
#pragma once

#include "libchik.h"

#include "camera.h"
#include "image.h"

typedef struct {
    image_t *target;
} rendertarget_t;

/*
 *    Creates a render target.
 *
 *    @param unsigned int width           The width of the render target.
 *    @param unsigned int height          The height of the render target.
 *    @param unsigned int fmt             The format of the render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_create(unsigned int width, unsigned int height, unsigned int fmt);

/*
 *    Frees a render target.
 *
 *    @param rendertarget_t *render_target    The render target to free.
 */
void rendertarget_free(rendertarget_t *render_target);

/*
 *    Gets a list of render targets.
 *
 *    @return rendertarget_t **    The list of render targets.
 *                                 NULL if the list could not be created.
 */
rendertarget_t **rendertarget_get_list(void);

/*
 *    Create backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_create_backbuffer(void);

/*
 *    Gets the backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_get_backbuffer(void);

/*
 *    Frees all render targets.
 */
void rendertarget_free_all(void);