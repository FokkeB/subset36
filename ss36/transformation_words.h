/**
* This file is part of "balise_codec".
* balise_codec is free software: you can distribute it and/or modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* See the GNU Lesser General Public License for more details.
* You should have received a copy of the GNU General Public License along with this program.
* If not, see < https://www.gnu.org/licenses/>.
*/

#ifndef TRANS_H
#define TRANS_H

// prototype of a lookup-function:
int find11(int val11);

// words used to perform the transformation described in Subset 36 (see ss36.c).
// copied directly from subset 36, appendix B2: The 10-to-11 bit Transformation Substitution Words.

#define N_TRANS_WORDS   1024
#define FIRST_TW_001    104 // index of first transformation word starting with 001
#define LAST_TW_001     260 // index of last transformation word starting with 001

static unsigned short int transformation_words[N_TRANS_WORDS] = {
00101, 00102, 00103, 00104, 00105, 00106, 00107, 00110, 00111, 00112,
00113, 00114, 00115, 00116, 00117, 00120, 00121, 00122, 00123, 00124,
00125, 00126, 00127, 00130, 00131, 00132, 00133, 00134, 00135, 00141,
00142, 00143, 00144, 00145, 00146, 00147, 00150, 00151, 00152, 00153,
00154, 00155, 00156, 00157, 00160, 00161, 00162, 00163, 00164, 00165,
00166, 00167, 00170, 00171, 00172, 00173, 00174, 00175, 00176, 00201,
00206, 00211, 00214, 00216, 00217, 00220, 00222, 00223, 00224, 00225,
00226, 00231, 00233, 00244, 00245, 00246, 00253, 00257, 00260, 00261,
00272, 00273, 00274, 00275, 00276, 00301, 00303, 00315, 00317, 00320,
00321, 00332, 00334, 00341, 00342, 00343, 00344, 00346, 00352, 00353,
00357, 00360, 00374, 00376, 00401, 00403, 00404, 00405, 00406, 00407,   // word#104=00401 is first word to start with 0b001, see set_next_sb_esb in ss36.c
00410, 00411, 00412, 00413, 00416, 00417, 00420, 00424, 00425, 00426,
00427, 00432, 00433, 00442, 00443, 00445, 00456, 00457, 00460, 00461,
00464, 00465, 00470, 00471, 00472, 00474, 00475, 00476, 00501, 00502,
00503, 00504, 00505, 00506, 00507, 00516, 00517, 00520, 00521, 00522,
00523, 00524, 00525, 00530, 00531, 00532, 00533, 00534, 00535, 00544,
00545, 00546, 00547, 00550, 00551, 00552, 00553, 00554, 00555, 00556,
00557, 00560, 00561, 00562, 00563, 00571, 00573, 00576, 00601, 00602,
00604, 00605, 00610, 00611, 00612, 00613, 00614, 00615, 00616, 00617,
00620, 00621, 00622, 00623, 00624, 00625, 00626, 00627, 00630, 00634,
00635, 00644, 00645, 00646, 00647, 00650, 00651, 00652, 00653, 00654,
00655, 00656, 00657, 00660, 00661, 00662, 00663, 00666, 00667, 00672,
00674, 00675, 00676, 00701, 00712, 00713, 00716, 00717, 00720, 00721,
00722, 00723, 00730, 00731, 00732, 00733, 00734, 00735, 00742, 00743,
00744, 00745, 00746, 00747, 00750, 00751, 00752, 00753, 00754, 00755,
00756, 00757, 00760, 00761, 00764, 00765, 00766, 00767, 00772, 00773,
00776, 01001, 01004, 01005, 01016, 01017, 01020, 01021, 01022, 01023,   // word#260=00776 is last word to start with 0b001, see set_next_sb_esb in ss36.c
01024, 01025, 01030, 01031, 01032, 01033, 01034, 01035, 01043, 01044,
01045, 01046, 01047, 01054, 01057, 01060, 01061, 01062, 01075, 01076,
01101, 01102, 01103, 01110, 01114, 01115, 01116, 01117, 01120, 01121,
01122, 01123, 01124, 01125, 01126, 01127, 01130, 01131, 01132, 01133,
01142, 01143, 01144, 01145, 01146, 01147, 01151, 01152, 01153, 01154,
01155, 01156, 01157, 01160, 01164, 01166, 01167, 01176, 01201, 01214,
01217, 01220, 01221, 01222, 01223, 01224, 01225, 01226, 01227, 01230,
01231, 01232, 01233, 01243, 01244, 01245, 01253, 01254, 01255, 01256,
01257, 01260, 01261, 01272, 01273, 01274, 01275, 01276, 01301, 01302,
01303, 01305, 01306, 01307, 01317, 01320, 01321, 01332, 01334, 01335,
01342, 01343, 01344, 01345, 01350, 01351, 01352, 01353, 01355, 01356,
01357, 01360, 01361, 01364, 01365, 01370, 01371, 01372, 01373, 01374,
01376, 01401, 01403, 01406, 01407, 01414, 01415, 01416, 01417, 01420,
01424, 01425, 01431, 01433, 01434, 01435, 01443, 01445, 01456, 01457,
01460, 01462, 01474, 01475, 01476, 01501, 01502, 01503, 01504, 01505,
01516, 01517, 01520, 01524, 01532, 01533, 01544, 01546, 01550, 01551,
01552, 01553, 01554, 01557, 01560, 01561, 01562, 01563, 01566, 01567,
01576, 01601, 01603, 01604, 01605, 01606, 01607, 01610, 01611, 01612,
01613, 01614, 01615, 01616, 01617, 01620, 01621, 01622, 01623, 01624,
01625, 01626, 01630, 01631, 01632, 01633, 01635, 01643, 01644, 01645,
01650, 01651, 01652, 01653, 01654, 01655, 01656, 01657, 01660, 01661,
01672, 01674, 01675, 01676, 01701, 01720, 01744, 01745, 01746, 01747,
01750, 01751, 01752, 01753, 01754, 01755, 01756, 01757, 01760, 01761,
01762, 01763, 01764, 01765, 01766, 01767, 01770, 01771, 01772, 01773,
01774, 01775, 02002, 02003, 02004, 02005, 02006, 02007, 02010, 02011,
02012, 02013, 02014, 02015, 02016, 02017, 02020, 02021, 02022, 02023,
02024, 02025, 02026, 02027, 02030, 02031, 02032, 02033, 02057, 02076,
02101, 02102, 02103, 02105, 02116, 02117, 02120, 02121, 02122, 02123,
02124, 02125, 02126, 02127, 02132, 02133, 02134, 02142, 02144, 02145,
02146, 02147, 02151, 02152, 02153, 02154, 02155, 02156, 02157, 02160,
02161, 02162, 02163, 02164, 02165, 02166, 02167, 02170, 02171, 02172,
02173, 02174, 02176, 02201, 02210, 02211, 02214, 02215, 02216, 02217,
02220, 02223, 02224, 02225, 02226, 02227, 02231, 02233, 02244, 02245,
02253, 02257, 02260, 02261, 02272, 02273, 02274, 02275, 02276, 02301,
02302, 02303, 02315, 02317, 02320, 02321, 02332, 02334, 02342, 02343,
02344, 02346, 02352, 02353, 02357, 02360, 02361, 02362, 02363, 02370,
02371, 02374, 02376, 02401, 02403, 02404, 02405, 02406, 02407, 02412,
02413, 02416, 02417, 02420, 02421, 02422, 02424, 02425, 02426, 02427,
02432, 02433, 02434, 02435, 02442, 02443, 02445, 02456, 02457, 02460,
02470, 02471, 02472, 02474, 02475, 02476, 02501, 02502, 02503, 02504,
02505, 02516, 02517, 02520, 02521, 02522, 02523, 02524, 02532, 02533,
02534, 02544, 02545, 02546, 02547, 02550, 02551, 02552, 02553, 02554,
02555, 02556, 02557, 02560, 02563, 02576, 02601, 02610, 02611, 02613,
02617, 02620, 02621, 02622, 02623, 02624, 02625, 02626, 02630, 02631,
02632, 02633, 02634, 02635, 02644, 02645, 02646, 02647, 02650, 02651,
02652, 02653, 02654, 02655, 02656, 02657, 02660, 02661, 02662, 02663,
02667, 02674, 02675, 02676, 02701, 02702, 02715, 02716, 02717, 02720,
02723, 02730, 02731, 02732, 02733, 02734, 02742, 02743, 02744, 02745,
02746, 02747, 02752, 02753, 02754, 02755, 02756, 02757, 02760, 02761,
02772, 02773, 02776, 03001, 03004, 03005, 03010, 03011, 03012, 03013,
03016, 03017, 03020, 03021, 03022, 03023, 03024, 03025, 03026, 03027,
03030, 03031, 03032, 03033, 03034, 03035, 03042, 03043, 03044, 03045,
03046, 03047, 03054, 03055, 03056, 03057, 03060, 03061, 03064, 03065,
03076, 03101, 03102, 03103, 03105, 03110, 03111, 03114, 03115, 03116,
03117, 03120, 03121, 03122, 03123, 03124, 03125, 03126, 03127, 03130,
03131, 03132, 03133, 03142, 03143, 03147, 03150, 03151, 03152, 03153,
03154, 03155, 03156, 03157, 03160, 03161, 03162, 03163, 03164, 03165,
03166, 03167, 03172, 03173, 03175, 03176, 03201, 03204, 03206, 03214,
03215, 03216, 03217, 03220, 03221, 03222, 03223, 03224, 03225, 03226,
03227, 03230, 03231, 03232, 03233, 03242, 03243, 03244, 03245, 03246,
03247, 03252, 03253, 03254, 03255, 03256, 03257, 03260, 03261, 03270,
03271, 03272, 03273, 03274, 03275, 03276, 03301, 03302, 03303, 03305,
03306, 03307, 03312, 03313, 03316, 03317, 03320, 03321, 03332, 03334,
03335, 03344, 03345, 03350, 03351, 03352, 03353, 03357, 03360, 03361,
03364, 03365, 03366, 03367, 03370, 03371, 03372, 03373, 03374, 03376,
03401, 03403, 03417, 03420, 03424, 03425, 03431, 03433, 03434, 03435,
03436, 03443, 03445, 03456, 03457, 03460, 03462, 03474, 03476, 03501,
03502, 03503, 03504, 03505, 03516, 03517, 03520, 03524, 03531, 03532,
03533, 03544, 03546, 03551, 03552, 03553, 03554, 03555, 03557, 03560,
03561, 03563, 03566, 03571, 03576, 03601, 03602, 03603, 03604, 03605,
03606, 03607, 03610, 03611, 03612, 03613, 03614, 03615, 03616, 03617,
03620, 03621, 03622, 03623, 03624, 03625, 03626, 03627, 03630, 03631,
03632, 03633, 03634, 03635, 03636, 03642, 03643, 03644, 03645, 03646,
03647, 03650, 03651, 03652, 03653, 03654, 03655, 03656, 03657, 03660,
03661, 03662, 03663, 03664, 03665, 03666, 03667, 03670, 03671, 03672,
03673, 03674, 03675, 03676
};

