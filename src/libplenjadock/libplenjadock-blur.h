#include <gtk/gtk.h>
#include <math.h>

void blur_image_surface (cairo_surface_t *surface, int radius);
void stack_blur (cairo_surface_t *input, cairo_surface_t *output, int rx, int ry);
