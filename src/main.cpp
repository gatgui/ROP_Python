#include <iostream>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_String.h>
#include <UT/UT_Version.h>
#include <ROP/ROP_Node.h>
#include <ROP/ROP_Templates.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
#include <PRM/PRM_ChoiceList.h>
#include <PRM/PRM_Conditional.h>
#include <CH/CH_LocalVariable.h>
#include <OP/OP_OperatorTable.h>
#include <HOM/HOM_Module.h>
#include <string>
#include "main.h"

// ---

ROP_Python::ROP_Python(OP_Network* net, const char* name, OP_Operator* op)
    : ROP_Node(net, name, op)
    , mPerFrame(false)
    , mLastRenderedTime(0.0)
    , mSingleCallTiming(ROP_Python::LastFrame)
{
}

ROP_Python::~ROP_Python()
{
}

int ROP_Python::startRender(int nframes, fpreal s, fpreal e)
{
    #ifdef _DEBUG
    std::cout << getName() << " | ROP_Python::startRender " << nframes << ", " << s << ", " << e << std::endl;
    #endif
    
    UT_String script;
    
    bool perframe = (evalFloat("run_per_frame", 0, s) > 0.0);
    CallTiming timing = (CallTiming) int(evalFloat("timing", 0, s));
    evalString(script, "script", 0, s);
    
    if (mPerFrame != perframe || timing != mSingleCallTiming || script != mScript)
    {
        mPerFrame = perframe;
        mSingleCallTiming = timing;
        mScript = script;
        
        setString((!perframe && timing == FirstFrame ? mScript : UT_String()), CH_STRING_LITERAL, "prerender", 0, s);
        setString((perframe ? mScript : UT_String()), CH_STRING_LITERAL, "preframe", 0, s);
        setString((!perframe && timing == LastFrame ? mScript : UT_String()), CH_STRING_LITERAL, "postrender", 0, s);
    }
    
    return 1;
}

