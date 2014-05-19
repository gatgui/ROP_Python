#include <iostream>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_String.h>
#include <ROP/ROP_Node.h>
#include <ROP/ROP_Templates.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_SpareData.h>
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
{
}

ROP_Python::~ROP_Python()
{
}

int ROP_Python::startRender(int nframes, fpreal s, fpreal e)
{
    mPerFrame = (evalFloat("run_per_frame", 0, s) > 0.0);
    evalString(mScript, "script", 0, s);
    
    return 1;
}

ROP_RENDER_CODE ROP_Python::renderFrame(fpreal time, UT_Interrupt* boss)
{
    mLastRenderedTime = time;
    
    if (mPerFrame)
    {
        PY_InterpreterAutoLock interpreter_lock;
        if (PY_PyRun_SimpleString(mScript.nonNullBuffer()) != 0)
        {
            return ROP_ABORT_RENDER;
        }
    }
    
    return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE ROP_Python::endRender()
{
    if (!mPerFrame)
    {
        // When rendering a frame range "frame-by-frame" with input dependencies
        //   Houdini calls startRender/renderFrame/endRender for each frame...
        // Need to avoid calling script until the final frame
        OP_Context ctx;
        ctx.setFrame(FEND());
        
        if (!getFrameByFrameFlag() || mLastRenderedTime >= ctx.getTime())
        {
            PY_InterpreterAutoLock interpreter_lock;
            if (PY_PyRun_SimpleString(mScript.nonNullBuffer()) != 0)
            {
                return ROP_ABORT_RENDER;
            }
        }
    }
    
    return ROP_CONTINUE_RENDER;
}

PRM_Name ROP_Python::ParameterNames[] = {
    PRM_Name("script", "Python Script"),
    PRM_Name("run_per_frame", "Run Per Frame")
};

PRM_Default ROP_Python::ParameterDefaults[] = {
    PRM_Default(0.0f, ""),
    PRM_Default(0.0f, "")
};

PRM_SpareData ROP_Python::PythonEditor;

PRM_Template ROP_Python::Parameters[] = {
    theRopTemplates[ROP_RENDER_TPLATE],
    theRopTemplates[ROP_RENDERDIALOG_TPLATE],
    theRopTemplates[ROP_TRANGE_TPLATE],
    theRopTemplates[ROP_FRAMERANGE_TPLATE],
    theRopTemplates[ROP_TAKENAME_TPLATE],
    PRM_Template(PRM_STRING, 1, &ParameterNames[0], &ParameterDefaults[0], 0, 0, 0, &ROP_Python::PythonEditor),
    PRM_Template(PRM_TOGGLE, 1, &ParameterNames[1], &ParameterDefaults[1]),
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
