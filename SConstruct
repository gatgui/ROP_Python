import excons
from excons.tools import houdini

env = excons.MakeBaseEnv()

prjs = [
   {"name": "ROP_Python",
    "type": "dynamicmodule",
    "ext": houdini.PluginExt(),
    "srcs": ["src/main.cpp"],
    "custom": [houdini.Require, houdini.Plugin]
   }
]

excons.DeclareTargets(env, prjs)
