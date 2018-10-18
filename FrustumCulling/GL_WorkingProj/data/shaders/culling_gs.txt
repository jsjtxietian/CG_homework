#version 400 core

layout (points) in;
layout (points, max_vertices = 1) out;

in vec4 instance_data1[];
in vec4 instance_data2[];
in int visible[];

out vec4 output_instance_data1;
out vec4 output_instance_data2;

void main(	)
{
	//if primitive is not visible - discard it !
	//visible comes from vertex shader
	if (visible[0] == 1)
	{
		//just transfer data
		output_instance_data1 = instance_data1[0];
		output_instance_data2 = instance_data2[0];
		
		gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
		EmitVertex();
		EndPrimitive();
	}
}