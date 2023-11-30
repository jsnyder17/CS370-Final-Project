// CS370 Final Project
// Fall 2023

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, Couch, Table, Chair, CeilingFan, Poster, Mirror, Frame, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, TangBuffer, BiTangBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube, WhiteCube, BlueOcta, GreenSphere, SomethingCylinder, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {RedPlastic, GreenPlastic, BluePlastic, WhitePlastic, YellowPlastic, BrownPlastic};
enum Textures {Blank, Megadeth, MegadethNormFlat, Floor, Wood, WoodNormOut, Wall, WallNormFlat, WallNormOut, NumTextures};
enum LightNames {WhitePointLight, DoorLight, PaintingLight, CeilingFanLight};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;
GLint tangCoords = 3;
GLint bitangCoords = 3;

// Model files
const char * cubeFile = "../models/unitcube.obj";
const char * couchFile = "../models/couch.obj";
const char * tableFile = "../models/round_table_again.obj";
const char * chairFile = "../models/chair_again.obj";
const char * ceilingFanFile = "../models/ceiling_fan_again.obj";
const char * posterFile = "../models/poster.obj";

// Texture files
const char * blankFile = "../textures/blank.png";
const char * megadethFile = "../textures/megadeth.jpg";
const char * megadethNormFlatFile = "../textures/megadethnormflat.png";
const char * woodFile = "../textures/wood.jpg";
const char * woodNormOutFile = "../textures/woodnormout.jpg";
const char * wallFile = "../textures/wall4.png";
const char * wallNormFlatFile = "../textures/wallnormflat.png";
const char * wallNormOutFile = "../textures/wallnormout.png";

