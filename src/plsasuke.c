#include "player.h"

/* Sasuke (character 2): his animation data (overlay 9's .data, 0x801AE8B0-0x801B15F4, then
 * IDO's 16-byte alignment), laid out as Goemon's (plgoemon.c): face animations, three piece
 * tables, the actions, the attack boxes and the event sounds. Only data: plmodel.c's and
 * plsound.c's tables point at it. */

AnimFrame gPlSasukeFace0Frames[] = {
    { 0x04002520, 4 }, { 0x04001CE0, 6 }, { 0x04001080, 6 }, { 0x04000C60, 6 }, { 0x04001080, 6 },
    { 0x04001CE0, 6 }, { 0x04002520, 4 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace0[2] = { gPlSasukeFace0Frames, gPlSasukeFace0Frames };
AnimFrame gPlSasukeFace1Frames[] = {
    { 0x04002520, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace1[2] = { gPlSasukeFace1Frames, gPlSasukeFace1Frames };
AnimFrame gPlSasukeFace2Frames[] = {
    { 0x04001080, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace2[2] = { gPlSasukeFace2Frames, gPlSasukeFace2Frames };
AnimFrame gPlSasukeFace3Frames[] = {
    { 0x040014A0, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace3[2] = { gPlSasukeFace3Frames, gPlSasukeFace3Frames };
AnimFrame gPlSasukeFace4Frames[] = {
    { 0x04000840, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace4[2] = { gPlSasukeFace4Frames, gPlSasukeFace4Frames };
AnimFrame gPlSasukeFace5Frames[] = {
    { 0x04000420, 4 }, { 0x04003180, 8 }, { 0x04002940, 8 }, { 0x04002100, 8 }, { 0x04002940, 8 },
    { 0x04003180, 8 }, { 0x04000420, 2 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace5[2] = { gPlSasukeFace5Frames, gPlSasukeFace5Frames };
AnimFrame gPlSasukeFace6Frames[] = {
    { 0x040018C0, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace6[2] = { gPlSasukeFace6Frames, gPlSasukeFace6Frames };
AnimFrame gPlSasukeFace7EyesFrames[] = {
    { 0x04000840, 1 }, { 0, 0 },
};
AnimFrame gPlSasukeFace7MouthFrames[] = {
    { 0x04002520, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace7[2] = { gPlSasukeFace7EyesFrames, gPlSasukeFace7MouthFrames };
AnimFrame gPlSasukeFace8Frames[] = {
    { 0x04000000, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace8[2] = { gPlSasukeFace8Frames, gPlSasukeFace8Frames };
AnimFrame gPlSasukeFace9Frames[] = {
    { 0x04002520, 3 }, { 0x04001CE0, 6 }, { 0x04001080, 6 }, { 0x04000C60, 6 }, { 0x04001080, 6 },
    { 0x04001CE0, 6 }, { 0x04002520, 3 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace9[2] = { gPlSasukeFace9Frames, gPlSasukeFace9Frames };
AnimFrame gPlSasukeFace10Frames[] = {
    { 0x04002D60, 1 }, { 0, 0 },
};
AnimFrame *gPlSasukeFace10[2] = { gPlSasukeFace10Frames, gPlSasukeFace10Frames };

/* The model pieces (segment 3 offsets in file 0x1A9), by PlActAnim.seg; 0 ends them. */
u32 gPlSasukePieceTable[] = {
    0x03000000, 0x03000770, 0x03001190, 0x03001B00, 0x03002500, 0x03002EF0,
    0x03003920, 0x030041F0, 0x03004AB0, 0x03005260, 0x030059B0, 0x03006130,
    0x030062F0, 0x030064B0, 0x03006670, 0x03006A70, 0x03006E00, 0x03007190,
    0x03007750, 0x03007C80, 0x03008DF0, 0x03009270, 0x03009B80, 0x0300A070,
    0x0300AA40, 0x0300B410, 0x0300BA00, 0x0300C0B0, 0x0300C760, 0x0300CE90,
    0x0300D6D0, 0x0300DC00, 0x0300E630, 0x0300EEB0, 0x0300FC40, 0x03010640,
    0x03010CC0, 0x03011590, 0x03011950, 0x03012450, 0x03012A60, 0x03012FF0,
    0x03013740, 0x03014240, 0x03014850, 0x03014EF0, 0x030155E0, 0x03015A90,
    0x03015EC0, 0x030160E0, 0x03016420, 0x03016B70, 0x03017230, 0x030176F0,
    0x03017BC0, 0x030183C0, 0x03018740, 0x03018DC0, 0x03019540, 0x03019FE0,
    0x0301AC00, 0x0301B6E0, 0x0301BFD0, 0x0301D250, 0x0301D6A0, 0x0301DFC0,
    0x0301E770, 0x0301EF90, 0x0301F650, 0x0301FD10, 0x030205B0, 0x03020D40,
    0x030211C0, 0x030219A0, 0x03022110, 0x030225C0, 0x03022BC0, 0x030231C0,
    0x030237C0, 0x03024380, 0x03024F40, 0x03025B00, 0x030263A0, 0x03026C30,
    0x030274C0, 0x030280D0, 0x03028CD0, 0x030298D0, 0x0302A2C0, 0x0302ACB0,
    0x0302B6A0, 0x0302BE90, 0x0302C680, 0x0302CE70, 0x0302D2D0, 0x0302D730,
    0x0302DB90, 0x0302DFE0, 0x0302E430, 0x0302E880, 0x0302EE10, 0x0302F5C0,
    0x0302FB30, 0x03030400, 0x03030A60, 0x03031450, 0x03031C40, 0x03032570,
    0x03032C70, 0x03032F00, 0x03033460, 0x03033ED0, 0x03034170, 0x030342F0,
    0x030344C0, 0x03034670, 0x03034820, 0x03034D20, 0x03035000, 0x03035570,
    0x03035B20, 0x03036280, 0x030369C0, 0x030370F0, 0x030374F0, 0x03037C50,
    0x030383C0, 0x03038800, 0x03038F60, 0x030396A0, 0x03039DE0, 0x0303A540,
    0x0303A7D0, 0x0303AA20, 0x0303AD50, 0x0303B4B0, 0x0303B960, 0x0303BEA0,
    0x0303C370, 0x0303CAE0, 0x0303CCD0, 0x0303D690, 0x0303DE90, 0x0303E8A0,
    0x0303EA90, 0x0303ECB0, 0x0303F1F0, 0x0303F5A0, 0x0303F790, 0x0303FE60,
    0,
};

/* The attack box pieces (segment 0x0B, file 0x1AB), by PlActAnim.atk; 0 ends them. */
u32 gPlSasukeAtkPieceTable[] = {
    0x0B000000, 0x0B000310, 0x0B000620, 0x0B000800, 0x0B000AF0, 0x0B000C60,
    0x0B000E60, 0x0B001050, 0x0B001340, 0x0B0014B0, 0x0B001650, 0x0B001800,
    0x0B001B10, 0x0B001DE0, 0x0B001F60, 0x0B002130, 0x0B0022D0, 0x0B002440,
    0,
};

/* The other pieces (segment 5, file 0x1AC), by PlActAnim.zanzouPiece; 0 ends them. */
u32 gPlSasukeSeg5PieceTable[] = {
    0x05000000, 0x05000410, 0x05000820, 0x05000C10, 0x05000E50, 0x05001240,
    0x05001480, 0x050016F0, 0x05001970, 0x05001D70, 0x05002130, 0x05002340,
    0x05002560, 0x05002750, 0,
};

/* The actions, by action number. */
PlActAnim gPlSasukeActAnimTable[] = {
    /* 0 */
    { 0x03000758, 0, 0, 0,0.4f, 0, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03001178, 0, 0x04000178, 0,0.4f, 1, -1, -1, 0, 0, 0x5, gPlSasukeFace4 },
    { 0x03001AF4, 0, 0x040001A8, 0,0.6f, 2, -1, -1, 1, 0, 0x5, gPlSasukeFace4 },
    { 0x030024EC, 0, 0x040001D8, 0,0.6f, 3, -1, -1, 2, 0, 0x5, gPlSasukeFace4 },
    { 0x03002EE4, 0, 0x04000208, 0,1.4f, 4, -1, -1, 3, 0, 0x5, gPlSasukeFace4 },
    { 0x03003910, 0, 0, 0,0.9f, 5, -1, -1, -1, 0, 0x1, gPlSasukeFace4 },
    { 0x030041D8, 0, 0, 0,2.0f, 6, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03004AA0, 0, 0, 0,2.2f, 7, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03005248, 0, 0, 0,0.6f, 8, -1, -1, -1, 0, 0x1, gPlSasukeFace1 },
    { 0x0300599C, 0, 0, 0,0.8f, 9, -1, -1, -1, 0, 0x1, gPlSasukeFace1 },
    /* 10 */
    { 0x03006120, 0, 0, 0,0.8f, 10, -1, -1, -1, 0, 0x1, gPlSasukeFace1 },
    { 0x030062E4, 0, 0, 0,0.8f, 11, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x030064A4, 0, 0, 0,0.8f, 12, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03006664, 0, 0, 0,0.8f, 13, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03006A64, 0, 0, 0,4.0f, 14, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03006DF4, 0, 0, 0,2.0f, 15, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x0300717C, 0, 0, 0,1.2f, 16, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x03007744, 0, 0, 0,0.2f, 17, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x03007C68, 0, 0, 0,0.6f, 18, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x03001178, 0, 0, 0,0.1f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace4 },
    /* 20 */
    { 0x03001178, 0, 0, 0,0.1f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace4 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace4 },
    { 0x03008DE0, 0, 0, 0,1.0f, 19, -1, -1, -1, 0, 0x0, gPlSasukeFace2 },
    { 0x0300925C, 0, 0x04000238, 0,0.45f, 20, -1, -1, 4, 0, 0x5, gPlSasukeFace0 },
    { 0x03009B70, 0, 0, 0,0.7f, 21, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0300A05C, 0, 0, 0,2.5f, 22, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x0300A05C, 0, 0, 0,-2.5f, 22, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x0300AA2C, 0, 0, 0x050003F8, 1.0f, 23, 0, 0, -1, 0, 0x90, gPlSasukeFace0 },
    { 0x0300B3FC, 0, 0, 0x05000808, 1.0f, 24, 1, 1, -1, 0, 0x90, gPlSasukeFace0 },
    /* 30 */
    { 0x0300B3FC, 0, 0, 0x05000808, 1.0f, 24, 1, 1, -1, 0, 0x90, gPlSasukeFace0 },
    { 0x0300B9E8, 0, 0, 0,0.75f, 25, 2, -1, -1, 0, 0x80, gPlSasukeFace0 },
    { 0x0300B9E8, 0, 0, 0,0.75f, 25, 2, -1, -1, 0, 0x80, gPlSasukeFace0 },
    { 0x0300B9E8, 0, 0, 0,0.75f, 25, 2, -1, -1, 0, 0x80, gPlSasukeFace0 },
    { 0x0300C0A4, 0, 0, 0,0.8f, 26, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x0300C748, 0, 0, 0,0.8f, 27, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x0300CE7C, 0, 0, 0,0.4f, 28, -1, -1, -1, 0, 0x1, gPlSasukeFace8 },
    { 0x0300D6C4, 0, 0, 0,2.0f, 29, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x0300D6C4, 0, 0, 0,-3.0f, 29, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x0300DBEC, 0, 0, 0,1.9f, 30, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    /* 40 */
    { 0x0300DBEC, 0, 0, 0,-1.9f, 30, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x0300E624, 0, 0, 0,0.5f, 31, -1, -1, -1, 0, 0x1, gPlSasukeFace8 },
    { 0x0300EEA0, 0, 0, 0,-3.1f, 32, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x0300FC34, 0, 0x04000260, 0,0.5f, 33, -1, -1, 5, 0, 0x4, gPlSasukeFace3 },
    { 0x03010634, 0, 0, 0,0.4f, 34, -1, -1, -1, 0, 0x0, gPlSasukeFace3 },
    { 0x0300FC34, 0, 0, 0,0.0f, 33, -1, -1, 5, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.0f, 1, -1, -1, 0, 0, 0x8, gPlSasukeFace0 },
    { 0x03010CA8, 0, 0, 0,0.7f, 35, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03010CA8, 0, 0, 0,0.9f, 35, -1, -1, -1, 0, 0x0, gPlSasukeFace3 },
    { 0x03010CA8, 0, 0, 0,1.1f, 35, -1, -1, -1, 0, 0x0, gPlSasukeFace3 },
    /* 50 */
    { 0x03011580, 0, 0, 0,1.4f, 36, -1, -1, -1, 0, 0x0, gPlSasukeFace3 },
    { 0x0301193C, 0, 0, 0,0.4f, 37, -1, -1, -1, 0, 0x0, gPlSasukeFace3 },
    { 0x03012444, 0, 0, 0x05000BFC, 1.0f, 38, 3, 2, -1, 0, 0xB0, gPlSasukeFace4 },
    { 0x03012A48, 0, 0, 0x05000E38, 1.0f, 39, 4, 3, -1, 0, 0xB0, gPlSasukeFace4 },
    { 0x03012FE4, 0, 0, 0,1.0f, 40, 5, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03014234, 0, 0, 0x05001230, 1.0f, 42, 7, 4, -1, 0, 0xB0, gPlSasukeFace4 },
    { 0x03014838, 0, 0, 0x05001468, 0.8f, 43, 8, 5, -1, 0, 0xB0, gPlSasukeFace4 },
    { 0x03012FE4, 0, 0, 0,1.0f, 40, 5, -1, -1, 0, 0x80, gPlSasukeFace8 },
    /* 60 */
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03014EE0, 0, 0, 0x050016E0, 0.9f, 44, 9, 6, -1, 0, 0x90, gPlSasukeFace4 },
    { 0x030155CC, 0, 0, 0x05001960, 0.7f, 45, 10, 7, -1, 0, 0x90, gPlSasukeFace4 },
    { 0x03012FE4, 0, 0, 0,1.0f, 40, 5, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03013728, 0, 0, 0,1.0f, 41, 6, -1, -1, 0, 0x80, gPlSasukeFace8 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    /* 70 */
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    /* 80 */
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,-0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,-0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03015A84, 0, 0, 0,0.3f, 46, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x03015EA8, 0x03015EB4, 0, 0,0.3f, 47, -1, -1, -1, 0, 0x2, gPlSasukeFace4 },
    { 0x030160BC, 0x030160C8, 0, 0,0.4f, 48, -1, -1, -1, 0, 0x3, gPlSasukeFace4 },
    { 0x03016400, 0x0301640C, 0, 0,0.3f, 49, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x03016B4C, 0x03016B58, 0, 0x05001D64, 0.9f, 50, 11, 8, -1, 0, 0xB2, gPlSasukeFace4 },
    { 0x03017210, 0x0301721C, 0, 0x05002120, 0.9f, 51, 12, 9, -1, 0, 0xB2, gPlSasukeFace4 },
    { 0x03017210, 0x0301721C, 0, 0x05002120, 0.9f, 51, 12, 9, -1, 0, 0xB2, gPlSasukeFace4 },
    /* 90 */
    { 0x030176D0, 0x030176DC, 0, 0,0.8f, 52, -1, -1, -1, 0, 0x2, gPlSasukeFace4 },
    { 0x03017BA8, 0x03017BB4, 0, 0,0.8f, 53, -1, -1, -1, 0, 0x2, gPlSasukeFace4 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    /* 100 */
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x030183AC, 0, 0, 0,1.0f, 54, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x0301872C, 0, 0, 0,0.1f, 55, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03018DB0, 0, 0, 0,0.1f, 56, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    /* 110 */
    { 0x03019528, 0, 0, 0,0.1f, 57, -1, -1, -1, 0, 0x1, gPlSasukeFace6 },
    { 0x03019FCC, 0, 0, 0,0.1f, 58, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0301ABEC, 0, 0, 0,0.3f, 59, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x0301B6D4, 0, 0, 0,0.2f, 60, -1, -1, -1, 0, 0x1, gPlSasukeFace10 },
    { 0x0301BFC4, 0, 0, 0,1.3f, 61, -1, -1, -1, 0, 0x0, gPlSasukeFace4 },
    { 0x0301D238, 0, 0, 0,0.25f, 62, -1, -1, -1, 0, 0x0, gPlSasukeFace10 },
    { 0x0301D688, 0, 0, 0,0.3f, 63, -1, -1, -1, 0, 0x1, gPlSasukeFace10 },
    { 0x0301DFAC, 0, 0, 0,0.8f, 64, -1, -1, -1, 0, 0x0, gPlSasukeFace1 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x0, gPlSasukeFace0 },
    /* 120 */
    { 0x03001178, 0, 0, 0,0.0f, 1, -1, -1, 0, 0, 0x1, gPlSasukeFace0 },
    { 0x0301E760, 0, 0x04000280, 0,0.8f, 65, -1, -1, 6, 0, 0x5, gPlSasukeFace5 },
    { 0x0301EF7C, 0, 0, 0,0.6f, 66, -1, -1, -1, 0, 0x1, gPlSasukeFace6 },
    { 0x0301F640, 0, 0, 0,0.6f, 67, -1, -1, -1, 0, 0x1, gPlSasukeFace6 },
    { 0x0301FCF8, 0, 0, 0,0.6f, 68, -1, -1, -1, 0, 0x1, gPlSasukeFace6 },
    { 0x0302059C, 0, 0, 0,0.6f, 69, -1, -1, -1, 0, 0x1, gPlSasukeFace6 },
    { 0x03020D34, 0, 0, 0,0.8f, 70, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x030211AC, 0, 0, 0,0.8f, 71, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03021990, 0, 0, 0,1.0f, 72, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03022104, 0, 0, 0,0.8f, 73, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    /* 130 */
    { 0x030225AC, 0, 0, 0,0.8f, 74, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03020D34, 0, 0, 0,0.8f, 70, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x030211AC, 0, 0, 0,0.6f, 71, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03021990, 0, 0, 0,0.7f, 72, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03022104, 0, 0, 0,0.6f, 73, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x030225AC, 0, 0, 0,0.6f, 74, -1, -1, -1, 0, 0x0, gPlSasukeFace8 },
    { 0x03015A84, 0, 0, 0,0.8f, 46, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03022B9C, 0x03022BA8, 0, 0,0.1f, 75, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302319C, 0x030231A8, 0, 0,0.1f, 76, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302379C, 0x030237A8, 0, 0,0.1f, 77, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    /* 140 */
    { 0x0302379C, 0x030237A8, 0, 0,0.1f, 77, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302435C, 0x03024368, 0x040002A8, 0,0.4f, 78, -1, -1, 7, 0, 0x7, gPlSasukeFace0 },
    { 0x03024F20, 0x03024F2C, 0x040002D8, 0,0.4f, 79, -1, -1, 8, 0, 0x7, gPlSasukeFace0 },
    { 0x03025AE0, 0x03025AEC, 0x04000308, 0,0.4f, 80, -1, -1, 9, 0, 0x7, gPlSasukeFace0 },
    { 0x03025AE0, 0x03025AEC, 0x04000308, 0,0.4f, 80, -1, -1, 9, 0, 0x7, gPlSasukeFace0 },
    { 0x0302637C, 0x03026388, 0x04000338, 0,0.6f, 81, -1, -1, 10, 0, 0x7, gPlSasukeFace0 },
    { 0x03026C18, 0x03026C24, 0x04000368, 0,0.6f, 82, -1, -1, 11, 0, 0x7, gPlSasukeFace0 },
    { 0x030274A8, 0x030274B4, 0x04000398, 0,0.6f, 83, -1, -1, 12, 0, 0x7, gPlSasukeFace0 },
    { 0x030274A8, 0x030274B4, 0x04000398, 0,0.6f, 83, -1, -1, 12, 0, 0x7, gPlSasukeFace0 },
    { 0x030280B0, 0x030280BC, 0x040003C8, 0,0.8f, 84, -1, -1, 13, 0, 0x7, gPlSasukeFace0 },
    /* 150 */
    { 0x03028CB4, 0x03028CC0, 0x040003F8, 0,0.8f, 85, -1, -1, 14, 0, 0x7, gPlSasukeFace0 },
    { 0x030298B4, 0x030298C0, 0x04000428, 0,0.8f, 86, -1, -1, 15, 0, 0x7, gPlSasukeFace0 },
    { 0x030298B4, 0x030298C0, 0x04000428, 0,0.8f, 86, -1, -1, 15, 0, 0x7, gPlSasukeFace0 },
    { 0x0302A29C, 0x0302A2A8, 0x04000458, 0,1.2f, 87, -1, -1, 16, 0, 0x7, gPlSasukeFace0 },
    { 0x0302AC8C, 0x0302AC98, 0x04000488, 0,1.2f, 88, -1, -1, 17, 0, 0x7, gPlSasukeFace0 },
    { 0x0302B67C, 0x0302B688, 0x040004B8, 0,1.2f, 89, -1, -1, 18, 0, 0x7, gPlSasukeFace0 },
    { 0x0302B67C, 0x0302B688, 0x040004B8, 0,1.2f, 89, -1, -1, 18, 0, 0x7, gPlSasukeFace0 },
    { 0x0302BE74, 0x0302BE80, 0, 0,1.2f, 90, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302C664, 0x0302C670, 0, 0,1.2f, 91, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302CE54, 0x0302CE60, 0, 0,1.2f, 92, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    /* 160 */
    { 0x0302CE54, 0x0302CE60, 0, 0,1.2f, 92, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302D2B8, 0x0302D2C4, 0, 0,1.2f, 93, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302D718, 0x0302D724, 0, 0,1.2f, 94, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302DB78, 0x0302DB84, 0, 0,1.2f, 95, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302DB78, 0x0302DB84, 0, 0,1.2f, 95, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302DFC4, 0x0302DFD0, 0, 0,1.2f, 96, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302E414, 0x0302E420, 0, 0,1.2f, 97, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302E864, 0x0302E870, 0, 0,1.2f, 98, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x0302E864, 0x0302E870, 0, 0,1.2f, 98, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    { 0x03022B9C, 0x03022BA8, 0, 0,1.2f, 75, -1, -1, -1, 0, 0x2, gPlSasukeFace0 },
    /* 170 */
    { 0x0302319C, 0x030231A8, 0, 0,1.2f, 76, -1, -1, -1, 0, 0x2, gPlSasukeFace0 },
    { 0x0302379C, 0x030237A8, 0, 0,1.2f, 77, -1, -1, -1, 0, 0x2, gPlSasukeFace0 },
    { 0x0302379C, 0x030237A8, 0, 0,1.2f, 77, -1, -1, -1, 0, 0x2, gPlSasukeFace0 },
    { 0x0302EE00, 0, 0, 0,0.8f, 99, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302F5B0, 0, 0, 0,0.8f, 100, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302F5B0, 0, 0, 0,1.2f, 100, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302FB18, 0, 0, 0,0.8f, 101, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302FB18, 0, 0, 0,1.2f, 101, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302EE00, 0, 0, 0,0.8f, 99, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302EE00, 0, 0, 0,0.8f, 99, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    /* 180 */
    { 0x0302EE00, 0, 0, 0,0.8f, 99, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.8f, 1, -1, -1, 0, 0, 0x8, gPlSasukeFace0 },
    { 0x030303F0, 0, 0, 0x0500232C, 1.3f, 102, 13, 10, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03030A54, 0, 0, 0,1.1f, 103, 14, -1, -1, 0, 0x80, gPlSasukeFace0 },
    { 0x03031438, 0, 0, 0x05002554, 1.2f, 104, 15, 11, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03031C30, 0, 0, 0x05002740, 1.3f, 105, 16, 12, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03030A54, 0, 0, 0,1.1f, 103, 14, -1, -1, 0, 0x80, gPlSasukeFace0 },
    { 0x03032558, 0, 0, 0x0500293C, 1.2f, 106, 17, 13, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03031C30, 0, 0, 0x05002740, 1.3f, 105, 16, 12, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03030A54, 0, 0, 0,1.1f, 103, 14, -1, -1, 0, 0x80, gPlSasukeFace0 },
    /* 190 */
    { 0x03032558, 0, 0, 0x0500293C, 1.2f, 106, 17, 13, -1, 0, 0xB0, gPlSasukeFace0 },
    { 0x03032C64, 0, 0, 0,0.8f, 107, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03032C64, 0, 0, 0,0.8f, 107, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03032C64, 0, 0, 0,0.8f, 107, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03022B9C, 0, 0, 0,0.1f, 75, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x0302435C, 0, 0x040002A8, 0,0.4f, 78, -1, -1, 7, 0, 0x5, gPlSasukeFace0 },
    { 0x0302637C, 0, 0x04000338, 0,0.6f, 81, -1, -1, 10, 0, 0x5, gPlSasukeFace0 },
    { 0x030280B0, 0, 0x040003C8, 0,1.2f, 84, -1, -1, 13, 0, 0x5, gPlSasukeFace0 },
    { 0x0302A29C, 0, 0x04000458, 0,1.2f, 87, -1, -1, 16, 0, 0x5, gPlSasukeFace0 },
    { 0x0302379C, 0x030237A8, 0, 0,0.1f, 77, -1, -1, -1, 0, 0x3, gPlSasukeFace0 },
    /* 200 */
    { 0x03025AE0, 0x03025AEC, 0x04000308, 0,0.4f, 80, -1, -1, 9, 0, 0x7, gPlSasukeFace0 },
    { 0x030274A8, 0x030274B4, 0x04000398, 0,0.6f, 83, -1, -1, 12, 0, 0x7, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.4f, 0, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03032EF4, 0, 0, 0,0.15f, 108, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03033450, 0, 0, 0,0.9f, 109, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03033EC4, 0, 0, 0,1.2f, 110, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03034158, 0, 0, 0,0.2f, 111, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x030342D8, 0, 0, 0,0.2f, 112, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x030344A8, 0, 0, 0,0.2f, 113, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03034664, 0, 0, 0,0.3f, 114, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    /* 210 */
    { 0x03034814, 0, 0, 0,0.15f, 115, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03034D10, 0, 0, 0,2.0f, 116, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03034FF0, 0, 0, 0,0.8f, 117, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03006A64, 0, 0, 0,4.0f, 14, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.4f, 0, -1, -1, -1, 0, 0x9, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.4f, 0, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.2f, 0, -1, -1, -1, 0, 0x1, gPlSasukeFace9 },
    { 0x03001178, 0, 0x04000178, 0,0.4f, 1, -1, -1, 0, 0, 0x5, gPlSasukeFace0 },
    { 0x03001AF4, 0, 0x040001A8, 0,0.6f, 2, -1, -1, 1, 0, 0x5, gPlSasukeFace0 },
    { 0x030024EC, 0, 0x040001D8, 0,0.6f, 3, -1, -1, 2, 0, 0x5, gPlSasukeFace0 },
    /* 220 */
    { 0x03002EE4, 0, 0x04000208, 0,1.4f, 4, -1, -1, 3, 0, 0x5, gPlSasukeFace0 },
    { 0x03006A64, 0, 0, 0,4.0f, 14, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03006DF4, 0, 0, 0,2.0f, 15, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x0300717C, 0, 0, 0,1.2f, 16, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03007744, 0, 0, 0,0.2f, 17, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03007C68, 0, 0, 0,0.6f, 18, -1, -1, -1, 0, 0x0, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x8, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x8, gPlSasukeFace0 },
    { 0x03035564, 0, 0, 0,0.25f, 118, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03035B0C, 0, 0, 0,0.25f, 119, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    /* 230 */
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x8, gPlSasukeFace0 },
    { 0x03036270, 0, 0, 0,0.25f, 120, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x030369AC, 0, 0, 0,0.25f, 121, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.25f, 0, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x030370D8, 0, 0, 0,0.25f, 122, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x030374D8, 0, 0, 0,1.0f, 123, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03037C44, 0, 0, 0,1.0f, 124, -1, -1, -1, 0, 0x61, gPlSasukeFace0 },
    { 0x030383B0, 0, 0, 0,0.25f, 125, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x030387E8, 0, 0, 0,0.25f, 126, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03038F54, 0, 0, 0,0.25f, 127, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    /* 240 */
    { 0x03039688, 0, 0, 0,0.25f, 128, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03039DC8, 0, 0, 0,0.25f, 129, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    /* 250 */
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    /* 260 */
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303A530, 0, 0, 0,1.0f, 130, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303A7B8, 0, 0, 0,1.0f, 131, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303AA0C, 0, 0, 0,1.0f, 132, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303A530, 0, 0, 0,1.0f, 130, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303AD40, 0, 0, 0,1.0f, 133, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303B4A0, 0, 0, 0,1.0f, 134, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    /* 270 */
    { 0x0303B4A0, 0, 0, 0,1.0f, 134, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303B954, 0, 0, 0,1.0f, 135, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303BE90, 0, 0, 0,1.0f, 136, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303C35C, 0, 0, 0,1.0f, 137, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    { 0x0303CAC8, 0, 0, 0,1.0f, 138, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303CCC4, 0, 0, 0,1.0f, 139, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303D67C, 0, 0, 0,1.0f, 140, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    /* 280 */
    { 0x0303DE80, 0, 0, 0,1.0f, 141, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    { 0x0303E88C, 0, 0, 0,1.0f, 142, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    { 0x0303EA84, 0, 0, 0,1.0f, 143, -1, -1, -1, 0, 0x40, gPlSasukeFace0 },
    { 0x0303ECA0, 0, 0, 0,1.0f, 144, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x0303F1E4, 0, 0, 0,1.0f, 145, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303F588, 0, 0, 0,1.0f, 146, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,1.0f, 1, -1, -1, 0, 0, 0x40, gPlSasukeFace0 },
    { 0x0303F780, 0, 0, 0,1.0f, 147, -1, -1, -1, 0, 0x60, gPlSasukeFace0 },
    /* 290 */
    { 0x03002EE4, 0, 0x04000208, 0,1.4f, 4, -1, -1, 3, 0, 0x5, gPlSasukeFace0 },
    { 0x0303FE50, 0, 0, 0,0.6f, 148, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03000758, 0, 0, 0,0.4f, 0, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    /* 300 */
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    /* 310 */
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x41, gPlSasukeFace0 },
    { 0x03001178, 0, 0, 0,0.6f, 1, -1, -1, 0, 0, 0x1, gPlSasukeFace0 },
    { 0x03040028, 0, 0, 0,1.0f, 149, -1, -1, -1, 0, 0x1, gPlSasukeFace0 },
};

/* The attack boxes (segment 0x0B), by PlActAnim.atk. */
u32 gPlSasukeAtkBoxTable[] = {
    0x0B0002F8, 0x0B000608, 0x0B0007F4, 0x0B000ADC, 0x0B000C54, 0x0B000E50,
    0x0B00103C, 0x0B00132C, 0x0B0014A4, 0x0B001644, 0x0B0017F0, 0x0B001AF8,
    0x0B001DCC, 0x0B001F54, 0x0B002124, 0x0B0022B8, 0x0B00242C, 0x0B0025A0,
};

/* The four event sounds of each model set (Player 0x205), for plsound.c plAnimEventSounds. */
u16 gPlSasukeEventSeTable[][4] = {
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0526, 0x0527, 0x0526, 0x0527 }, { 0x021B, 0x021B, 0x021B, 0x021B },
    { 0x03AA, 0, 0, 0 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
    { 0x0500, 0x0211, 0x0243, 0x0243 }, { 0x0500, 0x0211, 0x0243, 0x0243 },
};
