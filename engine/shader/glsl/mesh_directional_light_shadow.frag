#version 310 es

layout(early_fragment_tests) in;

//对应于附件0
layout(location = 0) out highp float out_depth;

void main()
{
    out_depth = gl_FragCoord.z;
}