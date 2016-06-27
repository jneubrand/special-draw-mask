#include <pebble.h>
#include <special-draw/special-draw.h>
#include "special-draw-mask.h"

#if defined(PBL_RECT) && !defined(SPECIAL_DRAW_SCREEN_SIZE)
#define SPECIAL_DRAW_SCREEN_SIZE (GSize(144, 168))
#elif defined(PBL_ROUND) && !defined(SPECIAL_DRAW_SCREEN_SIZE)
#define SPECIAL_DRAW_SCREEN_SIZE (GSize(180, 180))
#endif


static void prv_run_modifier(GSpecialSessionModifier * modifier,
        GBitmap * bitmap) {
    GRect bounds = gbitmap_get_bounds(bitmap);
    for (int y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; y++) {
        GBitmapDataRowInfo row_info =
            gbitmap_get_data_row_info(bitmap, y);
        GBitmapDataRowInfo row_info_mask =
            gbitmap_get_data_row_info(
                ((GSpecialSessionMaskModifierData *)modifier->context)->mask,
                y);
        for (int x = row_info.min_x; x <= row_info.max_x; x++) {
            GColor * pixel = (GColor *) &row_info.data[x];
            pixel->a = (row_info_mask.data[x/4] >>
                ((2*x) % 8)) & 0b11;
        }
    }
}

static void prv_destroy_modifier(GSpecialSessionModifier * modifier) {
    gbitmap_destroy(((GSpecialSessionMaskModifierData *)modifier->context)->mask);
    free(modifier->context);
    free(modifier);
}

GSpecialSessionModifier * graphics_special_draw_create_mask_modifier(
        GSpecialSession * session) {
    // Create the modifier.
    GSpecialSessionModifier * mod = malloc(sizeof(GSpecialSessionModifier));
    mod->overrides_draw = false;
    mod->action.modifier_run = prv_run_modifier;
    mod->destroy = prv_destroy_modifier;
    mod->context = malloc(sizeof(GSpecialSessionMaskModifierData));
    return mod;
}

void graphics_special_draw_mask_modifier_update(
        GSpecialSession * session, GSpecialSessionModifier * modifier,
        bool use_true_background) {
    GSpecialSessionMaskModifierData * data = modifier->context;
    data->mask = gbitmap_create_blank(
        gbitmap_get_bounds(session->new_fbuf).size,
        GBitmapFormat2BitPalette);
    GBitmap * fbuf = session->new_fbuf;
    GRect bounds = gbitmap_get_bounds(fbuf);
    GBitmap * temp_bitmap = gbitmap_create_with_data(session->initial_data);
    gbitmap_set_data(temp_bitmap, session->initial_data,
            session->old_format, session->old_row_size, false);
    for (int y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; y++) {
        GBitmapDataRowInfo row_info =
            gbitmap_get_data_row_info(fbuf, y);
        GBitmapDataRowInfo row_info_mask =
            gbitmap_get_data_row_info(data->mask, y);
        GBitmapDataRowInfo row_info_orig =
            gbitmap_get_data_row_info(temp_bitmap, y);
        for (int x = row_info.min_x; x <= row_info.max_x; x++) {
            GColor * pixel = (GColor *) &row_info.data[x];
            GColor * pixel_orig = (GColor *) &row_info.data[x];
            row_info_mask.data[x/4] |= pixel->a << ((2*x) %8);
            pixel->argb = use_true_background ? pixel_orig->argb : 0;
        }
    }
    graphics_draw_bitmap_in_rect(session->ctx, temp_bitmap,
        (GRect) {GPoint(0, 0), SPECIAL_DRAW_SCREEN_SIZE});
    gbitmap_destroy(temp_bitmap);
}
