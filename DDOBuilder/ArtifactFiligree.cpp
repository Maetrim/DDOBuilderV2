// ArtifactFiligree.cpp
//
#include "StdAfx.h"
#include "ArtifactFiligree.h"

#define DL_ELEMENT ArtifactFiligree

namespace
{
    const wchar_t f_saxElementName[] = L"ArtifactFiligree";
    const unsigned f_verCurrent = 1;
}

ArtifactFiligree::ArtifactFiligree() :
    TrainedFiligree(f_saxElementName, f_verCurrent)
{
}