#define N_LOOKUP_TABLE 2048 // 11 bits

// same table as transformation_words, but inverted (for faster lookups)
// non-existent transformation words are marked with -1
// existent transformation words are marked with their index in the transformation_words array.
static short int lookup_table[N_LOOKUP_TABLE] = {
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, -1, -1, -1, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
52, 53, 54, 55, 56, 57, 58, -1, -1, 59, -1, -1, -1, -1, 60, -1, -1, 61, -1, -1,
62, -1, 63, 64, 65, -1, 66, 67, 68, 69, 70, -1, -1, 71, -1, 72, -1, -1, -1, -1,
-1, -1, -1, -1, 73, 74, 75, -1, -1, -1, -1, 76, -1, -1, -1, 77, 78, 79, -1, -1,
-1, -1, -1, -1, -1, -1, 80, 81, 82, 83, 84, -1, -1, 85, -1, 86, -1, -1, -1, -1,
-1, -1, -1, -1, -1, 87, -1, 88, 89, 90, -1, -1, -1, -1, -1, -1, -1, -1, 91, -1,
92, -1, -1, -1, -1, 93, 94, 95, 96, -1, 97, -1, -1, -1, 98, 99, -1, -1, -1, 100,
101, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 102, -1, 103, -1, -1, 104, -1, 105,
106, 107, 108, 109, 110, 111, 112, 113, -1, -1, 114, 115, 116, -1, -1, -1, 117, 118, 119, 120,
-1, -1, 121, 122, -1, -1, -1, -1, -1, -1, 123, 124, -1, 125, -1, -1, -1, -1, -1, -1,
-1, -1, 126, 127, 128, 129, -1, -1, 130, 131, -1, -1, 132, 133, 134, -1, 135, 136, 137, -1,
-1, 138, 139, 140, 141, 142, 143, 144, -1, -1, -1, -1, -1, -1, 145, 146, 147, 148, 149, 150,
151, 152, -1, -1, 153, 154, 155, 156, 157, 158, -1, -1, -1, -1, -1, -1, 159, 160, 161, 162,
163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, -1, -1, -1, -1, -1, 175, -1, 176,
-1, -1, 177, -1, -1, 178, 179, -1, 180, 181, -1, -1, 182, 183, 184, 185, 186, 187, 188, 189,
190, 191, 192, 193, 194, 195, 196, 197, 198, -1, -1, -1, 199, 200, -1, -1, -1, -1, -1, -1,
201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, -1, -1, 217, 218,
-1, -1, 219, -1, 220, 221, 222, -1, -1, 223, -1, -1, -1, -1, -1, -1, -1, -1, 224, 225,
-1, -1, 226, 227, 228, 229, 230, 231, -1, -1, -1, -1, 232, 233, 234, 235, 236, 237, -1, -1,
-1, -1, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, -1, -1,
254, 255, 256, 257, -1, -1, 258, 259, -1, -1, 260, -1, -1, 261, -1, -1, 262, 263, -1, -1,
-1, -1, -1, -1, -1, -1, 264, 265, 266, 267, 268, 269, 270, 271, -1, -1, 272, 273, 274, 275,
276, 277, -1, -1, -1, -1, -1, 278, 279, 280, 281, 282, -1, -1, -1, -1, 283, -1, -1, 284,
285, 286, 287, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 288, 289, -1, -1, 290, 291, 292,
-1, -1, -1, -1, 293, -1, -1, -1, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305,
306, 307, 308, 309, -1, -1, -1, -1, -1, -1, 310, 311, 312, 313, 314, 315, -1, 316, 317, 318,
319, 320, 321, 322, 323, -1, -1, -1, 324, -1, 325, 326, -1, -1, -1, -1, -1, -1, 327, -1,
-1, 328, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 329, -1, -1, 330, 331, 332, 333, 334,
335, 336, 337, 338, 339, 340, 341, 342, -1, -1, -1, -1, -1, -1, -1, 343, 344, 345, -1, -1,
-1, -1, -1, 346, 347, 348, 349, 350, 351, 352, -1, -1, -1, -1, -1, -1, -1, -1, 353, 354,
355, 356, 357, -1, -1, 358, 359, 360, -1, 361, 362, 363, -1, -1, -1, -1, -1, -1, -1, 364,
365, 366, -1, -1, -1, -1, -1, -1, -1, -1, 367, -1, 368, 369, -1, -1, -1, -1, 370, 371,
372, 373, -1, -1, 374, 375, 376, 377, -1, 378, 379, 380, 381, 382, -1, -1, 383, 384, -1, -1,
385, 386, 387, 388, 389, -1, 390, -1, -1, 391, -1, 392, -1, -1, 393, 394, -1, -1, -1, -1,
395, 396, 397, 398, 399, -1, -1, -1, 400, 401, -1, -1, -1, 402, -1, 403, 404, 405, -1, -1,
-1, -1, -1, 406, -1, 407, -1, -1, -1, -1, -1, -1, -1, -1, 408, 409, 410, -1, 411, -1,
-1, -1, -1, -1, -1, -1, -1, -1, 412, 413, 414, -1, -1, 415, 416, 417, 418, 419, -1, -1,
-1, -1, -1, -1, -1, -1, 420, 421, 422, -1, -1, -1, 423, -1, -1, -1, -1, -1, 424, 425,
-1, -1, -1, -1, -1, -1, -1, -1, 426, -1, 427, -1, 428, 429, 430, 431, 432, -1, -1, 433,
434, 435, 436, 437, -1, -1, 438, 439, -1, -1, -1, -1, -1, -1, 440, -1, -1, 441, -1, 442,
443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, -1,
462, 463, 464, 465, -1, 466, -1, -1, -1, -1, -1, 467, 468, 469, -1, -1, 470, 471, 472, 473,
474, 475, 476, 477, 478, 479, -1, -1, -1, -1, -1, -1, -1, -1, 480, -1, 481, 482, 483, -1,
-1, 484, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 485, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 486, 487, 488, 489,
490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509,
510, 511, -1, -1, -1, -1, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525,
526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 538, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, 539, -1, -1, 540, 541, 542, -1, 543, -1, -1, -1, -1, -1, -1,
-1, -1, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, -1, -1, 554, 555, 556, -1, -1, -1,
-1, -1, 557, -1, 558, 559, 560, 561, -1, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572,
573, 574, 575, 576, 577, 578, 579, 580, 581, -1, 582, -1, -1, 583, -1, -1, -1, -1, -1, -1,
584, 585, -1, -1, 586, 587, 588, 589, 590, -1, -1, 591, 592, 593, 594, 595, -1, 596, -1, 597,
-1, -1, -1, -1, -1, -1, -1, -1, 598, 599, -1, -1, -1, -1, -1, 600, -1, -1, -1, 601,
602, 603, -1, -1, -1, -1, -1, -1, -1, -1, 604, 605, 606, 607, 608, -1, -1, 609, 610, 611,
-1, -1, -1, -1, -1, -1, -1, -1, -1, 612, -1, 613, 614, 615, -1, -1, -1, -1, -1, -1,
-1, -1, 616, -1, 617, -1, -1, -1, -1, -1, 618, 619, 620, -1, 621, -1, -1, -1, 622, 623,
-1, -1, -1, 624, 625, 626, 627, 628, -1, -1, -1, -1, 629, 630, -1, -1, 631, -1, 632, -1,
-1, 633, -1, 634, 635, 636, 637, 638, -1, -1, 639, 640, -1, -1, 641, 642, 643, 644, 645, -1,
646, 647, 648, 649, -1, -1, 650, 651, 652, 653, -1, -1, -1, -1, 654, 655, -1, 656, -1, -1,
-1, -1, -1, -1, -1, -1, 657, 658, 659, -1, -1, -1, -1, -1, -1, -1, 660, 661, 662, -1,
663, 664, 665, -1, -1, 666, 667, 668, 669, 670, -1, -1, -1, -1, -1, -1, -1, -1, 671, 672,
673, 674, 675, 676, 677, -1, -1, -1, -1, -1, 678, 679, 680, -1, -1, -1, -1, -1, -1, -1,
681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, -1, -1, 694, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, 695, -1, -1, 696, -1, -1, -1, -1, -1, -1, 697, 698, -1, 699,
-1, -1, -1, 700, 701, 702, 703, 704, 705, 706, 707, -1, 708, 709, 710, 711, 712, 713, -1, -1,
-1, -1, -1, -1, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729,
-1, -1, -1, 730, -1, -1, -1, -1, 731, 732, 733, -1, -1, 734, 735, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, 736, 737, 738, 739, -1, -1, 740, -1, -1, -1, -1, 741, 742, 743, 744,
745, -1, -1, -1, -1, -1, 746, 747, 748, 749, 750, 751, -1, -1, 752, 753, 754, 755, 756, 757,
758, 759, -1, -1, -1, -1, -1, -1, -1, -1, 760, 761, -1, -1, 762, -1, -1, 763, -1, -1,
764, 765, -1, -1, 766, 767, 768, 769, -1, -1, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779,
780, 781, 782, 783, 784, 785, -1, -1, -1, -1, 786, 787, 788, 789, 790, 791, -1, -1, -1, -1,
792, 793, 794, 795, 796, 797, -1, -1, 798, 799, -1, -1, -1, -1, -1, -1, -1, -1, 800, -1,
-1, 801, 802, 803, -1, 804, -1, -1, 805, 806, -1, -1, 807, 808, 809, 810, 811, 812, 813, 814,
815, 816, 817, 818, 819, 820, 821, 822, -1, -1, -1, -1, -1, -1, 823, 824, -1, -1, -1, 825,
826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, -1, -1, 842, 843,
-1, 844, 845, -1, -1, 846, -1, -1, 847, -1, 848, -1, -1, -1, -1, -1, 849, 850, 851, 852,
853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, -1, -1, -1, -1, -1, -1, 865, 866,
867, 868, 869, 870, -1, -1, 871, 872, 873, 874, 875, 876, 877, 878, -1, -1, -1, -1, -1, -1,
879, 880, 881, 882, 883, 884, 885, -1, -1, 886, 887, 888, -1, 889, 890, 891, -1, -1, 892, 893,
-1, -1, 894, 895, 896, 897, -1, -1, -1, -1, -1, -1, -1, -1, 898, -1, 899, 900, -1, -1,
-1, -1, -1, -1, 901, 902, -1, -1, 903, 904, 905, 906, -1, -1, -1, 907, 908, 909, -1, -1,
910, 911, 912, 913, 914, 915, 916, 917, 918, -1, 919, -1, -1, 920, -1, 921, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, 922, 923, -1, -1, -1, 924, 925, -1, -1, -1, 926, -1, 927,
928, 929, 930, -1, -1, -1, -1, 931, -1, 932, -1, -1, -1, -1, -1, -1, -1, -1, 933, 934,
935, -1, 936, -1, -1, -1, -1, -1, -1, -1, -1, -1, 937, -1, 938, -1, -1, 939, 940, 941,
942, 943, -1, -1, -1, -1, -1, -1, -1, -1, 944, 945, 946, -1, -1, -1, 947, -1, -1, -1,
-1, 948, 949, 950, -1, -1, -1, -1, -1, -1, -1, -1, 951, -1, 952, -1, -1, 953, 954, 955,
956, 957, -1, 958, 959, 960, -1, 961, -1, -1, 962, -1, -1, 963, -1, -1, -1, -1, 964, -1,
-1, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983,
984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, -1, -1, -1, 995, 996, 997, 998, 999, 1000,
1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020,
1021, 1022, 1023, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1
};

#endif