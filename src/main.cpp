#include <iostream>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_String.h>
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
#include <PY/PY_Python.h>
#include <PY/PY_API.h>
#include <PY/PY_CPythonAPI.h>
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
    //std::cout << "ROP_Python::startRender " << nframes << ", " << s << ", " << e << std::endl;
    
    mPerFrame = (evalFloat("run_per_frame", 0, s) > 0.0);
    
    if (!mPerFrame)
    {
        mSingleCallTiming = (CallTiming) evalFloat("timing", 0, s);
    }
    
    evalString(mScript, "script", 0, s);
    
    return 1;
}

ROP_RENDER_CODE ROP_Python::renderFrame(fpreal time, UT_Interrupt* boss)
{
    //std::cout << "ROP_Python::renderFrame " << time << std::endl;
    
    mLastRenderedTime = time;
    
    if (mPerFrame)
    {
        //std::cout << "  Execute script" << std::endl;
        PY_InterpreterAutoLock interpreter_lock;
        if (PY_PyRun_SimpleString(mScript.nonNullBuffer()) != 0)
        {
            return ROP_ABORT_RENDER;
        }
    }
    else
    {
        //std::cout << "  DORANGE=" << DORANGE() << ", RANGE=[" << FSTART() << ", " << FEND() << "]" << std::endl;
        
        // When rendering a frame range "frame-by-frame" with input dependencies
        //   Houdini calls startRender/renderFrame/endRender for each frame...
        // Need to avoid calling script for any other frame than first or last (as specified by timing parm)
        
        if (mSingleCallTiming == FirstFrame)
        {
            OP_Context ctx;
            ctx.setFrame(FSTART());
            
            if (DORANGE() == 0 || mLastRenderedTime <= ctx.getTime())
            {
                //std::cout << "  Execute script" << std::endl;
                PY_InterpreterAutoLock interpreter_lock;
                if (PY_PyRun_SimpleString(mScript.nonNullBuffer()) != 0)
                {
                    return ROP_ABORT_RENDER;
                }
            }
        }
        else if (mSingleCallTiming == LastFrame)
        {
            OP_Context ctx;
            ctx.setFrame(FEND());
            
            if (DORANGE() == 0 || mLastRenderedTime >= ctx.getTime())
            {
                //std::cout << "  Execute script" << std::endl;
                PY_InterpreterAutoLock interpreter_lock;
                if (PY_PyRun_SimpleString(mScript.nonNullBuffer()) != 0)
                {
                    return ROP_ABORT_RENDER;
                }
            }
        }
    }
    
    return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE ROP_Python::endRender()
{
    //std::cout << "ROP_Python::endRender" << std::endl;
    
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
        ROP_Python::PythonEditor.addTokenValue(PRM_SpareData::getEditorLinesToken(), "20");
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
                                      1,
                                      1,
                                      ROP_Python::Variables,
                                      0);
    op->setIconName("SOP_python");
    table->addOperator(op);
}
