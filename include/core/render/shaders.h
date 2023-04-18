static const char* shaderCodeVertex = R"(
    #version 460 core
    layout(std140, binding = 0) uniform PerFrameData{
        uniform mat4 mvp;
        uniform int is_wireframe;
    };
    layout (location=0) out vec3 color;

    const vec3 pos[8] = vec3[8](
        vec3(-1.0,-1.0, 1.0), vec3( 1.0,-1.0, 1.0),
        vec3(1.0, 1.0, 1.0), vec3(-1.0, 1.0, 1.0),
        vec3(-1.0,-1.0,-1.0), vec3(1.0,-1.0,-1.0),
        vec3( 1.0, 1.0,-1.0), vec3(-1.0, 1.0,-1.0)
    );

    const vec3 col[8] = vec3[8](
        vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0),
        vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
        vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)
    );

    const int indices[36] = int[36](
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            7, 6, 5, 5, 4, 7,
            4, 0, 3, 3, 7, 4,
            4, 5, 1, 1, 0, 4,
            3, 2, 6, 6, 7, 3
    );


    void main(){
        int idx = indices[gl_VertexID];
        gl_Position = mvp * vec4(pos[idx], 1.0);
        color = is_wireframe > 0 ? vec3(0.0) : col[idx];
    };

)";

static const char* shaderCodeFragment = R"(
    #version 460 core
    layout (location=0) in vec3 color;
    layout (location=0) out vec4 out_FragColor;
    void main()
    {
        out_FragColor = vec4(color, 1.0);
    };
)";

static const char* sdr_code_vert_textured = R"(
    #version 460 core
    layout(std140, binding = 0) uniform PerFrameData{
        uniform mat4 mvp;
        uniform int is_wireframe;
    };
    layout (location=0) out vec2 uv;

    const vec3 pos[8] = vec3[8](
        vec3(-1.0,-1.0, 1.0), vec3( 1.0,-1.0, 1.0),
        vec3(1.0, 1.0, 1.0), vec3(-1.0, 1.0, 1.0),
        vec3(-1.0,-1.0,-1.0), vec3(1.0,-1.0,-1.0),
        vec3( 1.0, 1.0,-1.0), vec3(-1.0, 1.0,-1.0)
    );

    const vec2 tc[4] = vec2[4](
        vec2( 0.0, 0.0 ),
        vec2( 0.0, 1.0 ),
        vec2( 1.0, 0.0 ),
        vec2( 1.0, 1.0 )
    );

    const int indices[36] = int[36](
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            7, 6, 5, 5, 4, 7,
            4, 0, 3, 3, 7, 4,
            4, 5, 1, 1, 0, 4,
            3, 2, 6, 6, 7, 3
    );


    void main(){
        int idx = indices[gl_VertexID];
        gl_Position = mvp * vec4(pos[idx], 1.0);
        uv = tc[idx];
    };
)";

static const char* sdr_code_frag_textured = R"(
        #version 460 core
        layout (location=0) in vec2 uv;
        layout (location=0) out vec4 out_FragColor;
        uniform sampler2D texture0;
        
        void main(){
            out_FragColor = texture(texture0, uv);
        }
)";

static const char* imgui_sdr_vert = R"(
    #version 460 core
    layout (location = 0) in vec2 position;
    layout (location = 1) in vec2 uv;
    layout (location = 2) in vec4 color;
    layout (std140, binding = 0) uniform PerFrameData{
        uniform mat4 MVP;
    };
    out vec2 frag_uv;
    out vec4 frag_color;
    
    void main(){
        frag_uv = uv;
        frag_color = color;
        gl_Position = MVP * vec4(position.xy,0,1);
    }
)";

static const char* imgui_sdr_frag = R"(
    #version 460 core
    in vec2 frag_uv;
    in vec4 frag_color;
    layout (binding = 0) uniform sampler2D Texture;
    layout (location = 0) out vec4 out_color;

    void main(){
        out_color = frag_color * texture(Texture, frag_uv.st);
    }
)";