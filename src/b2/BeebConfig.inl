//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ENAME BeebConfigFeatureFlag
EBEGIN()
EPNV(MasterTurbo, 1 << 0)
EPNV(6502SecondProcessor, 1 << 1)
EEND()
#undef ENAME

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define ENAME BeebConfigParasiteType
EBEGIN()
EPN(None)
EPN(MasterTurbo)
EPN(External3MHz6502)
EEND()
#undef ENAME

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
