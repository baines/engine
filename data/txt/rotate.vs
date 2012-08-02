!!ARBvp1.0
TEMP R0;
TEMP R2;
TEMP R3;
TEMP R4;
TEMP R5;

ATTRIB r = vertex.attrib[1];
ATTRIB pos = vertex.attrib[0];
ATTRIB tx = vertex.attrib[8];

ADD R5.x, pos.x, r.x;
ADD R5.y, pos.y, r.y;

MOV R3.x, r.w;
MOV R3.y, -r.z;

MOV R4.x, r.z;
MOV R4.y, r.w;

MAD R0, r.w, -R5.x, R5.x;
MAD R3.z, r.z,  R5.y, R0;

MAD R0, r.z, -R5.x, R5.y;
MAD R4.z, r.w, -R5.y, R0;

MOV R2, pos;
MOV R2.z, 1;

DP3 R0.x, R3, R2;
DP3 R0.y, R4, R2;
MOV R0.z, 1;
MOV R0.w, pos.w;

DP4 R2.x, state.matrix.mvp.row[0], R0;
DP4 R2.y, state.matrix.mvp.row[1], R0;
DP4 R2.z, state.matrix.mvp.row[2], R0;
DP4 R2.w, state.matrix.mvp.row[3], R0;
MOV result.position, R2;
MUL result.texcoord[0], tx, 0.0078125;
END
