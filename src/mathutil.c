#include "object.h"
#include "mathutil.h"

/* Overlay 6's maths: sines from a table, matrices and vectors. */

/* Sines and cosines of the first quarter turn, as (sin, cos) pairs: 257 of them. */
/* mathutil.c's .data (0x80107DD0-0x801085D8), in ROM order. */
f32 gMathSinTable[0x202] = { /* sin(i * 90 / 512 degrees), cosines interleaved: 0x202 entries */
    0.0f, 1.0f, 0.0061359f, 0.9999812f, 0.0122715f, 0.9999247f, 0.0184067f, 0.9998306f, 0.0245412f,
    0.9996988f, 0.0306748f, 0.9995294f, 0.0368072f, 0.9993224f, 0.0429383f, 0.9990777f, 0.0490677f,
    0.9987955f, 0.0551952f, 0.9984756f, 0.0613207f, 0.9981181f, 0.0674439f, 0.9977231f, 0.0735646f,
    0.9972905f, 0.0796824f, 0.9968203f, 0.0857973f, 0.9963126f, 0.091909f, 0.9957674f, 0.0980171f,
    0.9951847f, 0.1041216f, 0.9945646f, 0.1102222f, 0.993907f, 0.1163186f, 0.9932119f, 0.1224107f,
    0.9924795f, 0.1284981f, 0.9917098f, 0.1345807f, 0.9909026f, 0.1406582f, 0.9900582f, 0.1467305f,
    0.9891765f, 0.1527972f, 0.9882576f, 0.1588581f, 0.9873014f, 0.1649131f, 0.9863081f, 0.1709619f,
    0.9852776f, 0.1770042f, 0.9842101f, 0.1830399f, 0.9831055f, 0.1890687f, 0.9819639f, 0.1950903f,
    0.9807853f, 0.2011046f, 0.9795698f, 0.2071114f, 0.9783174f, 0.2131103f, 0.9770281f, 0.2191012f,
    0.9757021f, 0.2250839f, 0.9743394f, 0.2310581f, 0.97294f, 0.2370236f, 0.9715039f, 0.2429802f,
    0.9700313f, 0.2489276f, 0.9685221f, 0.2548657f, 0.9669765f, 0.2607941f, 0.9653944f, 0.2667128f,
    0.9637761f, 0.2726214f, 0.9621214f, 0.2785197f, 0.9604305f, 0.2844075f, 0.9587035f, 0.2902847f,
    0.9569403f, 0.2961509f, 0.9551412f, 0.3020059f, 0.953306f, 0.3078496f, 0.951435f, 0.3136817f,
    0.9495282f, 0.319502f, 0.9475856f, 0.3253103f, 0.9456073f, 0.3311063f, 0.9435935f, 0.3368899f,
    0.9415441f, 0.3426607f, 0.9394592f, 0.3484187f, 0.937339f, 0.3541635f, 0.9351835f, 0.359895f,
    0.9329928f, 0.365613f, 0.930767f, 0.3713172f, 0.9285061f, 0.3770074f, 0.9262102f, 0.3826834f,
    0.9238795f, 0.388345f, 0.921514f, 0.393992f, 0.9191139f, 0.3996242f, 0.9166791f, 0.4052413f,
    0.9142098f, 0.4108432f, 0.911706f, 0.4164296f, 0.909168f, 0.4220003f, 0.9065957f, 0.4275551f,
    0.9039893f, 0.4330938f, 0.9013488f, 0.4386162f, 0.8986745f, 0.4441221f, 0.8959662f, 0.4496113f,
    0.8932243f, 0.4550836f, 0.8904487f, 0.4605387f, 0.8876396f, 0.4659765f, 0.8847971f, 0.4713967f,
    0.8819213f, 0.4767992f, 0.8790122f, 0.4821838f, 0.8760701f, 0.4875502f, 0.873095f, 0.4928982f,
    0.870087f, 0.4982277f, 0.8670462f, 0.5035384f, 0.8639729f, 0.5088301f, 0.8608669f, 0.5141027f,
    0.8577286f, 0.519356f, 0.854558f, 0.5245897f, 0.8513552f, 0.5298036f, 0.8481203f, 0.5349976f,
    0.8448536f, 0.5401715f, 0.841555f, 0.545325f, 0.8382247f, 0.550458f, 0.8348629f, 0.5555702f,
    0.8314696f, 0.5606616f, 0.828045f, 0.5657318f, 0.8245893f, 0.5707807f, 0.8211025f, 0.5758082f,
    0.8175848f, 0.580814f, 0.8140363f, 0.5857979f, 0.8104572f, 0.5907597f, 0.8068476f, 0.5956993f,
    0.8032075f, 0.6006165f, 0.7995373f, 0.605511f, 0.7958369f, 0.6103828f, 0.7921066f, 0.6152316f,
    0.7883464f, 0.6200572f, 0.7845566f, 0.6248595f, 0.7807372f, 0.6296382f, 0.7768885f, 0.6343933f,
    0.7730105f, 0.6391244f, 0.7691033f, 0.6438315f, 0.7651673f, 0.6485144f, 0.7612024f, 0.6531728f,
    0.7572088f, 0.6578067f, 0.7531868f, 0.6624158f, 0.7491364f, 0.6669999f, 0.7450578f, 0.671559f,
    0.7409511f, 0.6760927f, 0.7368166f, 0.680601f, 0.7326543f, 0.6850837f, 0.7284644f, 0.6895405f,
    0.7242471f, 0.6939715f, 0.7200025f, 0.6983762f, 0.7157308f, 0.7027547f, 0.7114322f, 0.7071068f,
    0.7071068f, 0.7114322f, 0.7027547f, 0.7157308f, 0.6983762f, 0.7200025f, 0.6939715f, 0.7242471f,
    0.6895405f, 0.7284644f, 0.6850837f, 0.7326543f, 0.680601f, 0.7368166f, 0.6760927f, 0.7409511f,
    0.671559f, 0.7450578f, 0.6669999f, 0.7491364f, 0.6624158f, 0.7531868f, 0.6578067f, 0.7572088f,
    0.6531728f, 0.7612024f, 0.6485144f, 0.7651673f, 0.6438315f, 0.7691033f, 0.6391244f, 0.7730105f,
    0.6343933f, 0.7768885f, 0.6296382f, 0.7807372f, 0.6248595f, 0.7845566f, 0.6200572f, 0.7883464f,
    0.6152316f, 0.7921066f, 0.6103828f, 0.7958369f, 0.605511f, 0.7995373f, 0.6006165f, 0.8032075f,
    0.5956993f, 0.8068476f, 0.5907597f, 0.8104572f, 0.5857979f, 0.8140363f, 0.580814f, 0.8175848f,
    0.5758082f, 0.8211025f, 0.5707807f, 0.8245893f, 0.5657318f, 0.828045f, 0.5606616f, 0.8314696f,
    0.5555702f, 0.8348629f, 0.550458f, 0.8382247f, 0.545325f, 0.841555f, 0.5401715f, 0.8448536f,
    0.5349976f, 0.8481203f, 0.5298036f, 0.8513552f, 0.5245897f, 0.854558f, 0.519356f, 0.8577286f,
    0.5141027f, 0.8608669f, 0.5088301f, 0.8639729f, 0.5035384f, 0.8670462f, 0.4982277f, 0.870087f,
    0.4928982f, 0.873095f, 0.4875502f, 0.8760701f, 0.4821838f, 0.8790122f, 0.4767992f, 0.8819213f,
    0.4713967f, 0.8847971f, 0.4659765f, 0.8876396f, 0.4605387f, 0.8904487f, 0.4550836f, 0.8932243f,
    0.4496113f, 0.8959662f, 0.4441221f, 0.8986745f, 0.4386162f, 0.9013488f, 0.4330938f, 0.9039893f,
    0.4275551f, 0.9065957f, 0.4220003f, 0.909168f, 0.4164296f, 0.911706f, 0.4108432f, 0.9142098f,
    0.4052413f, 0.9166791f, 0.3996242f, 0.9191139f, 0.393992f, 0.921514f, 0.388345f, 0.9238795f,
    0.3826834f, 0.9262102f, 0.3770074f, 0.9285061f, 0.3713172f, 0.930767f, 0.365613f, 0.9329928f,
    0.359895f, 0.9351835f, 0.3541635f, 0.937339f, 0.3484187f, 0.9394592f, 0.3426607f, 0.9415441f,
    0.3368899f, 0.9435935f, 0.3311063f, 0.9456073f, 0.3253103f, 0.9475856f, 0.319502f, 0.9495282f,
    0.3136817f, 0.951435f, 0.3078496f, 0.953306f, 0.3020059f, 0.9551412f, 0.2961509f, 0.9569403f,
    0.2902847f, 0.9587035f, 0.2844075f, 0.9604305f, 0.2785197f, 0.9621214f, 0.2726214f, 0.9637761f,
    0.2667128f, 0.9653944f, 0.2607941f, 0.9669765f, 0.2548657f, 0.9685221f, 0.2489276f, 0.9700313f,
    0.2429802f, 0.9715039f, 0.2370236f, 0.97294f, 0.2310581f, 0.9743394f, 0.2250839f, 0.9757021f,
    0.2191012f, 0.9770281f, 0.2131103f, 0.9783174f, 0.2071114f, 0.9795698f, 0.2011046f, 0.9807853f,
    0.1950903f, 0.9819639f, 0.1890687f, 0.9831055f, 0.1830399f, 0.9842101f, 0.1770042f, 0.9852776f,
    0.1709619f, 0.9863081f, 0.1649131f, 0.9873014f, 0.1588581f, 0.9882576f, 0.1527972f, 0.9891765f,
    0.1467305f, 0.9900582f, 0.1406582f, 0.9909026f, 0.1345807f, 0.9917098f, 0.1284981f, 0.9924795f,
    0.1224107f, 0.9932119f, 0.1163186f, 0.993907f, 0.1102222f, 0.9945646f, 0.1041216f, 0.9951847f,
    0.0980171f, 0.9957674f, 0.091909f, 0.9963126f, 0.0857973f, 0.9968203f, 0.0796824f, 0.9972905f,
    0.0735646f, 0.9977231f, 0.0674439f, 0.9981181f, 0.0613207f, 0.9984756f, 0.0551952f, 0.9987955f,
    0.0490677f, 0.9990777f, 0.0429383f, 0.9993224f, 0.0368072f, 0.9995294f, 0.0306748f, 0.9996988f,
    0.0245412f, 0.9998306f, 0.0184067f, 0.9999247f, 0.0122715f, 0.9999812f, 0.0061359f, 1.0f, 0.0f,
};

