/*
 OFX CImgErode plugin.

 Copyright (C) 2014 INRIA

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 Neither the name of the {organization} nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 INRIA
 Domaine de Voluceau
 Rocquencourt - B.P. 105
 78153 Le Chesnay Cedex - France


 The skeleton for this source file is from:
 OFX Basic Example plugin, a plugin that illustrates the use of the OFX Support library.

 Copyright (C) 2004-2005 The Open Effects Association Ltd
 Author Bruno Nicoletti bruno@thefoundry.co.uk

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name The Open Effects Association Ltd, nor the names of its
 contributors may be used to endorse or promote products derived from this
 software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The Open Effects Association Ltd
 1 Wardour St
 London W1D 6PA
 England

 */

#include "CImgErode.h"

#include <memory>
#include <cmath>
#include <cstring>
#ifdef _WINDOWS
#include <windows.h>
#endif

#include "ofxsProcessing.H"
#include "ofxsMaskMix.h"
#include "ofxsMacros.h"
#include "ofxsMerging.h"
#include "ofxsCopier.h"

#define cimg_display 0
#include <CImg.h>

#include "CImgFilter.h"

#define kPluginName          "ErodeCImg"
#define kPluginGrouping      "Filter"
#define kPluginDescription \
"Erode input stream by a rectangular structuring element of specified size and Neumann boundary conditions.\n" \
"Uses the 'erode' function from the CImg library.\n" \
"CImg is a free, open-source library distributed under the CeCILL-C " \
"(close to the GNU LGPL) or CeCILL (compatible with the GNU GPL) licenses. " \
"It can be used in commercial applications (see http://cimg.sourceforge.net)."

#define kPluginIdentifier    "net.sf.cimg.CImgErode"
#define kPluginVersionMajor 1 // Incrementing this number means that you have broken backwards compatibility of the plug-in.
#define kPluginVersionMinor 0 // Increment this when you have fixed a bug or made it faster.

#define kSupportsTiles 1
#define kSupportsMultiResolution 1
#define kSupportsRenderScale 1
#define kRenderThreadSafety eRenderFullySafe
#define kHostFrameThreading true
#define kSupportsRGBA true
#define kSupportsRGB true
#define kSupportsAlpha true

#define kParamSize "size"
#define kParamSizeLabel "size"
#define kParamSizeHint "Size of the rectangular structuring element, in pixel units (>=0)."
#define kParamSizeDefault 1


using namespace OFX;

/// Erode plugin
struct CImgErodeParams
{
    int sx;
    int sy;
};

class CImgErodePlugin : public CImgFilterPluginHelper<CImgErodeParams>
{
public:

    CImgErodePlugin(OfxImageEffectHandle handle)
    : CImgFilterPluginHelper<CImgErodeParams>(handle, kSupportsRenderScale)
    {
        _size  = fetchInt2DParam(kParamSize);
        assert(_size);
    }

    virtual void getValuesAtTime(double time, CImgErodeParams& params) OVERRIDE FINAL
    {
        _size->getValueAtTime(time, params.sx, params.sy);
    }

    // compute the roi required to compute rect, given params. This roi is then intersected with the image rod.
    // only called if mix != 0.
    virtual void getRoI(const OfxRectI rect, const OfxPointD& renderScale, const CImgErodeParams& params, OfxRectI* roi) OVERRIDE FINAL
    {
        int delta_pix_x = std::ceil(params.sx * renderScale.x);
        int delta_pix_y = std::ceil(params.sy * renderScale.y);
        roi->x1 = rect.x1 - delta_pix_x;
        roi->x2 = rect.x2 + delta_pix_x;
        roi->y1 = rect.y1 - delta_pix_y;
        roi->y2 = rect.y2 + delta_pix_y;
    }

    virtual void render(const OFX::RenderArguments &args, const CImgErodeParams& params, int x1, int y1, cimg_library::CImg<float>& cimg) OVERRIDE FINAL
    {
        // PROCESSING.
        // This is the only place where the actual processing takes place
        cimg.erode(std::ceil(params.sx * args.renderScale.x), std::ceil(params.sy * args.renderScale.y));
    }

    virtual bool isIdentity(const CImgErodeParams& params) OVERRIDE FINAL
    {
        return (params.sx == 0 && params.sy == 0);
    };

private:

    // params
    OFX::Int2DParam *_size;
};


mDeclarePluginFactory(CImgErodePluginFactory, {}, {});

void CImgErodePluginFactory::describe(OFX::ImageEffectDescriptor& desc)
{
    // basic labels
    desc.setLabels(kPluginName, kPluginName, kPluginName);
    desc.setPluginGrouping(kPluginGrouping);
    desc.setPluginDescription(kPluginDescription);

    // add supported context
    desc.addSupportedContext(eContextFilter);
    desc.addSupportedContext(eContextGeneral);

    // add supported pixel depths
    //desc.addSupportedBitDepth(eBitDepthUByte);
    //desc.addSupportedBitDepth(eBitDepthUShort);
    desc.addSupportedBitDepth(eBitDepthFloat);

    // set a few flags
    desc.setSingleInstance(false);
    desc.setHostFrameThreading(kHostFrameThreading);
    desc.setSupportsMultiResolution(kSupportsMultiResolution);
    desc.setSupportsTiles(kSupportsTiles);
    desc.setTemporalClipAccess(false);
    desc.setRenderTwiceAlways(true);
    desc.setSupportsMultipleClipPARs(false);
    desc.setRenderThreadSafety(kRenderThreadSafety);
}

void CImgErodePluginFactory::describeInContext(OFX::ImageEffectDescriptor& desc, OFX::ContextEnum context)
{
    // create the clips and params
    OFX::PageParamDescriptor *page = CImgErodePlugin::describeInContextBegin(desc, context,
                                                                              kSupportsRGBA,
                                                                              kSupportsRGB,
                                                                              kSupportsAlpha,
                                                                              kSupportsTiles);

    {
        OFX::Int2DParamDescriptor *param = desc.defineInt2DParam(kParamSize);
        param->setLabels(kParamSizeLabel, kParamSizeLabel, kParamSizeLabel);
        param->setHint(kParamSizeHint);
        param->setRange(0, 0, 1000, 1000);
        param->setDisplayRange(0, 0, 100, 100);
        param->setDefault(kParamSizeDefault, kParamSizeDefault);
        page->addChild(*param);
    }

    CImgErodePlugin::describeInContextEnd(desc, context, page);
}

OFX::ImageEffect* CImgErodePluginFactory::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum context)
{
    return new CImgErodePlugin(handle);
}


void getCImgErodePluginID(OFX::PluginFactoryArray &ids)
{
    static CImgErodePluginFactory p(kPluginIdentifier, kPluginVersionMajor, kPluginVersionMinor);
    ids.push_back(&p);
}
