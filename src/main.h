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
