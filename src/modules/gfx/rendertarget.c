/*
 *    rendertarget.c    --    source for using render targets
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 *
 *    Here are the functions used for creating and using render targets.
 */
#include "rendertarget.h"

#include "gfx.h"

extern vec2u_t (*platform_get_screen_size)(void);

rendertarget_t **_render_targets = NULL;

rendertarget_t *_back_buffer = NULL;

/*
 *    Creates a render target.
 *
 *    @param u32 width           The width of the render target.
 *    @param u32 height          The height of the render target.
 *    @param u32 fmt             The format of the render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_create(u32 width, u32 height, u32 fmt) {
    unsigned long   i;
    image_t        *image;
    rendertarget_t *render_target = malloc(sizeof(rendertarget_t));

    if (render_target == NULL) {
        LOGF_ERR("Could not allocate memory for render target.\n");
        return 0;
    }

    image = image_create(width, height, fmt);
    if (image == NULL) {
        LOGF_ERR("Could not allocate memory for render target image.\n");
        return 0;
    }

    render_target->target = image;

    /*
     *    Make a new list of render targets if there is none.
     */
    if (_render_targets == NULL) {
        _render_targets = malloc(sizeof(rendertarget_t *) * 2);

        if (_render_targets == NULL) {
            LOGF_ERR("Could not allocate memory for render target list.");
            return 0;
        }

        _render_targets[0] = render_target;
        _render_targets[1] = NULL;
    } else {
        i = 0;

        while (_render_targets[i] != NULL)
            i++;

        _render_targets[i] = render_target;
        _render_targets =
            realloc(_render_targets, sizeof(rendertarget_t *) * (i + 1));

        if (_render_targets == NULL) {
            LOGF_ERR("Could not reallocate memory for render target list.");
            return 0;
        }

        _render_targets[i + 1] = NULL;
    }

    return render_target;
}

/*
 *    Frees a render target.
 *
 *    @param rendertarget_t *render_target    The render target to free.
 */
void rendertarget_free(rendertarget_t *render_target) {
    if (render_target == NULL) {
        LOGF_ERR("Render target is NULL.\n");
        return;
    }

    image_free(render_target->target);
    free(render_target);
}

/*
 *    Gets a list of render targets.
 *
 *    @return rendertarget_t **    The list of render targets.
 *                                 NULL if the list could not be created.
 */
rendertarget_t **rendertarget_get_list(void) { return _render_targets; }

/*
 *    Create backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_create_backbuffer(void) {
    vec2u_t         res           = platform_get_screen_size();
    rendertarget_t *render_target = rendertarget_create(res.x, res.y, 69);

    if (render_target == NULL) {
        LOGF_ERR("Could not create backbuffer render target.");
        return NULL;
    }

    _back_buffer = render_target;

    return render_target;
}

/*
 *    Gets the backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be
 * created. The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_get_backbuffer(void) { return _back_buffer; }

/*
 *    Frees all render targets.
 */
void rendertarget_free_all(void) {
    unsigned long i;

    if (_render_targets == NULL)
        return;

    i = 0;
    while (_render_targets[i] != NULL) {
        rendertarget_free(_render_targets[i]);
        i++;
    }

    free(_render_targets);
    _render_targets = NULL;
}