// Camera
vec3 eye = {3.0f, 0.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Bumpmapping shader program reference
GLuint bump_program;
GLuint bump_proj_mat_loc;
GLuint bump_camera_mat_loc;
GLuint bump_norm_mat_loc;
GLuint bump_model_mat_loc;
GLuint bump_vPos;
GLuint bump_vNorm;
GLuint bump_vTex;
GLuint bump_vTang;
GLuint bump_vBiTang;
GLuint bump_lights_block_idx;
GLuint bump_num_lights_loc;
GLuint bump_light_on_loc;
GLuint bump_eye_loc;
GLuint bump_base_loc;
GLuint bump_norm_loc;
const char *bump_vertex_shader = "../bumpTex.vert";
const char *bump_frag_shader = "../bumpTex.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

// My constants
vec3 wall_scale_matrix = {7.025f, 4.0f, 0.25f};
vec3 floor_scale_matrix = {7.0f, 0.1f, 7.0f};
vec3 door_coords = {3.45f, 0.0f, -1.25f};
vec3 painting_coords = {0.0f, 0.5f, 3.3f};
vec3 ceiling_fan_coords = {0.0f, -0.7f, 0.0f};
vec3 x_axis = { 1.0f, 0.0f, 0.0f };
vec3 y_axis = { 0.0f, 1.0f, 0.0f };
vec3 z_axis = { 0.0f, 0.0f, 1.0f };
vec3 first_person_center = {0.0f, 0.0f, 0.0f};
vec3 first_person_dir = { 0.0f, 0.0f, 0.0f };
vec3 first_person_eye = {0.0f, 0.5f, 0.0f};
vec3 mirror_center {15.0f, 0.5f, 0.0f};
vec3 mirror_eye = {-3.5f, 0.5f, -0.5f};
GLfloat ortho_constant = 7.0f;
GLfloat fan_angle = 0.0f;
GLfloat rpm = 3.0f;
GLfloat camera_angle = 180.0f;
GLfloat step_size = 0.1f;
GLdouble elTime = 0.0;
GLboolean pyr_dance = false;
GLboolean first_person = false;
GLboolean bump = false;
GLboolean mirror = false;
GLint x = 0; GLint y = 1; GLint z = 2;

void display();
void create_mirror();
void calculate_first_person_camera();
void update_animations();
void render_scene();
void render_walls();
void render_door();
void render_floor();
void render_ceiling();
void render_objects();
void render_table();
void render_chairs();
void render_ceiling_fan();
void build_poster(GLuint obj);
void build_mirror(GLuint m_texid);
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_textures();
void load_model(const char * filename, GLuint obj);
void load_bump_model(const char * filename, GLuint obj);
void load_texture(const char * filename, GLuint texID, GLint magFilter, GLint minFilter, GLint sWrap, GLint tWrap,
                  bool mipMap, bool invert);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void draw_frame(GLuint obj);
void draw_bump_object(GLuint obj, GLuint base_texture, GLuint normal_map);
void _computeTangentBasis(vector<vec4> & vertices, vector<vec2> & uvs, vector<vec3> & normals, vector<vec3> & tangents,
                          vector<vec3> & binormals);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv) {
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},
                                     {GL_FRAGMENT_SHADER, default_frag_shader},
                                     {GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},
                                      {GL_FRAGMENT_SHADER, lighting_frag_shader},
                                      {GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},
                                     {GL_FRAGMENT_SHADER, texture_frag_shader},
                                     {GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Load bump shader
    ShaderInfo bump_shaders[] = { {GL_VERTEX_SHADER, bump_vertex_shader},{GL_FRAGMENT_SHADER, bump_frag_shader},{GL_NONE, NULL} };
    bump_program = LoadShaders(bump_shaders);
    bump_vPos = glGetAttribLocation(bump_program, "vPosition");
    bump_vNorm = glGetAttribLocation(bump_program, "vNormal");
    bump_vTex = glGetAttribLocation(bump_program, "vTexCoord");
    bump_vTang = glGetAttribLocation(bump_program, "vTangent");
    bump_vBiTang = glGetAttribLocation(bump_program, "vBiTangent");
    bump_proj_mat_loc = glGetUniformLocation(bump_program, "proj_matrix");
    bump_camera_mat_loc = glGetUniformLocation(bump_program, "camera_matrix");
    bump_norm_mat_loc = glGetUniformLocation(bump_program, "normal_matrix");
    bump_model_mat_loc = glGetUniformLocation(bump_program, "model_matrix");
    bump_lights_block_idx = glGetUniformBlockIndex(bump_program, "LightBuffer");
    bump_num_lights_loc = glGetUniformLocation(bump_program, "NumLights");
    bump_light_on_loc = glGetUniformLocation(bump_program, "LightOn");
    bump_eye_loc = glGetUniformLocation(bump_program, "EyePosition");
    bump_base_loc = glGetUniformLocation(bump_program, "baseMap");
    bump_norm_loc = glGetUniformLocation(bump_program, "normalMap");

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();
    // Create mirror
    build_mirror(Blank);

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.0f, 0.0f, 0.7f, 1.0f);

    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Get initial time
    elTime = glfwGetTime();

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        // Create mirror
        create_mirror();

    	// Draw graphics
        display();

        // Update other events like input handling
        glfwPollEvents();

        // Update time-based animations
        update_animations();

        // Swap buffer onto screen
        glfwSwapBuffers( window );

        // Calculate first person camera coordinates
        calculate_first_person_camera();
    }

    // Close window
    glfwTerminate();
    return 0;

}

void create_mirror() {
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 5.0f;
    GLfloat yratio = 1.0f;
    GLfloat top_yratio = 7.25f;
    GLfloat bottom_yratio = 1.0f;

    // TODO: Set mirror projection matrix
    proj_matrix = frustum(-1.0f * xratio, 1.0f * xratio, -1.0f * bottom_yratio, 1.0f * top_yratio, 1.0f, 100.0f);

    // TODO: Set mirror camera matrix
    camera_matrix = lookat(mirror_eye, mirror_center, up);

    // Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[Blank]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);
}

void calculate_first_person_camera() {
    first_person_center[x] = first_person_eye[x] + (float) cos(camera_angle);
    first_person_center[y] = first_person_eye[y];
    first_person_center[z] = first_person_eye[z] + (float) sin(camera_angle);
}

