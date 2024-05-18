glslangvalidator -V basic.vert -o ../basic_vert.spv

if [ $? -ne 0 ]; then
	cmd /k
fi

glslangvalidator -V basic.frag -o ../basic_frag.spv

if [ $? -ne 0 ]; then
	cmd /k
fi



