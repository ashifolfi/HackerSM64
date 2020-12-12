#include <ultra64.h>
#include "config.h"
#include "mtx.h"

int myScale = 1;
int myDegrees = 0;
uObjMtx final_mtx, rot_mtx;

#define TEX_X -8
#define TEX_Y -8

static Gfx s2d_init_dl[] = {
	gsDPPipeSync(),
	gsDPSetTexturePersp(G_TP_NONE),
	gsDPSetTextureLOD(G_TL_TILE),
	gsDPSetTextureLUT(G_TT_NONE),
	gsDPSetTextureConvert(G_TC_FILT),
	gsDPSetAlphaCompare(G_AC_THRESHOLD),
	gsDPSetBlendColor(0, 0, 0, 0x01),
	gsDPSetCombineMode(G_CC_DECALRGBA, G_CC_DECALRGBA),
	gsSPEndDisplayList(),
};

void setup_font(int idx) {
	gDPPipeSync(gDisplayListHead++);
	gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
	gSPDisplayList(gDisplayListHead++, s2d_init_dl);
	gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
	gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
	gSPObjRenderMode(gDisplayListHead++, G_OBJRM_XLU | G_OBJRM_BILERP);
	gSPObjLoadTxtr(gDisplayListHead++, &s2d_tex[idx]);
}

extern void mat2_translate_vec(uObjMtx *m, f32 degrees, f32 mag);

// Original Mtx Pipeline
// Doesnt work with rotation, but is faster
void mtx_pipeline(uObjMtx *m, int x, int y) {
	// init
	mat2_ident(m, 1);
	mat2_ident(&rot_mtx, 1);

	// create rot matrix
	mat2_rotate(&rot_mtx, (myDegrees) * (M_PI / 180.0f));


	// scale m
	mat2_scale(m, myScale);
	// mat2_copy(m, &rot_mtx);
	mat2_dst_mul(m,m,  &rot_mtx);
	mat2_translate(m, x, y);

	// yeah
	gSPObjMatrix(gDisplayListHead++, m);
}

#include <PR/gu.h>
typedef float Mat4[4][4];

// New matrix pipeline
// Works with both rotation and scale,
// but is slow due to more float operations being performed
void mtx_pipeline2(uObjMtx *m, int x, int y) {
	// init
	Mat4 tmp, rot, scal, translate;
	guMtxIdentF(tmp);
	guScaleF(scal, myScale, myScale, 0);
	guRotateF(rot, (f32) myDegrees, 0, 0, 1.0f);
	guTranslateF(translate, x, y, 0);

	mtxf_mul(tmp, tmp, scal);
	mtxf_mul(tmp, tmp, rot);
	mtxf_mul(tmp, tmp, translate);

	gu_to_gs2dex(m, tmp);

    if (myDegrees != 0) {
        mat2_translate_vec(m, -(myDegrees) * M_DTOR, myScale);
    }

	// yeah
	gSPObjMatrix(gDisplayListHead++, m);
}

void draw_s2d_glyph(char c, int x, int y, uObjMtx *mt) {
	setup_font(c);

	// mtx_pipeline(mt, x, y);
    mtx_pipeline2(mt, x, y);

	gSPObjSprite(gDisplayListHead++, &s2d_font);
}