void update_animations() {
    GLdouble curTime = glfwGetTime();
    GLdouble dT = curTime - elTime;

    if (pyr_dance) {
        fan_angle += (float) (dT * (rpm / 60.0) * 360.0) * 4;
    }

    elTime = curTime;
}

void display() {
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // Projections
    if (!first_person) {
        proj_matrix = ortho(-ortho_constant * xratio, ortho_constant * xratio, -ortho_constant * yratio,
                            ortho_constant * yratio, -ortho_constant, ortho_constant);
        camera_matrix = lookat(eye, center, up);

    } else {
        proj_matrix = frustum(-1.0f * xratio, 1.0f * xratio, -1.0f * yratio, 1.0f * yratio, 1.0f, 100.0f);
        camera_matrix = lookat(first_person_eye, first_person_center, up);
    }

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene() {
    render_walls();
    render_door();
    render_floor();
    render_ceiling();
    render_objects();
    render_table();
    render_chairs();
    render_ceiling_fan();
}

void render_walls() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    trans_matrix = translate(0.0f, 1.0f, 3.5f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale(wall_scale_matrix);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    trans_matrix = translate(3.63f, 1.0f, 0.0f);
    rot_matrix = rotate(90.0f, y_axis);
    scale_matrix = scale(wall_scale_matrix);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    // Wall with window
    trans_matrix = translate(0.0f, -0.45f, -3.63f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale(wall_scale_matrix[x], 1.1f, wall_scale_matrix[z]);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    trans_matrix = translate(-2.4f, 1.0f, -3.63f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale(wall_scale_matrix[x] / 3, wall_scale_matrix[y], wall_scale_matrix[z]);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    trans_matrix = translate(2.4f, 1.0f, -3.63f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale(wall_scale_matrix[x] / 3, wall_scale_matrix[y], wall_scale_matrix[z]);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    trans_matrix = translate(0.0f, 2.335f, -3.63f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale((wall_scale_matrix[x] / 3) + 0.1f, wall_scale_matrix[y] / 3, wall_scale_matrix[z]);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);

    trans_matrix = translate(-3.63f, 1.0f, 0.0f);
    rot_matrix = rotate(90.0f, y_axis);
    scale_matrix = scale(wall_scale_matrix);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Cube, Wall, WallNormOut);
    else
        draw_tex_object(Cube, Wall);
}

void render_door() {
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Door
    trans_matrix = translate(door_coords);
    scale_matrix = scale(1.5f, 2.0f, 0.1f);
    rot_matrix = rotate(90.0f, y_axis);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cube, WhitePlastic);

    // Knob
    trans_matrix = translate(3.40f, 0.0f, -0.75f);
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Sphere, YellowPlastic);
}

void render_floor() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Draw floor
    trans_matrix = translate(0.0f, -1.0f, 0.0f);
    scale_matrix = scale(floor_scale_matrix);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Cube, BrownPlastic);
}

void render_ceiling() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Draw ceiling
    trans_matrix = translate(0.0f, 3.0f, 0.0f);
    scale_matrix = scale(floor_scale_matrix);
    model_matrix = trans_matrix*scale_matrix;
    draw_tex_object(Cube, Floor);
}

void render_objects() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 rot_matrix_2 = mat4().identity();
    mat4 rot_matrix_3 = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Mirror
    if (!mirror)
        draw_frame(Frame);
        trans_matrix = translate(-3.5f, 1.0f, -0.5f);
        scale_matrix = scale(3.5f, 4.5f, 0.1f);
        rot_matrix = rotate(90.0f, y_axis);
        rot_matrix_2 = rotate(90.0f, z_axis);
        model_matrix = trans_matrix * rot_matrix * rot_matrix_2 * scale_matrix;
        draw_tex_object(Cube, Blank);

    // Sofa
    trans_matrix = translate(2.0f, -0.85f, -3.0f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(Couch, BluePlastic);

    trans_matrix = translate(painting_coords);
    rot_matrix = rotate(0.0f, z_axis);
    rot_matrix_2 = rotate(180.0f, y_axis);
    rot_matrix_3 = rotate(90.0f, x_axis);
    scale_matrix = scale(1.0f, 1.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*rot_matrix_2*rot_matrix_3*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (bump)
        draw_bump_object(Poster, Megadeth, MegadethNormFlat);
    else
        draw_tex_object(Poster, Megadeth);
}

void render_table() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Draw table
    trans_matrix = translate(0.0f, -1.0f, 0.0f);
    scale_matrix = scale(0.5f, 0.5f, 0.5f);
    model_matrix = trans_matrix*scale_matrix;
    if (bump)
        draw_bump_object(Table, Wood, WoodNormOut);
    else
        draw_tex_object(Table, Wood);
}

