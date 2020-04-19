#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"


#include <iostream>
#include "nanoflann.hpp"

using namespace Ubpa;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
gl::Texture2D loadTexture(char const* path);
gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources);

// settings
unsigned int scr_width = 800;
unsigned int scr_height = 600;
float displacement_bias = 0.f;
float displacement_scale = 1.f;
float displacement_lambda = 0.2f;
bool have_denoise = false;

// camera
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// point cloud
template <typename T>
struct PointCloud
{
    struct Point
    {
        T  x, y;
        float disp;
    };

    std::vector<Point>  pts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline T kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        if (dim == 0) return pts[idx].x;
        else return pts[idx].y;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW8 - denoise", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // build and compile our shader zprogram
    // ------------------------------------
    gl::Shader vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3_denoise.vert");
    gl::Shader fs(gl::ShaderType::FragmentShader, "../data/shaders/light.frag");
    gl::Program program(&vs, &fs);
    rgbf ambient{ 0.2f,0.2f,0.2f };
    program.SetTex("albedo_texture", 0);
    program.SetTex("displacementmap", 1);
    program.SetVecf3("point_light_pos", { 0,5,0 });
    program.SetVecf3("point_light_radiance", { 100,100,100 });
    program.SetVecf3("ambient_irradiance", ambient);
    program.SetFloat("roughness", 0.5f );
    program.SetFloat("metalness", 0.f);

    // load model
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj", true);
    // world space positions of our cubes
    pointf3 instancePositions[] = {
        pointf3(0.0f,  0.0f,  0.0f),
        pointf3(2.0f,  5.0f, -15.0f),
        pointf3(-1.5f, -2.2f, -2.5f),
        pointf3(-3.8f, -2.0f, -12.3f),
        pointf3(2.4f, -0.4f, -3.5f),
        pointf3(-1.7f,  3.0f, -7.5f),
        pointf3(1.3f, -2.0f, -2.5f),
        pointf3(1.5f,  2.0f, -2.5f),
        pointf3(1.5f,  0.2f, -1.5f),
        pointf3(-1.3f,  1.0f, -1.5f)
    };

    // load and create a texture 
    // -------------------------
    gl::Texture2D spot_albedo = loadTexture("../data/textures/spot_albedo.png");