ROP_RENDER_CODE ROP_Python::renderFrame(fpreal time, UT_Interrupt* boss)
{
    #ifdef _DEBUG
    std::cout << getName() << " | ROP_Python::renderFrame " << time << std::endl;
    #endif
    
    mLastRenderedTime = time;
    
    if (mPerFrame)
    {
        executePreFrameScript(time);
    }
    else
    {
        if (mSingleCallTiming == FirstFrame)
        {
            OP_Context ctx;
            ctx.setFrame(FSTART());
            
            if (DORANGE() == 0 || time <= ctx.getTime())
            {
                executePreRenderScript(time);
            }
        }
        else if (mSingleCallTiming == LastFrame)
        {
            OP_Context ctx;
            ctx.setFrame(FEND());
            
            if (DORANGE() == 0 || time >= ctx.getTime())
            {
                executePostRenderScript(time);
            }
        }
    }
    
    return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE ROP_Python::endRender()
{
    #ifdef _DEBUG
    std::cout << getName() << " | ROP_Python::endRender" << std::endl;
    #endif
    
    return ROP_CONTINUE_RENDER;
}

PRM_Name ROP_Python::ParameterNames[] = {
    PRM_Name("script", "Python Script"),
    PRM_Name("run_per_frame", "Run Per Frame"),
    PRM_Name("timing", "Timing")
};

PRM_Default ROP_Python::ParameterDefaults[] = {
    PRM_Default(0.0f, ""),
    PRM_Default(0.0f, ""),
    PRM_Default(1.0f, "")
};


PRM_SpareData ROP_Python::PythonEditor;

PRM_Name ROP_Python::TimingNames[] = {
    PRM_Name("first", "First Frame"),
    PRM_Name("last", "Last Frame"),
    PRM_Name("", "")
};

PRM_ChoiceList ROP_Python::TimingChoiceList(PRM_CHOICELIST_SINGLE,  ROP_Python::TimingNames);

PRM_Conditional ROP_Python::TimingCondition("{ trange == off } { run_per_frame == 1 }", PRM_CONDTYPE_DISABLE);

PRM_Template ROP_Python::Parameters[] = {
    theRopTemplates[ROP_RENDER_TPLATE],
    theRopTemplates[ROP_RENDERDIALOG_TPLATE],
    theRopTemplates[ROP_TRANGE_TPLATE],
    theRopTemplates[ROP_FRAMERANGE_TPLATE],
    theRopTemplates[ROP_TAKENAME_TPLATE],
    PRM_Template(PRM_STRING, 1, &ROP_Python::ParameterNames[0], &ROP_Python::ParameterDefaults[0], 0, 0, 0, &ROP_Python::PythonEditor),
    PRM_Template(PRM_TOGGLE, 1, &ROP_Python::ParameterNames[1], &ROP_Python::ParameterDefaults[1]),
    PRM_Template(PRM_TYPE_ORDINAL, 1, &ROP_Python::ParameterNames[2], &ROP_Python::ParameterDefaults[2], &ROP_Python::TimingChoiceList, 0, 0, 0, 1, 0, &ROP_Python::TimingCondition),
    theRopTemplates[ROP_TPRERENDER_TPLATE],
    theRopTemplates[ROP_PRERENDER_TPLATE],
    theRopTemplates[ROP_LPRERENDER_TPLATE],
    theRopTemplates[ROP_TPREFRAME_TPLATE],
    theRopTemplates[ROP_PREFRAME_TPLATE],
    theRopTemplates[ROP_LPREFRAME_TPLATE],
    theRopTemplates[ROP_TPOSTFRAME_TPLATE],
    theRopTemplates[ROP_POSTFRAME_TPLATE],
    theRopTemplates[ROP_LPOSTFRAME_TPLATE],
    theRopTemplates[ROP_TPOSTRENDER_TPLATE],
    theRopTemplates[ROP_POSTRENDER_TPLATE],
    theRopTemplates[ROP_LPOSTRENDER_TPLATE],
    PRM_Template()
};

CH_LocalVariable ROP_Python::Variables[] = {{0, 0, 0}};

OP_Node* ROP_Python::Create(OP_Network* net, const char* name, OP_Operator* op)
{
    return new ROP_Python(net, name, op);
}

// ---

class StaticInitializer
{
public:
    
    StaticInitializer()
    {
        ROP_Python::PythonEditor.mergeFrom(PRM_SpareData::stringEditor);
        ROP_Python::PythonEditor.mergeFrom(PRM_SpareData::stringEditorLangPython);
#if UT_MAJOR_VERSION_INT < 14
        ROP_Python::PythonEditor.addTokenValue(PRM_SpareData::getEditorLinesToken(), "20");
#else
        ROP_Python::PythonEditor.addTokenValue(PRM_SpareData::getEditorLinesRangeToken(), "20");
#endif
        
        ROP_Python::Parameters[ 8].setInvisible(true);
        ROP_Python::Parameters[ 9].setInvisible(true);
        ROP_Python::Parameters[10].setInvisible(true);
        ROP_Python::Parameters[10].setDefault(0, PRM_Default(0.0, "python"));
        
        ROP_Python::Parameters[11].setInvisible(true);
        ROP_Python::Parameters[12].setInvisible(true);
        ROP_Python::Parameters[13].setInvisible(true);
        ROP_Python::Parameters[13].setDefault(0, PRM_Default(0.0, "python"));
        
        ROP_Python::Parameters[14].setInvisible(true);
        ROP_Python::Parameters[15].setInvisible(true);
        ROP_Python::Parameters[16].setInvisible(true);
        ROP_Python::Parameters[16].setDefault(0, PRM_Default(0.0, "python"));
        
        ROP_Python::Parameters[17].setInvisible(true);
        ROP_Python::Parameters[18].setInvisible(true);
        ROP_Python::Parameters[19].setInvisible(true);
        ROP_Python::Parameters[19].setDefault(0, PRM_Default(0.0, "python"));
    }
    
    ~StaticInitializer()
    {
    }
};

DLLEXPORT void newDriverOperator(OP_OperatorTable* table)
{
    static StaticInitializer sOneTimeInit;
    
    OP_Operator *op = new OP_Operator("ROP_Python",
                                      "Python",
                                      ROP_Python::Create,
                                      ROP_Python::Parameters,
                                      0,
                                      9999,
                                      ROP_Python::Variables,
                                      0);
    op->setIconName("SOP_python");
    table->addOperator(op);
}
