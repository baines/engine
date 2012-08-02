!!ARBfp1.0
TEMP c;
TEX c, fragment.texcoord[0], texture[0], 2D;
MOV result.color, c;
SUB c, c.w, 0.5;
KIL c;
END
