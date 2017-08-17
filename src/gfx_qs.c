#include <stdlib.h>
#include <SDL2/SDL.h>
#include "bstrlib.h"

#include "core.h"
#include "qrs.h"
#include "qrs.h"
#include "timer.h"
#include "gfx.h"
#include "game_qs.h"
#include "gfx_qs.h"

int piece_colors[26] =
{
    0xD00000,
    0x0000FF,
    0xFF6000,
    0x101010,
    0xBB3CBB,
    0x00CD00,
    0x70ECEE,   // ice
    0xFF658B,
    0xD22D04,
    0x1200BF,   // dark blue
    0xC70694,
    0x1200BF,   // dark blue
    0xEFEF00,
    0xC70694,
    0xD22D04,
    0x70ECEE,   // ice
    0xFF658B,
    0x00A3A3,   // teal
    0xD00000,
    0x00A3A3,   // teal
    0x0000FF,
    0xFF6000,
    0xEFEF00,
    0xBB3CBB,
    0x00CD00,
    0x808080
};

int gfx_drawqs(game_t *g)
{
	if(!g)
		return -1;
    if(!g->origin)
        return -1;

    coreState *cs = g->origin;
	qrsdata *q = (qrsdata *)(g->data);

	piecedef *pd_current = q->p1->def;

    unsigned int drawpiece_next1_flags = DRAWPIECE_PREVIEW;
    if(q->previews[0]) {
        if(q->previews[0]->qrs_id % 18 == 0)
            drawpiece_next1_flags |= DRAWPIECE_IPREVIEW;
    }

    unsigned int drawqrsfield_flags = 0;
    unsigned int drawpiece_flags = /*q->mode_type == MODE_G2_DEATH ? GFX_G2 : */0;

    SDL_Texture *font = (asset_by_name(cs, "font"))->data;
    SDL_Texture *labg = (asset_by_name(cs, "level_alphabg"))->data;
    SDL_Texture *tets_dark_qs = (asset_by_name(cs, "tets_dark_qs"))->data;

    SDL_Rect palettesrc = {.x = 0, .y = 0, .w = 16, .h = 16};
    SDL_Rect palettedest = {.x = FIELD_EDITOR_PALETTE_X, .y = FIELD_EDITOR_PALETTE_Y, .w = 16, .h = 16};

    SDL_Rect src = {.x = 0, .y = 0, .w = 32, .h = 32};
    SDL_Rect dest = {.x = 0, .y = 0, .w = 32, .h = 32};

    float lt = q->p1->speeds->lock;
    float l = q->p1counters->lock;
    char r = 255 - (char)(80*l/lt);
    Uint32 rgba = (r * 0x1000000) + (r * 0x10000) + (r * 0x100) + 0xFF;

    int i = 0;
	int x = q->field_x;
	int y = q->field_y;
	int piece_x = x + (16 * q->p1->x);
	int piece_y = y + (16 * YTOROW(q->p1->y)) - 16;

    SDL_Rect labg_dest = {.x = 264 - 48 + 4 + x, .y = 312 - 32 + y, .w = 95, .h = 64};

    int preview1_x = x + 5*16;
    int preview2_x = q->tetromino_only ? x + 20*8 : x + 21*8;
    int preview3_x = q->tetromino_only ? x + 24*8 + 12 : x + 27*8;
    int preview1_y = y - 3*16 - (q->game_type && !q->pracdata ? 4 : 0);
    int preview2_y = y - 2*8 - (q->game_type && !q->pracdata ? 4 : 0);
    int preview3_y = y - 2*8 - (q->game_type && !q->pracdata ? 4 : 0);
    int hold_x = preview1_x - 8*6;
    int hold_y = preview2_y - 12;

    int y_bkp = 0;
    int s_bkp = 0;

    double mspf = 1000.0 * (1.0/cs->fps);
    int cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent) / mspf));

	bstring text_level = bfromcstr("LEVEL");
	bstring level = bformat("%d", q->level);
    bstring next = bfromcstr("NEXT");
    bstring next_name = NULL;
    if(q->previews[0]) {
        next_name = bfromcstr(get_qrspiece_name(q->previews[0]->qrs_id));
    }

    bstring grade_text = bfromcstr(get_grade_name(q->grade));
    bstring score_text = bformat("%d", q->score);

    bstring undo = bfromcstr("UNDO");
    bstring redo = bfromcstr("REDO");
    //bstring columns = bfromcstr("0123456789AB");
    //bstring columns_adj = malloc(sizeof(bstring));
    bstring avg_sleep_ms = bformat("%LF", cs->avg_sleep_ms_recent);
    bstring ctp_bstr = bformat("%d%%", cpu_time_percentage);
    bstring ctp_overload_bstr = NULL;

    bstring undo_len = NULL;
    bstring redo_len = NULL;

    struct text_formatting fmt = {
        .rgba = RGBA_DEFAULT,
        .outline_rgba = RGBA_OUTLINE_DEFAULT,
        .outlined = true,
        .shadow = false,
        .line_spacing = 1.0,
        .align = ALIGN_LEFT,
        .wrap_length = 0
    };

    //columns_adj->data = columns->data + (QRS_FIELD_W - q->field_w)/2;
    //columns_adj->slen = q->field_w;

    if(q->state_flags & GAMESTATE_FADING)
        drawqrsfield_flags |= DRAWFIELD_NO_OUTLINE;

    if(q->state_flags & GAMESTATE_INVISIBLE)
        drawqrsfield_flags |= DRAWFIELD_INVISIBLE;

    if(q->pracdata) {
        if(q->pracdata->paused == QRS_FIELD_EDIT) {
            gfx_drawqrsfield(cs, q->pracdata->usr_field, MODE_UNSPECIFIED, DRAWFIELD_GRID | DRAWFIELD_NO_OUTLINE, x, y);
            if(q->pracdata->field_selection)
                gfx_drawfield_selection(g, q->pracdata);

            if(q->pracdata->usr_field_undo_len) {
                undo_len = bformat("%d", q->pracdata->usr_field_undo_len);

                gfx_drawtext(cs, undo, QRS_FIELD_X + 16, QRS_FIELD_Y + 23*16, monofont_square, NULL);
                gfx_drawtext(cs, undo_len, QRS_FIELD_X + 16, QRS_FIELD_Y + 24*16, monofont_square, NULL);

                src.x = 14*16;
                src.y = 64;
                dest.x = QRS_FIELD_X - 16;
                dest.y = QRS_FIELD_Y + 23*16;

                gfx_rendercopy(cs, font, &src, &dest);

                bdestroy(undo_len);
            }

            if(q->pracdata->usr_field_redo_len) {
                redo_len = bformat("%d", q->pracdata->usr_field_redo_len);

                gfx_drawtext(cs, redo, QRS_FIELD_X + 9*16, QRS_FIELD_Y + 23*16, monofont_square, NULL);
                gfx_drawtext(cs, redo_len, QRS_FIELD_X + 13*16 - 16*(redo_len->slen), QRS_FIELD_Y + 24*16, monofont_square, NULL);

                src.x = 16*16;
                src.y = 64;
                dest.x = QRS_FIELD_X + 13*16;
                dest.y = QRS_FIELD_Y + 23*16;

                gfx_rendercopy(cs, font, &src, &dest);

                bdestroy(redo_len);
            }

            if(q->pracdata->usr_seq_len) {
                if(qs_get_usrseq_elem(q->pracdata, 0) == QRS_I4 || qs_get_usrseq_elem(q->pracdata, 0) == QRS_I)
                    drawpiece_next1_flags = DRAWPIECE_PREVIEW|DRAWPIECE_IPREVIEW;
                else
                    drawpiece_next1_flags = DRAWPIECE_PREVIEW;

                //gfx_drawtext(cs, next, 48 - 32 + QRS_FIELD_X, 26, 0, 0xFFFFFF8C, 0x0000008C);

                if(q->num_previews > 0)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[0], drawpiece_next1_flags, FLAT, preview1_x, preview1_y, RGBA_DEFAULT);
                if(q->num_previews > 1)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[1], DRAWPIECE_PREVIEW|DRAWPIECE_SMALL, FLAT, preview2_x, preview2_y, RGBA_DEFAULT);
                if(q->num_previews > 2)
                    gfx_drawpiece(cs, g->field, x, y, q->previews[2], DRAWPIECE_PREVIEW|DRAWPIECE_SMALL, FLAT, preview3_x, preview3_y, RGBA_DEFAULT);
            }

            for(i = 0; i < 18; i++) {
                if(i < 3 || i == 4 || i == 5 || i > 10)
                    continue;

                palettesrc.x = i*16;
                gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                if(q->pracdata->palette_selection - 1 == i) {
                    palettesrc.x = 31*16;
                    SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                    gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                    SDL_SetTextureAlphaMod(tets_dark_qs, 255);
                }
                palettedest.y += 16;
            }

            palettedest.y += 16;

            for(i = 18; i < 26; i++) {
                palettesrc.x = i*16;
                gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                if(q->pracdata->palette_selection - 1 == i || (i == 25 && q->pracdata->palette_selection == -5)) {
                    palettesrc.x = 31*16;
                    SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                    gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                    SDL_SetTextureAlphaMod(tets_dark_qs, 255);
                }
                palettedest.y += 16;
            }

            palettesrc.x = 30*16;
            gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
            if(q->pracdata->palette_selection == QRS_PIECE_BRACKETS) {
                palettesrc.x = 31*16;
                SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                SDL_SetTextureAlphaMod(tets_dark_qs, 255);
            }

            palettedest.y += 16;
            palettesrc.x = 32*16;
            gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
            if(q->pracdata->palette_selection == QRS_PIECE_GEM) {
                palettesrc.x = 31*16;
                SDL_SetTextureAlphaMod(tets_dark_qs, 140);
                gfx_rendercopy(cs, tets_dark_qs, &palettesrc, &palettedest);
                SDL_SetTextureAlphaMod(tets_dark_qs, 255);
            }
        } else {
            if(q->pracdata->invisible)
                drawqrsfield_flags |= DRAWFIELD_INVISIBLE;

            gfx_drawqrsfield(cs, g->field, MODE_UNSPECIFIED, drawqrsfield_flags, x, y);
            gfx_drawtimer(cs, q->timer, x + 32, RGBA_DEFAULT);
            gfx_drawkeys(cs, cs->keys[0], 22*16, 27*16, RGBA_DEFAULT);

            goto active_game_drawing;
        }
    } else {
        if(q->mode_type != MODE_UNSPECIFIED)
            gfx_drawqrsfield(cs, g->field, q->mode_type, drawqrsfield_flags|TEN_W_TETRION, x, y);
        else
            gfx_drawqrsfield(cs, g->field, q->mode_type, drawqrsfield_flags, x, y);

        if(q->p1->speeds->grav >= 20*256) {
            if(cs->frames % 4 < 2)
                gfx_drawtimer(cs, q->timer, x + 32, RGBA_DEFAULT);
            else
                gfx_drawtimer(cs, q->timer, x + 32, 0xEFEF50FF);
        } else
            gfx_drawtimer(cs, q->timer, x + 32, RGBA_DEFAULT);

        gfx_drawkeys(cs, cs->keys[0], 22*16, 27*16, RGBA_DEFAULT);

        // uncomment this for DDR-esque input display :3
        if(q->playback) {
            for(i = 1; i < 24 && i < (q->replay->len - q->playback_index)/3; i++) {
                gfx_drawkeys(cs, &q->replay->inputs[3*i+q->playback_index], 22*16, (25-i)*16, RGBA_DEFAULT);
            }
        }

        gfx_rendercopy(cs, labg, NULL, &labg_dest);
        labg_dest.y -= 5*16;
        gfx_rendercopy(cs, labg, NULL, &labg_dest);

        if(q->p1->speeds->grav >= 20*256) {
            if(cs->frames % 4 < 2) {
                gfx_drawtext(cs, text_level, x + 14*16 + 4, y + 18*16, monofont_square, NULL);
                gfx_drawtext(cs, level, x + 14*16 + 4, y + 20*16, monofont_square, NULL);
            } else {
                fmt.rgba = 0xEFEF50FF;
                gfx_drawtext(cs, text_level, x + 14*16 + 4, y + 18*16, monofont_square, &fmt);
                gfx_drawtext(cs, level, x + 14*16 + 4, y + 20*16, monofont_square, &fmt);
            }
        } else {
            gfx_drawtext(cs, text_level, x + 14*16 + 4, y + 18*16, monofont_square, NULL);
            gfx_drawtext(cs, level, x + 14*16 + 4, y + 20*16, monofont_square, NULL);
        }

        if((q->grade & 0xff) != NO_GRADE) {
            gfx_drawtext(cs, grade_text, x + 15*16, y + 4*16, monofont_square, NULL);
        }

        fmt.rgba = 0xFFFF40FF;
        gfx_drawtext(cs, bfromcstr("SCORE"), x + 14*16+ 4, y + 13*16, monofont_square, &fmt);
        gfx_drawtext(cs, score_text, x + 14*16 + 4, y + 15*16, monofont_square, &fmt);

active_game_drawing:
        if(q->mode_type == MODE_UNSPECIFIED) {
            gfx_drawtext(cs, next, 50 - 32 + QRS_FIELD_X, 28, monofont_small, NULL);
            gfx_drawtext(cs, next_name, 50 - 32 + QRS_FIELD_X, 40, monofont_small, NULL);
        }

        if(q->num_previews > 0)
            gfx_drawpiece(cs, g->field, x, y, q->previews[0], drawpiece_flags|drawpiece_next1_flags, FLAT, preview1_x, preview1_y, RGBA_DEFAULT);
        if(q->num_previews > 1)
            gfx_drawpiece(cs, g->field, x, y, q->previews[1], drawpiece_flags|DRAWPIECE_PREVIEW|DRAWPIECE_SMALL, FLAT, preview2_x, preview2_y, RGBA_DEFAULT);
        if(q->num_previews > 2)
            gfx_drawpiece(cs, g->field, x, y, q->previews[2], drawpiece_flags|DRAWPIECE_PREVIEW|DRAWPIECE_SMALL, FLAT, preview3_x, preview3_y, RGBA_DEFAULT);

        if(q->hold)
            gfx_drawpiece(cs, g->field, x, y, q->hold, drawpiece_flags|DRAWPIECE_PREVIEW|DRAWPIECE_SMALL|(q->p1->state & PSUSEDHOLD ? DRAWPIECE_LOCKFLASH : 0), FLAT, hold_x, hold_y, RGBA_DEFAULT);

        if((q->p1->state & (PSFALL | PSLOCK)) && !(q->p1->state & PSPRELOCKED)) {
            if(!(pd_current->flags & PDBRACKETS)) {
                y_bkp = q->p1->y;
                s_bkp = q->p1->state;
                qrs_fall(g, q->p1, 28*256);
                piece_y = y + (16 * YTOROW(q->p1->y)) - 16;

                switch(q->mode_type) {
                    case MODE_UNSPECIFIED:
                        if(q->level < 400)
                            gfx_drawpiece(cs, g->field, x, y, pd_current, 0, q->p1->orient, piece_x, piece_y, 0xFFFFFF60);

                        break;

                    default:
                        if(q->level < 100)
                            gfx_drawpiece(cs, g->field, x, y, pd_current, 0, q->p1->orient, piece_x, piece_y, 0xFFFFFF60);

                        break;
                }

                q->p1->y = y_bkp;
                q->p1->state = s_bkp;
                piece_y = y + (16 * YTOROW(q->p1->y)) - 16;
            }

            if(pd_current->flags & PDBRACKETS)
                rgba = RGBA_DEFAULT;

            gfx_drawpiece(cs, g->field, x, y, pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, rgba);
        } else if(q->p1->state & (PSLOCKFLASH1 | PSLOCKFLASH2) && !(q->state_flags & GAMESTATE_BRACKETS))
            gfx_drawpiece(cs, g->field, x, y, pd_current, drawpiece_flags|DRAWPIECE_LOCKFLASH, q->p1->orient, piece_x, piece_y, RGBA_DEFAULT);
        else if(q->p1->state & PSPRELOCKED) {
            if(q->state_flags & GAMESTATE_BRACKETS)
                gfx_drawpiece(cs, g->field, x, y, pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, RGBA_DEFAULT);
            else
                gfx_drawpiece(cs, g->field, x, y, pd_current, drawpiece_flags, q->p1->orient, piece_x, piece_y, 0x404040FF);
        }
    }

    //gfx_drawtime(cs, 123456, 500, 400, RGBA_DEFAULT);

    /*if(q->pracdata) {
        if(q->pracdata->paused == QRS_FIELD_EDIT)
            gfx_drawtext(cs, columns_adj, QRS_FIELD_X + 16 + 8*(QRS_FIELD_W - q->field_w), QRS_FIELD_Y + 16, 0, 0xFFFFFFB0, 0x000000B0);
    }*/

    gfx_drawqsmedals(g);

    fmt.shadow = true;
    fmt.outlined = false;
    fmt.rgba = 0x7070D0FF;
    gfx_drawtext(cs, ctp_bstr, 640 - 16 + 16 * (1 - ctp_bstr->slen), 2, monofont_square, &fmt);

    if(cs->recent_frame_overload >= 0) {
        cpu_time_percentage = (int)(100.0 * ((mspf - cs->avg_sleep_ms_recent_array[cs->recent_frame_overload]) / mspf));
        ctp_overload_bstr = bformat("%d%%", cpu_time_percentage);

        fmt.rgba = 0xB00000FF;
        gfx_drawtext(cs, ctp_overload_bstr, 640 - 16 + 16 * (1 - ctp_overload_bstr->slen), 18, monofont_square, &fmt);

        bdestroy(ctp_overload_bstr);
    }

	bdestroy(text_level);
	bdestroy(level);
    bdestroy(next);
    bdestroy(next_name);
    bdestroy(grade_text);

    bdestroy(undo);
    bdestroy(redo);
    //bdestroy(columns);
    bdestroy(avg_sleep_ms);
    bdestroy(ctp_bstr);

	return 0;
}

