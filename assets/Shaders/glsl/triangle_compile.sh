glslangvalidator -V triangle.vert -o triangle_vert.spv

if [ $? -ne 0 ]; then
	cmd /k
fi

glslangvalidator -V triangle.frag -o triangle_frag.spv

if [ $? -ne 0 ]; then
	cmd /k
fi



