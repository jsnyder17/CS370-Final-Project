// CS370 Final Project
// Fall 2023

// Create solid color buffer
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer) {
    vector<vec4> obj_colors;
    for (int i = 0; i < num_vertices; i++) {
        obj_colors.push_back(color);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*num_vertices, obj_colors.data(), GL_STATIC_DRAW);
}

void load_model(const char * filename, GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Load model and set number of vertices
    loadOBJ(filename, vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void load_bump_model(const char * filename, GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;
    vector<vec3> tangents;
    vector<vec3> bitangents;

    // Load model and set number of vertices
    loadOBJ(filename, vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // TODO: Compute tangents and bitangents
    _computeTangentBasis(vertices, uvCoords, normals, tangents, bitangents);

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tangCoords*numVertices[obj], tangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*bitangCoords*numVertices[obj], bitangents.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void load_texture(const char * filename, GLuint texID, GLint magFilter, GLint minFilter, GLint sWrap, GLint tWrap, bool mipMap, bool invert) {
    int w, h, n;
    int force_channels = 4;
    unsigned char *image_data;

    // Activate unit 0
    glActiveTexture( GL_TEXTURE0 );

    image_data = stbi_load(filename, &w, &h, &n, force_channels);
    if (!image_data) {
        fprintf(stderr, "ERROR: could not load %s\n", filename);
    }
    // NPOT check for power of 2 dimensions
    if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
        fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n", filename);
    }
    
    // Invert image (e.g. jpeg, png)
    if (invert) {
        int width_in_bytes = w * 4;
        unsigned char *top = NULL;
        unsigned char *bottom = NULL;
        unsigned char temp = 0;
        int half_height = h / 2;

        for ( int row = 0; row < half_height; row++ ) {
            top = image_data + row * width_in_bytes;
            bottom = image_data + ( h - row - 1 ) * width_in_bytes;
            for ( int col = 0; col < width_in_bytes; col++ ) {
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                top++;
                bottom++;
            }
        }
	}

    // Bind current texture id
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texID]);
    // Load image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data);
    // Generate mipmaps for texture
    if (mipMap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    // Set scaling modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    // Set wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
    // Set maximum anisotropic filtering for system
    GLfloat max_aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    // set the maximum!
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
}

// Draw object with color
void draw_color_obj(GLuint obj, GLuint color) {
    // Select default shader program
    glUseProgram(default_program);

    // Pass projection matrix to default shader
    glUniformMatrix4fv(default_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to default shader
    glUniformMatrix4fv(default_cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to default shader
    glUniformMatrix4fv(default_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(default_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vPos);

    // Bind color buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(default_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vCol);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_mat_object(GLuint obj, GLuint material){
    // Select shader program
    glUseProgram(lighting_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));

    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Pass material index to shader
    glUniform1i(lighting_material_loc, material);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_tex_object(GLuint obj, GLuint texture){
    // Select shader program
    glUseProgram(texture_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(texture_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to shader
    glUniformMatrix4fv(texture_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(texture_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texture]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(texture_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vPos);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(texture_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vTex);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_bump_object(GLuint obj, GLuint base_texture, GLuint normal_map){
    // Select shader program
    glUseProgram(bump_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(bump_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(bump_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(bump_program, bump_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

    // Set camera position
    glUniform3fv(bump_eye_loc, 1, eye);

    // Set num lights and lightOn
    glUniform1i(bump_num_lights_loc, numLights);
    glUniform1iv(bump_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(bump_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(bump_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Set base texture to texture unit 0 and make it active
    glUniform1i(bump_base_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    // Bind base texture (to unit 0)
    glBindTexture(GL_TEXTURE_2D, TextureIDs[base_texture]);

    // Set normal map texture to texture unit 1 and make it active
    glUniform1i(bump_norm_loc, 1);
    glActiveTexture(GL_TEXTURE1);
    // Bind normal map texture (to unit 1)
    glBindTexture(GL_TEXTURE_2D, TextureIDs[normal_map]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(bump_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(bump_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vNorm);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(bump_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vTex);

    // Bind tangent object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TangBuffer]);
    glVertexAttribPointer(bump_vTang, tangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vTang);

    // Bind bitangent object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][BiTangBuffer]);
    glVertexAttribPointer(bump_vBiTang, bitangCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(bump_vBiTang);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

void draw_frame(GLuint obj){
    // Draw frame using lines at mirror location
    glUseProgram(lighting_program);
    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));
    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));
    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);
    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Set frame transformation matrix
    mat4 trans_matrix = translate(mirror_eye);
    mat4 rot_matrix = rotate(90.0f, z_axis);
    mat4 scale_matrix = scale(0.5f, 0.0f, 1.5f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);
    glUniform1i(lighting_material_loc, RedPlastic);

    // Draw object using line loop
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);
    glDrawArrays(GL_LINE_LOOP, 0, numVertices[obj]);
}


void _computeTangentBasis(
        // inputs
        std::vector<vec4> & vertices,
        std::vector<vec2> & uvs,
        std::vector<vec3> & normals,
        // outputs
        std::vector<vec3> & tangents,
        std::vector<vec3> & binormals
){

    for (unsigned int i=0; i<vertices.size(); i+=3 ){

        // Shortcuts for vertices
        vec4 & v0 = vertices[i+0];
        vec4 & v1 = vertices[i+1];
        vec4 & v2 = vertices[i+2];

        // Shortcuts for UVs
        vec2 & uv0 = uvs[i+0];
        vec2 & uv1 = uvs[i+1];
        vec2 & uv2 = uvs[i+2];

        // Edges of the triangle : postion delta
        vec3 deltaPos1;
        deltaPos1[0] = v1[0]-v0[0];
        deltaPos1[1] = v1[1]-v0[1];
        deltaPos1[2] = v1[2]-v0[2];
        vec3 deltaPos2;
        deltaPos2[0] = v2[0]-v0[0];
        deltaPos2[1] = v2[1]-v0[1];
        deltaPos2[2] = v2[2]-v0[2];

        // UV delta
        vec2 deltaUV1 = uv1-uv0;
        vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0]);
        vec3 tangent = (deltaPos1 * deltaUV2[1]   - deltaPos2 * deltaUV1[1])*r;
        vec3 binormal = (deltaPos2 * deltaUV1[0]   - deltaPos1 * deltaUV2[0])*r;

        // Set the same tangent for all three vertices of the triangle.
        // They will be merged later, in vboindexer.cpp
        tangents.push_back(tangent);
        tangents.push_back(tangent);
        tangents.push_back(tangent);

        // Same thing for binormals
        binormals.push_back(binormal);
        binormals.push_back(binormal);
        binormals.push_back(binormal);

    }

    // See "Going Further"
    for (unsigned int i=0; i<vertices.size(); i+=1 )
    {
        vec3 & n = normals[i];
        vec3 & t = tangents[i];
        vec3 & b = binormals[i];

        // Gram-Schmidt orthogonalize
        t = normalize(t - n * dot(n, t));

        // Calculate handedness
        if (dot(cross(n, t), b) < 0.0f){
            t = t * -1.0f;
        }

    }


}
