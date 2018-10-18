#version 330 core

in vec4 s_attribute_0;
in vec4 s_attribute_1;

out vec4 instance_data1;
out vec4 instance_data2;
out int visible;


uniform mat4 ModelViewProjectionMatrix;
uniform vec4 frustum_planes[6];


int InstanceCloudReduction()
{
//sphere - frustum test
	bool inside = true;
	for (int i = 0; i < 6; i++)
	{
		if (dot(frustum_planes[i].xyz, s_attribute_0.xyz) + frustum_planes[i].w <= -s_attribute_0.w)
			inside = false;
	}
    return inside ? 1 : 0;
}


void main()
{
//read instance data
	instance_data1 = s_attribute_0;
	instance_data2 = s_attribute_1;

//visibility
	visible = InstanceCloudReduction();
	
	gl_Position = ModelViewProjectionMatrix * vec4(s_attribute_0.xyz,1);
}