void render_chairs() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 rot_2_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Draw chairs
    trans_matrix = translate(0.0f, -1.0f, -1.0f);
    scale_matrix = scale(0.01f, 0.01f, 0.01f);
    rot_matrix = rotate(270.0f, x_axis);
    model_matrix = trans_matrix*scale_matrix*rot_matrix;
    if (bump)
        draw_bump_object(Chair, Wood, WoodNormOut);
    else
        draw_tex_object(Chair, Wood);

    trans_matrix = translate(0.0f, -1.0f, 1.0f);
    scale_matrix = scale(0.01f, 0.01f, 0.01f);
    rot_matrix = rotate(270.0f, x_axis);
    rot_2_matrix = rotate(180.0f, z_axis);
    model_matrix = trans_matrix*scale_matrix*rot_matrix*rot_2_matrix;
    if (bump)
        draw_bump_object(Chair, Wood, WoodNormOut);
    else
        draw_tex_object(Chair, Wood);
}

void render_ceiling_fan() {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Draw fan
    trans_matrix = translate(ceiling_fan_coords);
    scale_matrix = scale(0.004f, 0.004f, 0.004f);
    rot_matrix = rotate(fan_angle, y_axis);
    model_matrix = trans_matrix*scale_matrix*rot_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(CeilingFan, YellowPlastic);
}

void build_poster(GLuint obj) {
    // Carpet geometry
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> indices;

    vertices = {
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
    };

    /******************************************/
    /*       INSERT (b) CODE HERE             */
    /******************************************/
    // TODO: Add carpet texture coordinates
    uvCoords = {
            vec2(1.0f, 1.0f),
            vec2(1.0f, 0.0f),
            vec2(0.0f, 0.0f),
            vec2(0.0f, 1.0f),
    };

    // Define face indices
    indices = {
            {0, 1, 2},
            {2, 3, 0},
    };
    int numFaces = indices.size();

    // Create object vertices and colors from faces
    vector<vec4> obj_vertices;
    vector<vec3> obj_normals;
    vector<vec2> obj_uvs;
    for (int i = 0; i < numFaces; i++) {
        for (int j = 0; j < 3; j++) {
            obj_vertices.push_back(vertices[indices[i][j]]);
            obj_normals.push_back(vec3(0.0f, 1.0f, 0.0f));
            obj_uvs.push_back(uvCoords[indices[i][j]]);
        }
    }

    // Set numVertices as total number of INDICES
    numVertices[obj] = 3*numFaces;

    // Generate object buffers for obj
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);

    // Bind and load object buffers for obj
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], obj_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], obj_normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], obj_uvs.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tangCoords*numVertices[obj], obj_uvs.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*bitangCoords*numVertices[obj], obj_uvs.data(), GL_STATIC_DRAW);
}

void build_mirror(GLuint m_texid ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[m_texid]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[m_texid]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void build_frame(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;

    // Create wireframe for mirror
    vertices = {
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f)
    };

    normals = {
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
    };

    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_geometry() {
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_bump_model(cubeFile, Cube);
    load_model(couchFile, Couch);
    load_bump_model(tableFile, Table);
    load_bump_model(chairFile, Chair);
    load_model(ceilingFanFile, CeilingFan);
    load_bump_model(posterFile, Poster);

    build_poster(Poster);

    build_frame(Frame);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);
}

