#ifndef DFFACETEX_H
#define DFFACETEX_H

#include "../math/Vector3.h"

/* Local type for matrix conversion parameters */
struct df3duvparams_l
{
    float X[4], Y[4], Z[4];
    float U[4], V[4];
};


/* Used to convert XYZ point coordinates to DF UV coordinates */
struct df3duvmatrix
{
    float UA, UB, UC, UD;
    float VA, VB, VC, VD;
};

struct ObjVertex
{
    Vector3 pos;
    float tu;
    float tv;
};

// Class to wrap Dave Humnprey's UV code into its own namespace
class DFFaceTex
{
public:     // methods
    bool    ComputeDFFaceTextureUVMatrix( df3duvmatrix& Matrix, ObjVertex *pFaceVerts );

private:    // methods
    bool    l_ComputeDFUVMatrixXY(   df3duvmatrix& Matrix, const df3duvparams_l& Params );
    bool    l_ComputeDFUVMatrixXZ(   df3duvmatrix& Matrix, const df3duvparams_l& Params );
    bool    l_ComputeDFUVMatrixYZ(   df3duvmatrix& Matrix, const df3duvparams_l& Params );
    bool    l_ComputeDFUVMatrixXYZ(  df3duvmatrix& Matrix, const df3duvparams_l& Params );
    bool    l_ComputeDFUVMatrixXYZ1( df3duvmatrix& Matrix, const df3duvparams_l& Params );
};

#endif //DFFACETEX_H
