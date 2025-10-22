using namespace metal;
// *INDENT-OFF*

struct VertexOut {
	/**/
	float4 pos [[position]];
	float2 uv;
};
typedef float4 FragmentOut;


/* called per vertex */
vertex
VertexOut
vertex_shader(constant float4* vertex_buf [[buffer(0)]],
              uint id [[vertex_id]]) {
/* The shader wants:
	1. a constant flat buffer of float4, 'verticies'
	2. the builtin vertex_id AS 'id'
	- id acts as the index for each vertex that is processed.
	- We defined which verticies to process in our draw() function.
		- The mtl.vertex_offset we defined describes the start position in the vertex_buf 
		- the mtl.vertex_count describes how many verticies will be processed per frame by this function.

	- in this function, we simply grab the vertex relevant to the call we are currently in,
	  and set its position and uv, and return that.
	- uv = (pos+1)/2 (?)
*/
	float4 cur_vtx = vertex_buf[id];
	return (VertexOut) {
		.pos = cur_vtx,
		.uv = (float2){ (cur_vtx.x+1)/2, (cur_vtx.y+1)/2},
	};
}

/* called per pixel */
fragment
FragmentOut
fragment_shader(VertexOut vtx[[stage_in]],
		texture2d<uint> generation [[texture(0)]]){
	constexpr sampler smplr(coord::normalized, address::clamp_to_zero, filter::nearest);
	uint cell = generation.sample(smplr, vtx.uv).r;
	return float4(cell);
}