void build_materials() {
    // Add materials to Materials vector
    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties greenPlastic = {
            vec4(0.0f, 0.3f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 0.6f, 0.0f, 1.0f), //diffuse
            vec4(0.6f, 0.8f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties bluePlastic = {
            vec4(0.0f, 0.0f, 0.3f, 1.0f), //ambient
            vec4(0.0f, 0.0f, 0.6f, 1.0f), //diffuse
            vec4(0.6f, 0.6f, 0.8f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties whitePlastic = {
            vec4(0.3f, 0.3f, 0.3f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties yellowPlastic = {
            vec4(0.4f, 0.4f, 0.1f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.1f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.2f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties brownPlastic = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            20.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad //pad
    };

    Materials.push_back(redPlastic);
    Materials.push_back(greenPlastic);
    Materials.push_back(bluePlastic);
    Materials.push_back(whitePlastic);
    Materials.push_back(yellowPlastic);
    Materials.push_back(brownPlastic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(),
                 GL_STATIC_DRAW);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(3.0f, 4.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties doorLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(door_coords[x] - 1, 9.0f, door_coords[z], 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            10.0f,   //cutoff
            10.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties paintingLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(painting_coords[x], 9.0f, painting_coords[z] - 1, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            10.0f,   //cutoff
            10.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    LightProperties ceilingFanLight = {
            SPOT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.2f, 0.2f, 0.2f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(ceiling_fan_coords[x], 9.0f, ceiling_fan_coords[z], 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            10.0f,   //cutoff
            10.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    Lights.push_back(whitePointLight);
    Lights.push_back(doorLight);
    Lights.push_back(paintingLight);
    Lights.push_back(ceilingFanLight);

    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(),
                 GL_STATIC_DRAW);
}

void build_textures() {

    // Create textures and activate unit 0
    glGenTextures( NumTextures,  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    load_texture(blankFile, Blank, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(megadethFile, Megadeth, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(megadethNormFlatFile, MegadethNormFlat, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(woodFile, Wood, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(woodNormOutFile, WoodNormOut, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(wallFile, Wall, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(wallNormFlatFile, WallNormFlat, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
    load_texture(wallNormOutFile, WallNormOut, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
                 GL_REPEAT, GL_REPEAT, true, false);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        lightOn[WhitePointLight] = (lightOn[WhitePointLight] + 1) % 2;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        lightOn[DoorLight] = (lightOn[DoorLight] + 1) % 2;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        lightOn[PaintingLight] = (lightOn[PaintingLight] + 1) % 2;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        lightOn[CeilingFanLight] = (lightOn[CeilingFanLight] + 1) % 2;
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        pyr_dance = (pyr_dance + 1) % 2;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        first_person = (first_person + 1) % 2;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        bump = (bump + 1) % 2;
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        mirror = (mirror + 1) % 2;
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        if (!first_person) {
            azimuth += daz;
            if (azimuth > 360.0) {
                azimuth -= 360.0;
            }
        } else {
            camera_angle -= 0.1f;
        }

    } else if (key == GLFW_KEY_D) {
        if (!first_person) {
            azimuth -= daz;
            if (azimuth < 0.0)
            {
                azimuth += 360.0;
            }
        } else {
            camera_angle += 0.1f;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        if (!first_person) {
            elevation += del;
            if (elevation > 179.0)
            {
                elevation = 179.0;
            }
        } else {
            first_person_dir = first_person_center - first_person_eye;
            first_person_eye = first_person_eye + (first_person_dir * step_size);
        }
    }
    else if (key == GLFW_KEY_S)
    {
        if (!first_person) {
            elevation -= del;
            if (elevation < 1.0)
            {
                elevation = 1.0;
            }
        } else {
            first_person_dir = first_person_center - first_person_eye;
            first_person_eye = first_person_eye - (first_person_dir * step_size);
        }
    }

    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x,y,z);

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods) {

}

#include "utilfuncs.cpp"
