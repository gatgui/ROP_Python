/*
MIT License

Copyright (c) 2014 Gaetan Guidet

This file is part of ROP_Python.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __ROP_Python_h__
#define __ROP_Python_h__

class ROP_Python : public ROP_Node
{
public:
    
    enum CallTiming
    {
        FirstFrame = 0,
        LastFrame
    };

    ROP_Python(OP_Network* net, const char* name, OP_Operator* op);
    virtual ~ROP_Python();
    
    //virtual unsigned int disableParams();
    virtual int startRender(int nframes, fpreal s, fpreal e);
    virtual ROP_RENDER_CODE renderFrame(fpreal time, UT_Interrupt* boss=0);
    virtual ROP_RENDER_CODE endRender();

private:

    bool mPerFrame;
    UT_String mScript;
    fpreal mLastRenderedTime;
    CallTiming mSingleCallTiming;

public:

    static PRM_SpareData PythonEditor;
    static PRM_Name ParameterNames[];
    static PRM_Default ParameterDefaults[];
    static PRM_Template Parameters[];
    static CH_LocalVariable Variables[];
    static PRM_Name TimingNames[];
    static PRM_ChoiceList TimingChoiceList;
    static PRM_Conditional TimingCondition;
    
    static OP_Node* Create(OP_Network *net, const char *name, OP_Operator *op);
};

#endif