/* The sine of an angle (1024 a turn), and its cosine in *c.
 * The first three quarters are each one line, if and body: IDO schedules by line, and split over
 * lines the table's lui moves above the index's andi and the cosine's store above an addu. */
f32 mathSinCos(u16 angle, f32 *c) {
    angle = (angle & 0x3FF) * 2;
    if (angle < 0x200) { angle++; *c = gMathSinTable[angle]; angle--; return gMathSinTable[angle]; }
    if (angle < 0x400) { angle = 0x401 - angle; *c = -gMathSinTable[angle]; angle--; return gMathSinTable[angle]; }
    if (angle < 0x600) { angle = angle - 0x3FF; *c = -gMathSinTable[angle]; angle--; return -gMathSinTable[angle]; }
    angle = 0x801 - angle;
    *c = gMathSinTable[angle];
    angle--;
    return -gMathSinTable[angle];
}

/* The sine of an angle in radians, from the table. */
f32 mathSin(f32 rad) {
    f32 c;

    return mathSinCos((s16) (rad * 162.9777527f), &c);
}

/* And its cosine. */
f32 mathCos(f32 rad) {
    f32 c;

    mathSinCos((s16) (rad * 162.9777527f), &c);
    return c;
}

/* A matrix from a translation, a rotation (radians: x, then y, then z) and a scale per row. */
void mathMtxTransRotScale(f32 m[4][4], f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx,
                            f32 sy, f32 sz) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 cxsy;
    f32 sxsy;

    sinX = mathSinCos((s16) (rx * 162.9777527f), &cosX);
    sinY = mathSinCos((s16) (ry * 162.9777527f), &cosY);
    sinZ = mathSinCos((s16) (rz * 162.9777527f), &cosZ);
    cxsy = cosX * sinY;
    m[0][0] = cosY * cosZ;
    m[0][1] = cosY * sinZ;
    m[0][2] = -sinY;
    sxsy = sinX * sinY;
    m[1][0] = sxsy * cosZ - cosX * sinZ;
    m[1][1] = sxsy * sinZ + cosX * cosZ;
    m[1][2] = sinX * cosY;
    m[2][0] = cxsy * cosZ + sinX * sinZ;
    m[2][1] = cxsy * sinZ - sinX * cosZ;
    m[2][2] = cosX * cosY;
    m[3][0] = tx;
    m[3][1] = ty;
    m[3][2] = tz;
    m[0][3] = m[1][3] = m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    if (sx != 1.0f) {
        m[0][0] *= sx;
        m[0][1] *= sx;
        m[0][2] *= sx;
    }
    if (sy != 1.0f) {
        m[1][0] *= sy;
        m[1][1] *= sy;
        m[1][2] *= sy;
    }
    if (sz != 1.0f) {
        m[2][0] *= sz;
        m[2][1] *= sz;
        m[2][2] *= sz;
    }
}