int gfx_qs_lineclear(game_t *g, int row)
{
    qrsdata *q = g->data;
    if(q->pracdata) {
        if(q->pracdata->brackets)
            return 0;
    }

    int i = 0;
    int mod = 0;
    int c = 0;
    bstring name = bfromcstr("animation/lineclear");

    for(i = (QRS_FIELD_W - q->field_w)/2; i < (QRS_FIELD_W + q->field_w)/2; i+=2) {
        c = gridgetcell(g->field, i, row);
        if(c & QRS_PIECE_BRACKETS)
            continue;
        if(c > 0)
            mod = piece_colors[(c & 0xFF) - 1] * 0x100 + 0xFF;
        else
            mod = piece_colors[25] * 0x100 + 0xFF;

        if(row % 2)
            gfx_pushanimation(g->origin, name, q->field_x + (i * 16), (3 + row) * 16 - QRS_FIELD_Y + q->field_y, 5, 3, mod);
        else
            gfx_pushanimation(g->origin, name, q->field_x + (i * 16) + 16, (3 + row) * 16 - QRS_FIELD_Y + q->field_y, 5, 3, mod);
    }

    return 0;
}

int gfx_drawqsmedals(game_t *g)
{
    if(!g)
        return -1;

    qrsdata *q = g->data;
    SDL_Rect dest = {.x = 270 - 32 + QRS_FIELD_X, .y = 158, .w = 40, .h = 20};
    SDL_Texture *medal = NULL;

    switch(q->medal_re) {
        case BRONZE:
            medal = (asset_by_name(g->origin, "medals/bronzeRE"))->data;
            break;
        case SILVER:
            medal = (asset_by_name(g->origin, "medals/silverRE"))->data;
            break;
        case GOLD:
            medal = (asset_by_name(g->origin, "medals/goldRE"))->data;
            break;
        case PLATINUM:
            medal = (asset_by_name(g->origin, "medals/platinumRE"))->data;
            break;
        default:
            medal = NULL;
            break;
    }

    if(medal)
        gfx_rendercopy(g->origin, medal, NULL, &dest);

    dest.y += 24;

    switch(q->medal_sk) {
        case BRONZE:
            medal = (asset_by_name(g->origin, "medals/bronzeSK"))->data;
            break;
        case SILVER:
            medal = (asset_by_name(g->origin, "medals/silverSK"))->data;
            break;
        case GOLD:
            medal = (asset_by_name(g->origin, "medals/goldSK"))->data;
            break;
        case PLATINUM:
            medal = (asset_by_name(g->origin, "medals/platinumSK"))->data;
            break;
        default:
            medal = NULL;
            break;
    }

    if(medal)
        gfx_rendercopy(g->origin, medal, NULL, &dest);

    dest.y += 24;

    switch(q->medal_co) {
        case BRONZE:
            medal = (asset_by_name(g->origin, "medals/bronzeCO"))->data;
            break;
        case SILVER:
            medal = (asset_by_name(g->origin, "medals/silverCO"))->data;
            break;
        case GOLD:
            medal = (asset_by_name(g->origin, "medals/goldCO"))->data;
            break;
        case PLATINUM:
            medal = (asset_by_name(g->origin, "medals/platinumCO"))->data;
            break;
        default:
            medal = NULL;
            break;
    }

    if(medal)
        gfx_rendercopy(g->origin, medal, NULL, &dest);

    dest.y += 24;

    switch(q->medal_st) {
        case BRONZE:
            medal = (asset_by_name(g->origin, "medals/bronzeST"))->data;
            break;
        case SILVER:
            medal = (asset_by_name(g->origin, "medals/silverST"))->data;
            break;
        case GOLD:
            medal = (asset_by_name(g->origin, "medals/goldST"))->data;
            break;
        case PLATINUM:
            medal = (asset_by_name(g->origin, "medals/platinumST"))->data;
            break;
        default:
            medal = NULL;
            break;
    }

    if(medal)
        gfx_rendercopy(g->origin, medal, NULL, &dest);

    return 0;
}

