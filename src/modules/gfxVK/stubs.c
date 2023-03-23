#include "libchik.h"

void image_create_from_file(const s8 *path, image_t *img) {

}

void text_create(void) {

}

void *vbuffer_create(void *v, u32 size, u32 stride, v_layout_t layout) {

}

void vbuffer_free(void *buf) {

}

void *mesh_create(void *v) {

}

void mesh_set_vbuffer(void *m, void *v) {

}

void mesh_append_asset(void *m, void *a, unsigned long size) {

}

void mesh_set_asset(void *m, void *a, unsigned long size, unsigned long index) {

}

void *mesh_get_asset(void *m, unsigned long index) {

}

void mesh_draw(void *m) {

}

void mesh_free(void *m) {

}

mat4_t get_camera_view(trap_t sCamera) {

}

void draw_frame(void) {

}

trap_t create_camera(void) {

}

void set_camera_position(trap_t sCamera, vec3_t sPosition) {

}

void set_camera_direction(trap_t sCamera, vec2_t sDirection) {

}

void set_camera_fov(trap_t sCamera, float sFov) {

}

void set_camera(trap_t sCamera) {

}

vec2_t get_screen_size(void) {
    
}