/* guMtxF2L with its arguments the other way round: a float matrix to a fixed-point one. */
void mathMtxFToL(Mtx *m, f32 mf[4][4]) {
    s32 i;
    s32 e1;
    s32 e2;
    s32 *ai;
    s32 *af;

    ai = (s32 *) &m->m[0][0];
    af = (s32 *) &m->m[2][0];
    for (i = 0; i < 4; i++) {
        e1 = FTOFIX32(mf[i][0]);
        e2 = FTOFIX32(mf[i][1]);
        *(ai++) = (e1 & 0xFFFF0000) | ((e2 >> 16) & 0xFFFF);
        *(af++) = ((e1 << 16) & 0xFFFF0000) | (e2 & 0xFFFF);
        e1 = FTOFIX32(mf[i][2]);
        e2 = FTOFIX32(mf[i][3]);
        *(ai++) = (e1 & 0xFFFF0000) | ((e2 >> 16) & 0xFFFF);
        *(af++) = ((e1 << 16) & 0xFFFF0000) | (e2 & 0xFFFF);
    }
}

/* The same, straight into a fixed-point matrix. */
void mathMtxTransRotScaleL(Mtx *mtx, f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx,
                            f32 sy, f32 sz) {
    f32 m[4][4];

    mathMtxTransRotScale(m, tx, ty, tz, rx, ry, rz, sx, sy, sz);
    mathMtxFToL(mtx, m);
}

/* A matrix that looks along (x, y, z), then a translation and a scale per row. */
void mathMtxLookTransScale(f32 m[4][4], f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 sx,
                            f32 sy, f32 sz) {
    mathMtxLookDir(m, x, y, z);
    m[3][0] = tx;
    m[3][1] = ty;
    m[3][2] = tz;
    if (sx != 1.0f) {
        m[0][0] *= sx;
        m[0][1] *= sx;
        m[0][2] *= sx;
    }
    if (sy != 1.0f) {
        m[1][0] *= sy;
        m[1][1] *= sy;
        m[1][2] *= sy;
    }
    if (sz != 1.0f) {
        m[2][0] *= sz;
        m[2][1] *= sz;
        m[2][2] *= sz;
    }
}

/* The same, straight into a fixed-point matrix. */
void mathMtxLookTransScaleL(Mtx *mtx, f32 x, f32 y, f32 z, f32 tx, f32 ty, f32 tz, f32 sx,
                            f32 sy, f32 sz) {
    f32 m[4][4];

    mathMtxLookTransScale(m, x, y, z, tx, ty, tz, sx, sy, sz);
    mathMtxFToL(mtx, m);
}

/* The inverse of mathMtxTransRotScale's matrix: the rotation by the negated angles in reverse order,
 * the translation turned back, and the scale divided out by column. */
void mathMtxTransRotScaleInv(f32 m[4][4], f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx,
                            f32 sy, f32 sz) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 cxsy;
    f32 sxsy;
    f32 pad; /* an unused local: the frame needs its slot */

    sinX = mathSinCos((s16) (-rx * 162.9777527f), &cosX);
    sinY = mathSinCos((s16) (-ry * 162.9777527f), &cosY);
    sinZ = mathSinCos((s16) (-rz * 162.9777527f), &cosZ);
    cxsy = cosX * sinY;
    sxsy = sinX * sinY;
    m[0][0] = cosY * cosZ;
    m[0][1] = sxsy * cosZ + cosX * sinZ;
    m[0][2] = -cxsy * cosZ + sinX * sinZ;
    m[1][0] = -cosY * sinZ;
    m[1][1] = -sxsy * sinZ + cosX * cosZ;
    m[1][2] = cxsy * sinZ + sinX * cosZ;
    m[2][0] = sinY;
    m[2][1] = -sinX * cosY;
    m[2][2] = cosX * cosY;
    m[3][0] = -(tx * m[0][0] + ty * m[1][0] + tz * m[2][0]);
    m[3][1] = -(tx * m[0][1] + ty * m[1][1] + tz * m[2][1]);
    m[3][2] = -(tx * m[0][2] + ty * m[1][2] + tz * m[2][2]);
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][3] = 1.0f;
    if (sx != 1.0f) {
        sx = 1.0f / sx;
        m[0][0] *= sx;
        m[1][0] *= sx;
        m[2][0] *= sx;
        m[3][0] *= sx;
    }
    if (sy != 1.0f) {
        sy = 1.0f / sy;
        m[0][1] *= sy;
        m[1][1] *= sy;
        m[2][1] *= sy;
        m[3][1] *= sy;
    }
    if (sz != 1.0f) {
        sz = 1.0f / sz;
        m[0][2] *= sz;
        m[1][2] *= sz;
        m[2][2] *= sz;
        m[3][2] *= sz;
    }
}

