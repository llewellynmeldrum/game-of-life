#include <metal_stdlib>
#define RGBA(r,g,b,a) (r/255.0f), (g/255.0f), (b/255.0f), a

using namespace metal;


vertex
float4 vertex_shader(uint vertex_id[[vertex_id]], constant simd::float3* vertexPositions) {
	float4 vertex_out_pos = float4(
	                            vertex_pos[vertexId][0],
	                            vertex_pos[vertexId][1],
	                            vertex_pos[vertexId][2],
	                            1.0f)
	                        return vertex_out_pos;
}

fragment
float4 fragment_shader(float4 vertex_out_pos [[stage_in]]) {
	return float4(RGBA(182, 240, 228, 1));
}
