
struct VSOut {
	float4 position [[position]];
};

vertex VSOut vertex_shader(constant float4* verts [[buffer(0)]],
                           uint vid [[vertex_id]]) {
	VSOut o;
	o.position = verts[vid];
	return o;
}

fragment float4 fragment_shader(VSOut in [[stage_in]]) {
	return float4(1, 0, 1, 1); // magenta test
}
kernel void compute_shader() {

}