/* The rotation of mathMtxTransRotScale, transposed (its inverse), with no translation. */
void mathMtxRotInv(f32 m[4][4], f32 rx, f32 ry, f32 rz) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 cxsy;
    f32 sxsy;
    f32 pad[4];

    sinX = mathSinCos((s16) (rx * 162.9777527f), &cosX);
    sinY = mathSinCos((s16) (ry * 162.9777527f), &cosY);
    sinZ = mathSinCos((s16) (rz * 162.9777527f), &cosZ);
    cxsy = cosX * sinY;
    m[0][0] = cosY * cosZ;
    m[1][0] = cosY * sinZ;
    m[2][0] = -sinY;
    sxsy = sinX * sinY;
    m[0][1] = sxsy * cosZ - cosX * sinZ;
    m[1][1] = sxsy * sinZ + cosX * cosZ;
    m[2][1] = sinX * cosY;
    m[0][2] = cxsy * cosZ + sinX * sinZ;
    m[1][2] = cxsy * sinZ - sinX * cosZ;
    m[2][2] = cosX * cosY;
    m[0][3] = 0.0f;
    m[1][3] = 0.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

/* Inverts a matrix in place (Gauss-Jordan, no pivoting). It holds the file's one compiled-out
 * print; IDO still puts the dead call's string in .rodata, where the game has it.
 * Its steps follow Okumura's textbook gaussjordan, in floats. It works out the determinant but
 * never returns it: the function falls off its end (no mov.s to f0), and that is what put det
 * in f0 and the pivot in f2. "det = 1.0" (a double literal) keeps det's 1.0 apart from the 1.0
 * of 1 / pivot, as the original does. */
f32 fmtxInv(f32 m[4][4]) {
    s32 i;
    s32 j;
    s32 k;
    f32 p;
    f32 f;
    f32 det;

    det = 1.0;
    for (k = 0; k < 4; k++) {
        p = m[k][k];
        det *= p;
        if (p == 0.0f) {
            PRINTF("fmtxInv: 0 除算しました\n");
        }
        for (i = 0; i < 4; i++) {
            m[k][i] /= p;
        }
        m[k][k] = 1.0f / p;
        for (j = 0; j < 4; j++) {
            if (j != k) {
                f = m[j][k];
                for (i = 0; i < 4; i++) {
                    if (i != k) {
                        m[j][i] -= m[k][i] * f;
                    } else {
                        m[j][i] = -f / p;
                    }
                }
            }
        }
    }
}

/* Turns a rotation (1024 a turn) half round on x and z and mirrors it on y. */
void mathRotAltEuler(u16 *x, u16 *y, u16 *z) {
    *y = (0x200 - *y) & 0x3FF;
    *x = (*x + 0x200) & 0x3FF;
    *z = (*z + 0x200) & 0x3FF;
}

/* An identity matrix. The zeros are one chained assignment. */
void mathMtxIdent(f32 m[4][4]) {
    m[0][0] = 1.0f;
    m[3][2] = m[3][1] = m[3][0] = m[2][3] = m[2][1] = m[2][0] = m[1][3] = m[1][2] = m[1][0] =
        m[0][3] = m[0][2] = m[0][1] = 0.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

/* Rotates a matrix about x by a (radians): its rows 1 and 2. */
void mathMtxPreRotX(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[3][4];

    c = cosf(a);
    s = sinf(a);
    t[0][0] = c * m[1][0] - m[2][0] * s;
    t[0][1] = c * m[1][1] - m[2][1] * s;
    t[0][2] = c * m[1][2] - m[2][2] * s;
    t[1][0] = s * m[1][0] + m[2][0] * c;
    t[1][1] = s * m[1][1] + m[2][1] * c;
    t[1][2] = s * m[1][2] + m[2][2] * c;
    m[1][0] = t[0][0];
    m[1][1] = t[0][1];
    m[1][2] = t[0][2];
    m[2][0] = t[1][0];
    m[2][1] = t[1][1];
    m[2][2] = t[1][2];
}

/* About y: its rows 0 and 2. */
void mathMtxPreRotY(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[4][4];

    c = cosf(a);
    s = sinf(a);
    t[0][0] = c * m[0][0] - m[2][0] * s;
    t[0][1] = c * m[0][1] - m[2][1] * s;
    t[0][2] = c * m[0][2] - m[2][2] * s;
    t[2][0] = s * m[0][0] + m[2][0] * c;
    t[2][1] = s * m[0][1] + m[2][1] * c;
    t[2][2] = s * m[0][2] + m[2][2] * c;
    m[0][0] = t[0][0];
    m[0][1] = t[0][1];
    m[0][2] = t[0][2];
    m[2][0] = t[2][0];
    m[2][1] = t[2][1];
    m[2][2] = t[2][2];
}

/* About z: its rows 0 and 1. */
void mathMtxPreRotZ(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[4][4];

    c = cosf(a);
    s = sinf(a);
    t[0][0] = c * m[0][0] + m[1][0] * s;
    t[0][1] = c * m[0][1] + m[1][1] * s;
    t[0][2] = c * m[0][2] + m[1][2] * s;
    t[1][0] = -s * m[0][0] + m[1][0] * c;
    t[1][1] = -s * m[0][1] + m[1][1] * c;
    t[1][2] = -s * m[0][2] + m[1][2] * c;
    m[0][0] = t[0][0];
    m[0][1] = t[0][1];
    m[0][2] = t[0][2];
    m[1][0] = t[1][0];
    m[1][1] = t[1][1];
    m[1][2] = t[1][2];
}

/* Rotates a whole matrix (translation included) about x, with the table's sine: its columns 1
 * and 2. */
void mathMtxPostRotX(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[4][4];

    c = mathCos(a);
    s = mathSin(a);
    t[0][1] = c * m[0][1] + m[0][2] * s;
    t[0][2] = -s * m[0][1] + m[0][2] * c;
    t[1][1] = c * m[1][1] + m[1][2] * s;
    t[1][2] = -s * m[1][1] + m[1][2] * c;
    t[2][1] = c * m[2][1] + m[2][2] * s;
    t[2][2] = -s * m[2][1] + m[2][2] * c;
    t[3][1] = c * m[3][1] + m[3][2] * s;
    t[3][2] = -s * m[3][1] + m[3][2] * c;
    m[0][1] = t[0][1];
    m[0][2] = t[0][2];
    m[1][1] = t[1][1];
    m[1][2] = t[1][2];
    m[2][1] = t[2][1];
    m[2][2] = t[2][2];
    m[3][1] = t[3][1];
    m[3][2] = t[3][2];
}

/* About y: its columns 0 and 2. */
void mathMtxPostRotY(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[4][4];

    c = mathCos(a);
    s = mathSin(a);
    t[0][0] = c * m[0][0] + m[0][2] * s;
    t[0][2] = -s * m[0][0] + m[0][2] * c;
    t[1][0] = c * m[1][0] + m[1][2] * s;
    t[1][2] = -s * m[1][0] + m[1][2] * c;
    t[2][0] = c * m[2][0] + m[2][2] * s;
    t[2][2] = -s * m[2][0] + m[2][2] * c;
    t[3][0] = c * m[3][0] + m[3][2] * s;
    t[3][2] = -s * m[3][0] + m[3][2] * c;
    m[0][0] = t[0][0];
    m[0][2] = t[0][2];
    m[1][0] = t[1][0];
    m[1][2] = t[1][2];
    m[2][0] = t[2][0];
    m[2][2] = t[2][2];
    m[3][0] = t[3][0];
    m[3][2] = t[3][2];
}

/* About z: its columns 0 and 1. */
void mathMtxPostRotZ(f32 m[4][4], f32 a) {
    f32 c;
    f32 s;
    f32 t[4][4];

    c = mathCos(a);
    s = mathSin(a);
    t[0][0] = c * m[0][0] - m[0][1] * s;
    t[0][1] = s * m[0][0] + m[0][1] * c;
    t[1][0] = c * m[1][0] - m[1][1] * s;
    t[1][1] = s * m[1][0] + m[1][1] * c;
    t[2][0] = c * m[2][0] - m[2][1] * s;
    t[2][1] = s * m[2][0] + m[2][1] * c;
    t[3][0] = c * m[3][0] - m[3][1] * s;
    t[3][1] = s * m[3][0] + m[3][1] * c;
    m[0][0] = t[0][0];
    m[0][1] = t[0][1];
    m[1][0] = t[1][0];
    m[1][1] = t[1][1];
    m[2][0] = t[2][0];
    m[2][1] = t[2][1];
    m[3][0] = t[3][0];
    m[3][1] = t[3][1];
}

/* Moves a matrix by (x, y, z) in its own axes: the translation row becomes the point
 * transformed. */
void mathMtxPreTrans(f32 m[4][4], f32 x, f32 y, f32 z) {
    f32 t[4];

    t[0] = x * m[0][0] + y * m[1][0] + z * m[2][0] + m[3][0];
    t[1] = x * m[0][1] + y * m[1][1] + z * m[2][1] + m[3][1];
    t[2] = x * m[0][2] + y * m[1][2] + z * m[2][2] + m[3][2];
    m[3][0] = t[0];
    m[3][1] = t[1];
    m[3][2] = t[2];
}

/* Moves a matrix by (x, y, z). */
void mathMtxPostTrans(f32 m[4][4], f32 x, f32 y, f32 z) {
    m[3][0] += x;
    m[3][1] += y;
    m[3][2] += z;
}

/* Scales a matrix's rotation along x (its first column). */
void mathMtxScaleRotX(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][0] *= s;
        m[1][0] *= s;
        m[2][0] *= s;
    }
}

