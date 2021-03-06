#include "rgbargbatohsv.h"
#include "hsv.h"

namespace anl
{
    CRGBARGBAToHSV::CRGBARGBAToHSV() : CRGBAModuleBase(), m_source(){}
    CRGBARGBAToHSV::CRGBARGBAToHSV(SRGBA s) : CRGBAModuleBase(), m_source(s){}
    CRGBARGBAToHSV::CRGBARGBAToHSV(CRGBAModuleBase * s) : CRGBAModuleBase(), m_source(s){}

    void CRGBARGBAToHSV::setSource(CRGBAModuleBase * m)
    {
        m_source.set(m);
    }
    void CRGBARGBAToHSV::setSource(SRGBA c)
    {
        m_source.set(c);
    }

    SRGBA CRGBARGBAToHSV::get(ANLFloatType x, ANLFloatType y)
    {
        SRGBA s=m_source.get(x,y);
        SRGBA d;
        RGBAtoHSV(s,d);
        return d;
    }
    SRGBA CRGBARGBAToHSV::get(ANLFloatType x, ANLFloatType y, ANLFloatType z)
    {
        SRGBA s=m_source.get(x,y,z);
        SRGBA d;
        RGBAtoHSV(s,d);
        return d;
    }
    SRGBA CRGBARGBAToHSV::get(ANLFloatType x, ANLFloatType y, ANLFloatType z, ANLFloatType w)
    {
        SRGBA s=m_source.get(x,y,z,w);
        SRGBA d;
        RGBAtoHSV(s,d);
        return d;
    }
    SRGBA CRGBARGBAToHSV::get(ANLFloatType x, ANLFloatType y, ANLFloatType z, ANLFloatType w, ANLFloatType u, ANLFloatType v)
    {
        SRGBA s=m_source.get(x,y,z,w,u,v);
        SRGBA d;
        RGBAtoHSV(s,d);
        return d;
    }
};
