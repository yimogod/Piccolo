#version 310 es

layout(early_fragment_tests) in;

// 对应于附件0.
// 但实际上, c++新建了2个附件. 另外一个索引为1的附件是深度缓冲
// 由此推出, 深度缓冲不用在这里声明. 只要开启了, 就会直接写入
layout(location = 0) out highp float out_depth;

void main()
{
    out_depth = gl_FragCoord.z;
}