/* Along y. */
void mathMtxScaleRotY(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][1] *= s;
        m[1][1] *= s;
        m[2][1] *= s;
    }
}

/* Along z. */
void mathMtxScaleRotZ(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][2] *= s;
        m[1][2] *= s;
        m[2][2] *= s;
    }
}

/* Scales a matrix's rotation evenly. */
void mathMtxScaleRot(f32 m[4][4], f32 s) {
    mathMtxScaleRotX(m, s);
    mathMtxScaleRotY(m, s);
    mathMtxScaleRotZ(m, s);
}

/* Scales a whole matrix column along x (with its translation). */
void mathMtxScaleX(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][0] *= s;
        m[1][0] *= s;
        m[2][0] *= s;
        m[3][0] *= s;
    }
}

/* Along y. */
void mathMtxScaleY(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][1] *= s;
        m[1][1] *= s;
        m[2][1] *= s;
        m[3][1] *= s;
    }
}

/* Along z. */
void mathMtxScaleZ(f32 m[4][4], f32 s) {
    if (s != 1.0f) {
        m[0][2] *= s;
        m[1][2] *= s;
        m[2][2] *= s;
        m[3][2] *= s;
    }
}

/* Scales a matrix evenly, translation included. */
void mathMtxScale(f32 m[4][4], f32 s) {
    mathMtxScaleX(m, s);
    mathMtxScaleY(m, s);
    mathMtxScaleZ(m, s);
}

/* A matrix that looks along the direction (x, y, z); the identity for a straight up or down
 * one. */
void mathMtxLookDir(f32 m[4][4], f32 x, f32 y, f32 z) {
    f32 len;
    f32 inv;

    guNormalize(&x, &y, &z);
    len = sqrtf(x * x + z * z);
    if (len != 0.0f) {
        inv = 1.0f / len;
        m[0][0] = -z * inv;
        m[1][0] = -y * x * inv;
        m[2][0] = -x;
        m[3][0] = 0.0f;
        m[0][1] = 0.0f;
        m[1][1] = len;
        m[2][1] = -y;
        m[3][1] = 0.0f;
        m[0][2] = x * inv;
        m[1][2] = -y * z * inv;
        m[2][2] = -z;
        m[3][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][3] = 0.0f;
        m[2][3] = 0.0f;
        m[3][3] = 1.0f;
    } else {
        mathMtxIdent(m);
    }
}

/* Normalises a vector (unless it is zero). */
void mathVecNormalize(Vec3f *v) {
    f32 len = sqrtf(v->z * v->z + (v->x * v->x + v->y * v->y));

    if (len != 0.0f) {
        v->x = v->x / len;
        v->y = v->y / len;
        v->z = v->z / len;
    }
}

/* The same, summed in order. */
void mathVecNormalizeB(Vec3f *v) {
    f32 x = v->x;
    f32 y = v->y;
    f32 z = v->z;
    f32 len = sqrtf(x * x + y * y + z * z);

    if (len != 0.0f) {
        v->x = x / len;
        v->y = y / len;
        v->z = z / len;
    }
}

/* Scales a vector. */
void mathVecScale(Vec3f *v, f32 s) {
    v->x *= s;
    v->y *= s;
    v->z *= s;
}