    gl::Texture2D displacementmap = genDisplacementmap(spot);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        gl::ClearColor({ ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

        program.SetVecf3("camera_pos", camera.Position);

        // bind textures on corresponding texture units
        program.Active(0, &spot_albedo);
        program.Active(1, &displacementmap);

        // pass projection matrix to shader (note that in this case it could change every frame)
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        program.SetMatf4("projection", projection);

        // camera/view transformation
        program.SetMatf4("view", camera.GetViewMatrix());
        program.SetFloat("displacement_bias", displacement_bias);
        program.SetFloat("displacement_scale", displacement_scale);
        program.SetFloat("displacement_lambda", displacement_lambda);
        program.SetBool("have_denoise", have_denoise);

        // render spots
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            //float angle = 20.0f * i;
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            program.SetMatf4("model", model);
            spot->va->Draw(&program);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    delete spot;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        displacement_lambda = -1;
        printf("current lambda: %lf\n", displacement_lambda);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        displacement_lambda = 1;
        printf("current lambda: %lf\n", displacement_lambda);
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        have_denoise = false;
        printf("Current denoise: %d\n", have_denoise);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        have_denoise = true;
        printf("Current denoise: %d\n", have_denoise);
    }
       

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

gl::Texture2D loadTexture(char const* path)
{
    gl::Texture2D tex;
    tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    gl::PixelDataFormat c2f[4] = {
        gl::PixelDataFormat::Red,
        gl::PixelDataFormat::Rg,
        gl::PixelDataFormat::Rgb,
        gl::PixelDataFormat::Rgba
    };
    gl::PixelDataInternalFormat c2if[4] = {
        gl::PixelDataInternalFormat::Red,
        gl::PixelDataInternalFormat::Rg,
        gl::PixelDataInternalFormat::Rgb,
        gl::PixelDataInternalFormat::Rgba
    };
    if (data)
    {
        tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
        tex.GenerateMipmap();
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return tex;
}

gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources) {
    float* displacementData = new float[1024 * 1024];
    memset((void *)displacementData, 0, sizeof(float) * 1024 * 1024);
    // TODO: HW8 - 1_denoise | genDisplacementmap
    // 1. set displacementData with resources's positions, indices, normals, ...
    // 2. change global variable: displacement_bias, displacement_scale, displacement_lambda

    // auxiliary functions
    auto findOrPush = [](std::vector<size_t>& v, int index) {
        if (std::find(v.begin(), v.end(), index) == v.end()) {
            v.push_back(index);
        }
    };

    auto getNeighbor = [&](int vec) {
        std::vector<size_t> neighbors;
        size_t total_triangles = resources->indices.size() / 3;
        for (size_t i = 0; i < total_triangles; i++) {
            if (vec == resources->indices[i * 3]) {
                findOrPush(neighbors, resources->indices[i * 3 + 1]);
                findOrPush(neighbors, resources->indices[i * 3 + 2]);
            }
            else if (vec == resources->indices[i * 3 + 1]) {
                findOrPush(neighbors, resources->indices[i * 3]);
                findOrPush(neighbors, resources->indices[i * 3 + 2]);
            }
            else if (vec == resources->indices[i * 3 + 2]) {
                findOrPush(neighbors, resources->indices[i * 3]);
                findOrPush(neighbors, resources->indices[i * 3 + 1]);
            }
        }
        return neighbors;
    };

    // 1. calculate delta of all vertices
    std::vector<float> offsets;
    int total_vertices = resources->positions.size();
    for (size_t i = 0; i < total_vertices; i++) {

#ifdef ZT_DBG
        printf("Processing %d\n", i);

        auto pos_self = resources->positions[i];
        printf("pos_self: (%lf, %lf, %lf)\n", pos_self[0], pos_self[1], pos_self[2]);
#endif

        // find neighbour vertices
        auto neighbors = getNeighbor(i);

#ifdef ZT_DBG
        printf("Neighbors: ");
        for (auto m : neighbors) {
            printf("%d ", m);
        }
        printf("\n");

#endif
        size_t num_neighbors = neighbors.size();

        assert(neighbors.size() > 0);
        vecf3 offset = resources->positions[i].cast_to<vecf3>();

        for (auto neighbor : neighbors) {
            offset -= (1.0f / num_neighbors) * resources->positions[neighbor].cast_to<vecf3>();
        }
#ifdef ZT_DBG
        printf("offset: (%lf, %lf, %lf)\n", offset[0], offset[1], offset[2]);
#endif
        // project to normal
        // todo: figure out why we can't use normal?
        offsets.push_back(offset.dot(resources->normals[i].cast_to<vecf3>()));
    }
#ifdef ZT_DBG
    for (auto offset : offsets) {
        printf("%lf  ", offset);
    }
#endif
    std::cout << std::endl;

    // 2. get min and max of all offsets
    float min_offset = offsets[0];
    float max_offset = offsets[0];

    for (size_t i = 0; i < total_vertices; i++) {
        if (offsets[i] < min_offset) {
            min_offset = offsets[i];
        }
        else if (offsets[i] > max_offset) {
            max_offset = offsets[i];
        }
    }

    std::cout << "offset: " << min_offset << " " << max_offset << std::endl;
    // [0, 1] -> [min_offset, max_offset]
    displacement_bias = min_offset;
    displacement_scale = max_offset - min_offset;

    PointCloud<int> cloud;
    std::map<pointi2, size_t> rev_map;
    cloud.pts.resize(total_vertices);

    // 3. find apporpriate map from vertices to uv coords - done by texture coords
    for (size_t i = 0; i < total_vertices; i++) {
        
        // coord: [0, 1] -> [0, 1023]
        // value: [0, 1]
        pointf2 texcoord = resources->texcoords[i];
        vecf2 real_coord = texcoord.cast_to<vecf2>() * 1023;

        // add into point cloud
        cloud.pts[i].x = (int)floor(real_coord[0]);
        cloud.pts[i].y = (int)floor(real_coord[1]);
        cloud.pts[i].disp = (offsets[i] - min_offset) / displacement_scale;

        displacementData[cloud.pts[i].x + cloud.pts[i].y * 1024] =
            cloud.pts[i].disp;

        // add into map
        rev_map.insert(std::make_pair(pointi2(cloud.pts[i].x, cloud.pts[i].y), i));
    }

    // build kd-tree
    typedef nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor<int, PointCloud<int>, double>,
        PointCloud<int>,
        2 /* dim */
    > my_kd_tree_t;

    my_kd_tree_t   index(2 /*dim*/, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
    index.buildIndex();



    // interpolate remaining
    for (size_t u = 0; u < 1024; u++) {
        for (size_t v = 0; v < 1024; v++) {
            if (rev_map.find(pointi2(u, v)) == rev_map.end()) {
                // do kd-tree search, find 2 nearest neighbor and do interpolation
                size_t num_neighbors = 10;
                nanoflann::KNNResultSet<double> resultSet(num_neighbors);
                std::vector<size_t> ret_index(num_neighbors);
                std::vector<double> out_dist_sqr(num_neighbors);

                int query_pt[2] = { u, v };
                num_neighbors = index.knnSearch(&query_pt[0], num_neighbors, &ret_index[0], &out_dist_sqr[0]);

                //std::cout << "knnSearch(nn=2): \n";
                //std::cout << "ret_index=" << ret_index << " out_dist_sqr=" << out_dist_sqr << std::endl;

                double total_dist = 0;
                for (size_t i = 0; i < num_neighbors; i++) {
                    total_dist += out_dist_sqr[i];
                }
                displacementData[u + v * 1024] = 0;
                for (size_t i = 0; i < num_neighbors; i++) {
                    displacementData[u + v * 1024] +=
                        (out_dist_sqr[i] / total_dist) * cloud.pts[ret_index[i]].disp;
                }
            }
        }
    }


    gl::Texture2D displacementmap;
    displacementmap.SetImage(0, gl::PixelDataInternalFormat::Red, 1024, 1024, gl::PixelDataFormat::Red, gl::PixelDataType::Float, displacementData);
    displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
        gl::MinFilter::Linear, gl::MagFilter::Linear);
    stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
    for (size_t i = 0; i < 1024 * 1024; i++)
        stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
    stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);
    delete[] stbi_data;
    delete[] displacementData;
    return displacementmap;
}