int gfx_drawfield_selection(game_t *g, struct pracdata *d)
{
    SDL_Texture *tets = (asset_by_name(g->origin, "tets_bright_qs"))->data;
    SDL_Rect src = {.x = 31 * 16, .y = 0, .w = 16, .h = 16};
    SDL_Rect dest = {.x = 0, .y = 0, .w = 16, .h = 16};

    int i = 0;
    int j = 0;

    int lesser_x = 0;
    int greater_x = 0;
    int lesser_y = 0;
    int greater_y = 0;

    if(d->field_selection_vertex1_x <= d->field_selection_vertex2_x) {
        lesser_x = d->field_selection_vertex1_x;
        greater_x = d->field_selection_vertex2_x;
    } else {
        lesser_x = d->field_selection_vertex2_x;
        greater_x = d->field_selection_vertex1_x;
    }

    if(d->field_selection_vertex1_y <= d->field_selection_vertex2_y) {
        lesser_y = d->field_selection_vertex1_y;
        greater_y = d->field_selection_vertex2_y;
    } else {
        lesser_y = d->field_selection_vertex2_y;
        greater_y = d->field_selection_vertex1_y;
    }

    SDL_SetTextureColorMod(tets, 170, 170, 255);
    SDL_SetTextureAlphaMod(tets, 160);

    for(i = lesser_x; i <= greater_x; i++) {
        for(j = lesser_y; j <= greater_y; j++) {
            if(i >= 0 && i < 12 && j >= 0 && j < 20) {
                if(gridgetcell(d->usr_field, i, j + 2) != QRS_FIELD_W_LIMITER) {
                    dest.x = QRS_FIELD_X + 16*(i + 1);
                    dest.y = QRS_FIELD_Y + 16*(j + 2);

                    gfx_rendercopy(g->origin, tets, &src, &dest);
                }
            }
        }
    }

    SDL_SetTextureColorMod(tets, 255, 255, 255);
    SDL_SetTextureAlphaMod(tets, 255);

    return 0;
}