/* The arctangent, in doubles: a continued fraction of 19 terms, on x or on 1/x above 1. */
f32 mathAtan(f32 x) {
    f64 dx;
    f64 d;
    f64 acc;
    f64 n;

    acc = 0.0;
    dx = x;
    if (fabsf(dx) <= 1.0f) {
        d = dx;
    } else {
        d = 1.0 / dx;
    }
    for (n = 19.0; n >= 1.0; n -= 1.0) {
        acc = n * d * n * d / (1.0 + acc + 2 * n);
    }
    d = d / (1.0 + acc);
    if (dx > 1.0) {
        return PI / 2 - d;
    }
    if (dx < -1.0) {
        return -PI / 2 - d;
    }
    return d;
}

/* The angle of (x, y) from the x axis: atan2(y, x). */
f32 mathAtan2(f32 y, f32 x) {
    if (!(fabsf(x) < 1e-6)) {
        if (x < 0.0f) {
            return mathAtan(y / x) + PI;
        }
        return mathAtan(y / x);
    }
    if (fabsf(y) < 1e-6) {
        return 0.0f;
    }
    if (y < 0.0f) {
        return -PI / 2;
    }
    return PI / 2;
}

/* Turns the point (*x, *y, *z) by mathMtxTransRotScale's rotation (radians: x, then y, then z).
 * MATCHED (2026-10-03): the plain sums with *x, *y and *z read in place, no tx/ty/tz
 * locals and no FAKE. uopt makes the three reads common subexpressions: *z gets f18, *x and *y
 * its own temps at 0x2C and 0x28 (below the 11 named locals; the frame stays 0x60), and it
 * places them, with cxsy and sxsy, just before nx's statement in the order of the expression
 * tree, so cosX (for cxsy) is loaded before *x and *y as in the game. With named tx/ty the
 * reads are statements of their own and stay above the products, which uopt always sinks to
 * their first use (that was the hill below). Why the registers followed (cc -S, IDO_NOTES 279):
 * ugen runs its float-temp FIFO over the function twice and starts the real pass with the
 * queue the first pass ended with, so the prologue's temps are the fixed point of the whole
 * function's order. This form matched too, with a FAKE struct view of cosX in cxsy and nx's row
 * (a load through a view is not propagated); not kept. Also measured (a0, scratch): cosX as
 * "f32 cosX[1]" is not sunk but IDO then sorts the rows' operands; "(&cosX)[0]", a volatile
 * read or a union view: frame 0x68; a one-field struct: sunk like a scalar.
 * History, NON_MATCHING (registers): the frame and the stack slots agree; the
 * float registers differ from the first product, and the original spills more of its
 * temporaries to the stack. Its matched twin mathVecRotateInv (the inverse turn, just above)
 * shows the way: the load order is set by the statement order, and each z factor sits in a
 * variable dead at that point (FAKE) to give "mul.s r, z, k". Read off this one (2026-10-02):
 * the game loads *x, then *z, then *y, and spills tx (0x2C) and ty (0x28). The twin's FAKE form
 * gave 144 differing lines (permuter score 3910). The permuter then found natural C that is
 * closer: the plain sums, with "tz = *z;" read between cxsy and sxsy (score 696, 172 lines:
 * registers only, no reordering left). Its next best, a FAKE "(0, tz)" in nx (score 490): 170
 * lines, not kept. 2026-10-02: a FAKE block boundary before the three stores (the splits
 * sweep, IDO_NOTES 228): 142 lines, float register names only (from the first constant on:
 * the game's 162.98 in f4, here f10). 2026-10-03 (142 to 136, the register sweep's find): nz
 * summed in two statements, the z-term added last; ny the same way: 181. The greedy mode
 * (2026-10-03, 136 to 130, mix distance 0): FAKE empty tests of sinX and nz at the end.
 * 2026-10-03 (130 to 62): sxsy computed BEFORE cxsy (polygon.c polModelNodeMtx's
 * order, IDO_NOTES 248) gives the game's sxsy f2, cxsy f12, sinX f14, sinY f16, tz f18; each
 * row's z factor k first assigned to the row's own variable ("nx = k; nx = tz * nx + (...)", the
 * twin's way: uopt substitutes it back, and the multiply comes out "mul.s r, tz, k"); the empty
 * tests dropped. From the third call to ny's store everything is the game's. Left: the nz row
 * (the game keeps nz in f0 and reloads cosY into f10 late; this C puts nz in f2 and reloads cosY
 * early into the dead sinZ's f0) and, from that, the prologue's float temps (f8/f10 for f10/f4:
 * the prologue's registers follow the body, the v00 form had the game's). Scratch forms: "tz *
 * k" written inline comes out "mul.s r, k, tz" (and "k * tz" too); boundary between ny and nz:
 * 8 instructions short; *z written directly: 6 short; empty test of sxsy at the end: worse.
 * nz's "tx * -sinY" for "-sinY * tx" (the game loads tx before the neg): 62 to 60, kept.
 * THE HILL (measured 2026-10-03): without the "do {} while (0)" this C is the game's code
 * instruction for instruction (nz in f0, no early cosY reload, mix 0) with the float temps
 * f4/f6/f8 permuted all through: 158 lines, so not kept. ugen hands out f4-f10 from a FIFO free
 * queue (every temp of this C, and of the twin, fits that model); read that way, the game's ugen
 * loads cosX (for cxsy) BEFORE *x and *y, while this C's ugen loads *z, *x, *y first and sinks
 * the register products to their first use, whatever their source order (all 16 orders of sxsy,
 * cxsy, tz, tx, ty tried). Keeping cxsy above the loads took a redefinition between ("sinY =
 * sinY * 1": a real mul) or grew the frame by 8 ("sinY = sinY;", "cosX = cosX;", cosX * sinY
 * inline, empty tests of cxsy/cosX/sinY there). Find what keeps cxsy above *x's load with no
 * code and frame 0x60, then drop the boundary. Empty test of nz at the end with the boundary:
 * one nop more (mix 1), not kept. Greedy on 60 (2026-10-03, every mode): 0 kept steps. The
 * hill with a "do {} while (0)" right after cxsy instead (to hold it above the loads): 197,
 * 7 instructions short. A float function compiled just above (a probe): the same 60 lines,
 * the same temps, so ugen's float queue starts fresh in each function. nx's z factor line
 * moved above the three loads (cxsy's first use before them, as the game's ugen has it): the
 * same 60 (uopt substitutes the line back into the row wherever it sits). */
