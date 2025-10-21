#include <metal_stdlib>

using namespace metal;


vertex
float4 vertex_shader(uint vertex_id[[vertex_id]], constant simd::float3* vertex_pos) {
	float4 vertex_out_pos = float4( vertex_pos[vertex_id][0], vertex_pos[vertex_id][1], vertex_pos[vertex_id][2], 1.0f);
	return vertex_out_pos;
}

fragment
float4 fragment_shader(float4 vertex_out_pos [[stage_in]]) {
	return float4(182.0f / 255.0f, 240.0f / 255.0f, 228 / 255.0f, 1.0f);
}