void mathVecRotate(f32 rx, f32 ry, f32 rz, f32 *x, f32 *y, f32 *z) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 sxsy;
    f32 cxsy;
    f32 nx;
    f32 ny;
    f32 nz;

    sinX = mathSinCos((s16) (rx * 162.9777527f), &cosX);
    sinY = mathSinCos((s16) (ry * 162.9777527f), &cosY);
    sinZ = mathSinCos((s16) (rz * 162.9777527f), &cosZ);
    sxsy = sinX * sinY;
    cxsy = cosX * sinY;
    nx = cosY * cosZ * *x + (sxsy * cosZ - cosX * sinZ) * *y + (cxsy * cosZ + sinX * sinZ) * *z;
    ny = cosY * sinZ * *x + (sxsy * sinZ + cosX * cosZ) * *y + (cxsy * sinZ - sinX * cosZ) * *z;
    nz = -sinY * *x + sinX * cosY * *y + cosX * cosY * *z;
    *x = nx;
    *y = ny;
    *z = nz;
}

/* The same turned back: the point by the transposed rotation (mathMtxRotInv's).
 * Written as its twin mathVecRotate (2026-10-03, no FAKE): the plain sums in row order with *x,
 * *y and *z read in place and no tx/ty/tz locals; the frame stays 0x60. IDO evaluates the right
 * operand of a sum first, which gives the game's add order (the z-term first) and its pointer
 * registers (z v0, y v1, x a0), and a read in place gives the game's "mul.s r, z, k" where a
 * named tz gave "mul.s r, k, tz". Before this, "tz = *z; tx = *x; ty = *y;" and each z factor
 * held in a variable dead at that point (ny, nz, sinY: a FAKE) did the same. */
void mathVecRotateInv(f32 rx, f32 ry, f32 rz, f32 *x, f32 *y, f32 *z) {
    f32 sinX;
    f32 cosX;
    f32 sinY;
    f32 cosY;
    f32 sinZ;
    f32 cosZ;
    f32 sxsy;
    f32 cxsy;
    f32 nx;
    f32 ny;
    f32 nz;

    sinX = mathSinCos((s16) (rx * 162.9777527f), &cosX);
    sinY = mathSinCos((s16) (ry * 162.9777527f), &cosY);
    sinZ = mathSinCos((s16) (rz * 162.9777527f), &cosZ);
    cxsy = cosX * sinY;
    sxsy = sinX * sinY;
    nx = cosY * cosZ * *x + cosY * sinZ * *y + -sinY * *z;
    ny = (sxsy * cosZ - cosX * sinZ) * *x + (sxsy * sinZ + cosX * cosZ) * *y + sinX * cosY * *z;
    nz = (cxsy * cosZ + sinX * sinZ) * *x + (cxsy * sinZ - sinX * cosZ) * *y + cosX * cosY * *z;
    *x = nx;
    *y = ny;
    *z = nz;
}

/* Turns the point (*x, *z) about the y axis by a radians, in place (y is not used).
 * The textbook rotation, with *x and *z read in place (2026-10-03, no FAKE): IDO evaluates the
 * right operand of a sum first, so the z-terms written last are computed first and z takes v0
 * (x v1), as in the game; the two reads become common subexpressions, z in f2 and x in f12.
 * "c * *x" and "-s * *x" give the game's "mul.s c, x"; "*x * c" gives "mul.s x, c". Before this
 * the unused y was set to z ahead of "a = *x" (FAKE) to put z's pointer first; written
 * "*z * s + c * *x" the x-terms come first and x takes v0. */
void mathVecRotateY(f32 a, f32 *x, f32 *y, f32 *z) {
    f32 nz;
    f32 c;
    f32 s;
    f32 nx;

    s = mathSinCos((s16) (a * 162.9777527f), &c);
    nx = c * *x + *z * s;
    nz = -s * *x + *z * c;
    *x = nx;
    *z = nz;
}

/* The same for the point (x, z), into *outX and *outZ. */
void mathXZRotateY(f32 a, f32 x, f32 z, f32 *outX, f32 *outZ) {
    f32 tz;
    f32 c;
    f32 s;

    s = (*mathSinCos)((s16) (a * 162.9777527f), &c);
    tz = -s * x + c * z;
    *outX = c * x + s * z;
    *outZ = tz;
}

/* A point on a cardinal spline segment through p[1] and p[2] (p[0] and p[3] steer it), at
 * t from 0 to 1, with tension s. The four blend weights are:
 *   p[0]: (2t^2 - t^3 - t)s          p[1]: (2 - s)t^3 + (s - 3)t^2 + 1
 *   p[2]: (s - 2)t^3 + (3 - 2s)t^2 + st      p[3]: (t^3 - t^2)s
 * Written as vram.c's vramCardinalPoint, its twin instruction for instruction (the weights in
 * locals; the unused fourth sets the frame and the spill slots). */
void mathCardinalSpline(Vec3f *out, Vec3f *p, f32 tension, f32 t) {
    f32 t2;
    f32 s;
    f32 t3;
    f32 h0;
    f32 h1;
    f32 h2;
    f32 h3; /* unused: p[3]'s weight is written out */

    t2 = t * t;
    s = tension;
    t3 = t2 * t;
    h0 = (2.0f * t2 - t3 - t) * s;
    h1 = (2.0f - s) * t3 + (s - 3.0f) * t2 + 1.0f;
    h2 = (s - 2.0f) * t3 + (3.0f - 2.0f * s) * t2 + s * t;
    out->x = p[0].x * h0 + p[1].x * h1 + p[2].x * h2 + p[3].x * ((t3 - t2) * tension);
    out->y = p[0].y * h0 + p[1].y * h1 + p[2].y * h2 + p[3].y * ((t3 - t2) * tension);
    out->z = p[0].z * h0 + p[1].z * h1 + p[2].z * h2 + p[3].z * ((t3 - t2) * tension);
}

/* The point at t (0 to 1) along a path: straight lines or a cardinal spline. */
void mathSplinePoint(Vec3f *out, Spline *path, f32 t) {
    s16 i;
    Vec3f *p;

    if (t < 0.0f || t > 1.0f) {
        return;
    }
    if (t < 1.0f) {
        t *= path->n - 1;
        i = t;
        t -= i;
        switch (path->kind) {
            case 0:
                p = &path->points[i];
                out->x = (p[1].x - p[0].x) * t + p[0].x;
                out->y = (p[1].y - p[0].y) * t + p[0].y;
                out->z = (p[1].z - p[0].z) * t + p[0].z;
                break;
            case 1:
                mathCardinalSpline(out, &path->points[i], path->tension, t);
                break;
        }
    } else {
        i = path->n - 1;
        switch (path->kind) {
            case 0:
                p = &path->points[i];
                *out = *p;
                break;
            case 1:
                p = &path->points[i + 1];
                *out = *p;
                break;
        }
    }
}

/* The linear solvers: the LU decomposition and the inverse, for 3x3 and 4x4 matrices. They are
 * Okumura's textbook lu and matinv (as fmtxInv is his gaussjordan), in floats and with n
 * fixed, his locals and all: ii = ip[i] and ik read once, u for both the largest and the pivot,
 * j for the pivot's row, the weights declared last, and "goto EXIT" for a singular matrix. */
/* Decomposes a 3x3 matrix in place by Gaussian elimination with scaled partial pivoting: the
 * multipliers go below the diagonal, U on and above it, and the row order into p. Returns the
 * determinant (0 for a singular matrix). */
f32 mathLuDecomp3(f32 a[3][3], s32 p[3]) {
    s32 i;
    s32 j;
    s32 k;
    s32 ii;
    s32 ik;
    f32 t;
    f32 u;
    f32 det;
    f32 weight[3];

    det = 0.0f;
    for (k = 0; k < 3; k++) {
        p[k] = k;
        u = 0.0f;
        for (j = 0; j < 3; j++) {
            t = fabsf(a[k][j]);
            if (t > u) {
                u = t;
            }
        }
        if (u == 0.0f) {
            goto end;
        }
        weight[k] = 1.0f / u;
    }
    det = 1.0f;
    for (k = 0; k < 3; k++) {
        u = -1.0f;
        for (i = k; i < 3; i++) {
            ii = p[i];
            t = fabsf(a[ii][k]) * weight[ii];
            if (t > u) {
                u = t;
                j = i;
            }
        }
        ik = p[j];
        if (j != k) {
            p[j] = p[k];
            p[k] = ik;
            det = -det;
        }
        u = a[ik][k];
        det *= u;
        if (u == 0.0f) {
            goto end;
        }
        for (i = k + 1; i < 3; i++) {
            ii = p[i];
            t = (a[ii][k] /= u);
            for (j = k + 1; j < 3; j++) {
                a[ii][j] -= t * a[ik][j];
            }
        }
    }
end:
    return det;
}

/* Inverts a 3x3 matrix into inv, column by column, through mathLuDecomp3's decomposition of a
 * copy (forward, then back substitution). Returns the determinant; inv is untouched when it is
 * 0. */
f32 mathMatInv3(f32 a[3][3], f32 inv[3][3]) {
    f32 det;
    f32 x;
    s32 i;
    s32 j;
    s32 c;
    s32 ii;
    f32 lu[3][3];
    s32 p[3];

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            lu[i][j] = a[i][j];
        }
    }
    det = mathLuDecomp3(lu, p);
    if (det != 0.0f) {
        for (c = 0; c < 3; c++) {
            for (i = 0; i < 3; i++) {
                ii = p[i];
                x = (ii == c);
                for (j = 0; j < i; j++) {
                    x -= lu[ii][j] * inv[j][c];
                }
                inv[i][c] = x;
            }
            for (i = 2; i >= 0; i--) {
                x = inv[i][c];
                ii = p[i];
                for (j = i + 1; j < 3; j++) {
                    x -= lu[ii][j] * inv[j][c];
                }
                inv[i][c] = x / lu[ii][i];
            }
        }
    }
    return det;
}

/* mathLuDecomp3 for a 4x4 matrix. */
f32 mathLuDecomp4(f32 a[4][4], s32 p[4]) {
    s32 i;
    s32 j;
    s32 k;
    s32 ii;
    s32 ik;
    f32 t;
    f32 u;
    f32 det;
    f32 weight[4];

    det = 0.0f;
    for (k = 0; k < 4; k++) {
        p[k] = k;
        u = 0.0f;
        for (j = 0; j < 4; j++) {
            t = fabsf(a[k][j]);
            if (t > u) {
                u = t;
            }
        }
        if (u == 0.0f) {
            goto end;
        }
        weight[k] = 1.0f / u;
    }
    det = 1.0f;
    for (k = 0; k < 4; k++) {
        u = -1.0f;
        for (i = k; i < 4; i++) {
            ii = p[i];
            t = fabsf(a[ii][k]) * weight[ii];
            if (t > u) {
                u = t;
                j = i;
            }
        }
        ik = p[j];
        if (j != k) {
            p[j] = p[k];
            p[k] = ik;
            det = -det;
        }
        u = a[ik][k];
        det *= u;
        if (u == 0.0f) {
            goto end;
        }
        for (i = k + 1; i < 4; i++) {
            ii = p[i];
            t = (a[ii][k] /= u);
            for (j = k + 1; j < 4; j++) {
                a[ii][j] -= t * a[ik][j];
            }
        }
    }
end:
    return det;
}

/* mathMatInv3 for a 4x4 matrix. */
f32 mathMatInv4(f32 a[4][4], f32 inv[4][4]) {
    f32 det;
    f32 x;
    s32 i;
    s32 j;
    s32 c;
    s32 ii;
    f32 lu[4][4];
    s32 p[4];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            lu[i][j] = a[i][j];
        }
    }
    det = mathLuDecomp4(lu, p);
    if (det != 0.0f) {
        for (c = 0; c < 4; c++) {
            for (i = 0; i < 4; i++) {
                ii = p[i];
                x = (ii == c);
                for (j = 0; j < i; j++) {
                    x -= lu[ii][j] * inv[j][c];
                }
                inv[i][c] = x;
            }
            for (i = 3; i >= 0; i--) {
                x = inv[i][c];
                ii = p[i];
                for (j = i + 1; j < 4; j++) {
                    x -= lu[ii][j] * inv[j][c];
                }
                inv[i][c] = x / lu[ii][i];
            }
        }
    }
    return det;
}
