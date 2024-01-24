#include "Sandbox3D.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"

using namespace Ume;

static void ImGuiShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static float s_SkyboxVt[] = {
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
};

static uint32_t s_SkyboxId[] = {
    0, 3, 1,
    1, 3, 2,
    2, 3, 7,
    2, 7, 6,
    0, 4, 7,
    0, 7, 3,
    1, 2, 6,
    1, 6, 5,
    0, 1, 4,
    1, 5, 4,
    4, 5, 6,
    4, 6, 7,
};

static float s_QuadVertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f
};

static uint32_t s_QuadIndices[] = {
    0, 1, 2, 2, 3, 0
};

static float cubeVertices[] = {
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,

    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,

    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f};

static uint32_t cubeIndices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20};

static Ref<VertexArray> cubeVertexArray = nullptr;
static Ref<Shader> textureShader = nullptr;

// skybox
static Ref<VertexArray> s_SkyboxVA = nullptr;
static Ref<Shader> s_SkyboxShader = nullptr;

static Ref<VertexArray> s_QuadVA = nullptr;
static Ref<Shader> s_QuadShader = nullptr;

static float s_Exposure = 1.4f;
static bool s_UseHDR = true;
static bool s_UseGamma = false;

static glm::vec3 s_Scale = glm::vec3(1.0f);

static std::vector<glm::vec3> s_SSAOKernel;


//////////////////////////////////////////////////
// PBR
//////////////////////////////////////////////////

PBRLayer::PBRLayer()
    : m_CameraSample(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
{
    auto &window = Application::Get().GetWindow();
    window.SetVSync(true);

    m_CameraController = PerspectiveCameraController(45.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.1f, 500.0f);
    m_Camera = PerspectiveCamera(45.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.1f, 500.0f);

    m_RenderPasses.push_back({ "Pos", Framebuffer::Create(window.GetWidth(), window.GetHeight(), FramebufferFormat::RGBA16F) });
    m_RenderPasses.push_back({ "Nor", Framebuffer::Create(window.GetWidth(), window.GetHeight(), FramebufferFormat::RGBA16F) });
    m_RenderPasses.push_back({ "SSAO", Framebuffer::Create(window.GetWidth(), window.GetHeight(), FramebufferFormat::RGBA8) });
    m_RenderPasses.push_back({ "PBR", Framebuffer::Create(window.GetWidth(), window.GetHeight(), FramebufferFormat::RGBA16F) });
    m_RenderPasses.push_back({ "HDR", Framebuffer::Create(window.GetWidth(), window.GetHeight(), FramebufferFormat::RGBA8) });

    m_FinalPass = m_RenderPasses.size() - 1;

    LUT::PrecomputeEmu("assets/textures/luts/LUT_Emu.png");
    LUT::PrecomputeEavg("assets/textures/luts/LUT_Eavg.png", "assets/textures/luts/LUT_Emu.png");

    m_EmuLUT = Texture2D::Create("assets/textures/luts/LUT_Emu.png");
    m_EavgLUT = Texture2D::Create("assets/textures/luts/LUT_Eavg.png");

    {
        Ref<VertexBuffer> vb = VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
        vb->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
        });
        Ref<IndexBuffer> ib = IndexBuffer::Create(cubeIndices, sizeof(cubeIndices) / sizeof(uint32_t));
        cubeVertexArray = VertexArray::Create();
        cubeVertexArray->AddVertexBuffer(vb);
        cubeVertexArray->SetIndexBuffer(ib);
        Renderer::GetShaderLibrary()->Load("assets/shaders/Texture.glsl");
        m_CheckerboardTex = Texture2D::Create("assets/textures/Checkerboard.png");
        textureShader = Renderer::GetShaderLibrary()->Get("Texture");
        UniformBufferDeclaration<sizeof(glm::vec3) + sizeof(int32_t), 2> uniformBuffer;
        uniformBuffer.Push("u_Texture", 0);
        uniformBuffer.Push("u_ColorTint", glm::vec3(0.3f));
        textureShader->BindImmediatly();
        textureShader->UploadUniformBufferImmediatly(uniformBuffer);
        textureShader->SaveUniformBuffer(uniformBuffer);
    }

    Renderer::GetShaderLibrary()->Load("assets/shaders/SimplePBR.glsl");
    m_Shader = Renderer::GetShaderLibrary()->Get("SimplePBR");
    m_Mesh = CreateRef<Mesh>("assets/meshes/cerberus.fbx");
    //m_Mesh = CreateRef<Mesh>("assets/models/nanosuit/nanosuit.obj");
    //m_Mesh = CreateRef<Mesh>("assets/models/sphere.fbx");

    {
        const uint32_t slots = 9;
        const uint32_t size = sizeof(int32_t) * slots;
        UniformBufferDeclaration<size, slots> uniformBuffer;
        uniformBuffer.Push("u_AlbedoTexture", 1);
        uniformBuffer.Push("u_NormalTexture", 2);
        uniformBuffer.Push("u_MetalnessTexture", 3);
        uniformBuffer.Push("u_RoughnessTexture", 4);
        uniformBuffer.Push("u_BRDFLUTTexture", 5);
        uniformBuffer.Push("u_EnvRadianceTex", 6);
        uniformBuffer.Push("u_EnvIrradianceTex", 7);
        uniformBuffer.Push("u_EmuLUT", 8);
        uniformBuffer.Push("u_EavgLUT", 9);
        m_Shader->BindImmediatly();
        m_Shader->UploadUniformBufferImmediatly(uniformBuffer);
        m_Shader->UnbindImmediatly();
        m_Shader->SaveUniformBuffer(uniformBuffer);
    }

    {
        TextureSpecification specification;
        specification.Filter = TextureFilter::MipMapLinear;
        specification.Wrap = TextureWrap::Clamp;
        specification.GenMips = true;
        m_EnvRadiance = TextureCube::Create("assets/textures/environments/Arches_E_PineTree_Radiance.tga", specification);
        //m_EnvRadiance = TextureCube::Create("assets/textures/sky_ocean", {"posx", "negx", "posy", "negy", "posz", "negz"}, "jpg");
        m_EnvIrradiance = TextureCube::Create("assets/textures/environments/Arches_E_PineTree_Irradiance.tga", specification);
        //m_EnvRadiance = TextureCube::Create("assets/textures/sky_ocean", {"posx", "negx", "posy", "negy", "posz", "negz"}, "jpg");
    }
    //{
    //    TextureSpecification specification;
    //    specification.SRGB = true;
    //    m_AlbedoTex = Texture2D::Create("assets/textures/cerberus/cerberus_A.png", specification);
    //}
    //m_NormalTex = Texture2D::Create("assets/textures/cerberus/cerberus_N.png");
    //m_MetalnessTex = Texture2D::Create("assets/textures/cerberus/cerberus_M.png");
    //m_RoughnessTex = Texture2D::Create("assets/textures/cerberus/cerberus_R.png");
    m_BRDFLUT = Texture2D::Create("assets/textures/BRDF_LUT.tga");
    // m_BRDFLUT = Texture2D::Create("assets/textures/environments/Arches_E_PineTree_Radiance.tga");

    m_Light.Position = {0.0f, 0.0f, 0.0f};
    m_Light.Direction = {-0.5f, 1.0f, 1.0f};
    m_Light.Color = {1.0f, 1.0f, 1.0f};
    m_Light.Intensity = 1.0f;

    {
        s_SkyboxVA = VertexArray::Create();
        Ref<VertexBuffer> vb = VertexBuffer::Create(s_SkyboxVt, sizeof(s_SkyboxVt));
        vb->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
        });
        Ref<IndexBuffer> ib = IndexBuffer::Create(s_SkyboxId, sizeof(s_SkyboxId) / sizeof(uint32_t));
        s_SkyboxVA->AddVertexBuffer(vb);
        s_SkyboxVA->SetIndexBuffer(ib);
        Renderer::GetShaderLibrary()->Load("assets/shaders/Skybox.glsl");
        s_SkyboxShader = Renderer::GetShaderLibrary()->Get("Skybox");
        UniformBufferDeclaration<sizeof(int32_t), 1> uniformBuffer;
        uniformBuffer.Push("u_EnvTexture", 0);
        s_SkyboxShader->BindImmediatly();
        s_SkyboxShader->UploadUniformBufferImmediatly(uniformBuffer);
        s_SkyboxShader->SaveUniformBuffer(uniformBuffer);
    }

    {
        s_QuadVA = VertexArray::Create();
        Ref<VertexBuffer> vb = VertexBuffer::Create(s_QuadVertices, sizeof(s_QuadVertices));
        vb->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
        });
        Ref<IndexBuffer> ib = IndexBuffer::Create(s_QuadIndices, sizeof(s_QuadIndices) / sizeof(uint32_t));
        s_QuadVA->AddVertexBuffer(vb);
        s_QuadVA->SetIndexBuffer(ib);
        Renderer::GetShaderLibrary()->Load("assets/shaders/HDR.glsl");
        s_QuadShader = Renderer::GetShaderLibrary()->Get("HDR");
        UniformBufferDeclaration<sizeof(int32_t), 1> uniformBuffer;
        uniformBuffer.Push("u_Texture", 0);
        s_QuadShader->BindImmediatly();
        s_QuadShader->UploadUniformBufferImmediatly(uniformBuffer);
        s_QuadShader->SaveUniformBuffer(uniformBuffer);
    }

    {
        Renderer::GetShaderLibrary()->Load("assets/shaders/Position.glsl");
        m_GShader = Renderer::GetShaderLibrary()->Get("Position");
        Renderer::GetShaderLibrary()->Load("assets/shaders/Normal.glsl");

        Renderer::GetShaderLibrary()->Load("assets/shaders/SSAO.glsl");
        m_SSAOShader = Renderer::GetShaderLibrary()->Get("SSAO");
        UniformBufferDeclaration<sizeof(int32_t) * 3, 3> uniformBuffer;
        uniformBuffer.Push("u_PositionBuffer", 0);
        uniformBuffer.Push("u_NormalBuffer", 1);
        uniformBuffer.Push("u_NoiseTexture", 2);
        m_SSAOShader->BindImmediatly();
        m_SSAOShader->UploadUniformBufferImmediatly(uniformBuffer);
        m_SSAOShader->SaveUniformBuffer(uniformBuffer);
    }

    {
        // calculate ssao noise texture
        int size = 4;
        std::vector<glm::vec3> ssaoNoise(size * size);
        for (int i = 0; i < size * size; i++)
            ssaoNoise[i] = { RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f, 0.0f };

        TextureSpecification sp;
        sp.Format = ImageFormat::RGB16F;
        m_SSAONoise = Texture2D::Create(size, size, sp);
        m_SSAONoise->SetData(ssaoNoise.data(), sizeof(glm::vec3) * ssaoNoise.size());

        //calculate ssao kernels
        uint8_t kernelSize = 64;
        s_SSAOKernel.reserve(kernelSize);
        for (int i = 0; i < kernelSize; i++)
        {
            auto sample = glm::normalize(glm::vec3(RandomFloat() * 2.0f - 1.0f, RandomFloat() * 2.0f - 1.0f, RandomFloat()));
            float scale = float(i) / kernelSize;
            scale = Lerp(0.1, 1.0, scale * scale);
            s_SSAOKernel.push_back(sample * scale);
        }
        m_SSAOShader->SetFloat3Array("u_Samples", s_SSAOKernel.data(), kernelSize);
    }
}

void PBRLayer::OnUpdate(Timestep ts)
{
    m_Camera.OnUpdate(ts);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
    model = glm::scale(glm::mat4(1.0f), 0.02f * s_Scale);
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

    int index = 0;

    {
        m_RenderPasses[index].FrameBuffer->Bind();
        Renderer::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        Renderer::Clear();

        const uint32_t matrices = 3;
        const uint32_t size = sizeof(glm::mat4) * matrices;
        UniformBufferDeclaration<size, matrices> uniformBuffer;
        uniformBuffer.Push("u_Model", model);
        uniformBuffer.Push("u_View", m_Camera.GetViewMatrix());
        uniformBuffer.Push("u_Projection", m_Camera.GetProjectionMatrix());
        m_GShader->Bind();
        m_GShader->UploadUniformBuffer(uniformBuffer);
        Renderer::Submit(m_Mesh->GetVertexArray(), true);
    }

    {
        index++;
        m_RenderPasses[index].FrameBuffer->Bind();
        Renderer::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        Renderer::Clear();

        const uint32_t matrices = 4;
        const uint32_t size = sizeof(glm::mat4) * matrices;
        UniformBufferDeclaration<size, matrices> uniformBuffer;
        uniformBuffer.Push("u_Model", model);
        uniformBuffer.Push("u_View", m_Camera.GetViewMatrix());
        uniformBuffer.Push("u_Projection", m_Camera.GetProjectionMatrix());
        uniformBuffer.Push("u_NormalMatrix", glm::transpose(glm::inverse(m_Camera.GetViewMatrix() * model)));
        Renderer::GetShaderLibrary()->Get("Normal")->Bind();
        Renderer::GetShaderLibrary()->Get("Normal")->UploadUniformBuffer(uniformBuffer);
        Renderer::Submit(m_Mesh->GetVertexArray(), true);
    }
    
    {
        index++;
        m_RenderPasses[index].FrameBuffer->Bind();
        Renderer::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        Renderer::Clear();

        m_SSAOShader->Bind();
        Texture2D::BindByID(m_RenderPasses[index - 2].FrameBuffer->GetColorAttachmentRendererID(), 0);
        Texture2D::BindByID(m_RenderPasses[index - 1].FrameBuffer->GetColorAttachmentRendererID(), 1);
        m_SSAONoise->Bind(2);

        // get window
        auto &window = Application::Get().GetWindow();
        //upload window width and height
        UniformBufferDeclaration<sizeof(int32_t) * 2 + sizeof(glm::mat4), 3> uniformBuffer;
        uniformBuffer.Push("u_WindowWidth", (int)window.GetWidth());
        uniformBuffer.Push("u_WindowHeight", (int)window.GetHeight());
        uniformBuffer.Push("u_Projection", m_Camera.GetProjectionMatrix());
        Renderer::Submit(s_QuadVA, false);
    }

    {
        index++;
        m_RenderPasses[index].FrameBuffer->Bind();
        Renderer::SetClearColor({ 0.2f, 0.1f, 0.11f, 1.f });
        Renderer::Clear();
        //Renderer::StartScene((Camera &)m_Camera);

        if (m_ReceiveEnv)
        {
            UniformBufferDeclaration<sizeof(glm::mat4) + sizeof(float), 2> uniformBuffer;
            uniformBuffer.Push("u_ViewProjection", m_Camera.GetProjectionMatrix() * glm::mat4(glm::mat3(m_Camera.GetViewMatrix())));
            uniformBuffer.Push("u_EnvMapRotation", m_EnvMapRotation);
            s_SkyboxShader->Bind();
            s_SkyboxShader->UploadUniformBuffer(uniformBuffer);
            m_EnvRadiance->Bind(0);
            Renderer::Submit(s_SkyboxVA, false);
        }

        {
            const uint32_t size = sizeof(glm::mat4) * 3 + sizeof(glm::vec3) * 5 + sizeof(float) * 7 + sizeof(int32_t) * 5;
            UniformBufferDeclaration<size, 20> uniformBuffer;
            uniformBuffer.Push("u_Model", model);
            uniformBuffer.Push("u_ViewProjection", m_Camera.GetViewProjectionMatrix());
            uniformBuffer.Push("u_NormalMatrix", normalMatrix);

            uniformBuffer.Push("u_LightDirection", glm::normalize(m_Light.Direction));
            uniformBuffer.Push("u_LightColor", m_Light.Color);
            uniformBuffer.Push("u_Albedo", m_Albedo);
            uniformBuffer.Push("u_AlbedoTint", m_AlbedoTint);
            uniformBuffer.Push("u_CameraPosition", m_Camera.GetPosition());

            uniformBuffer.Push("u_UseAlbedoTex", m_UseAlbedoTex ? 1 : 0);
            uniformBuffer.Push("u_UseNormalTex", m_UseNormalTex ? 1 : 0);
            uniformBuffer.Push("u_UseMetalnessTex", m_UseMetalnessTex ? 1 : 0);
            uniformBuffer.Push("u_UseRoughnessTex", m_UseRoughnessTex ? 1 : 0);
            uniformBuffer.Push("u_ReceiveEnvLight", m_ReceiveEnv ? 1 : 0);

            uniformBuffer.Push("u_LightIntensity", m_Light.Intensity);
            uniformBuffer.Push("u_NormalStrengh", 1.0f / m_NormalStrengh);
            uniformBuffer.Push("u_Fresnel", m_Fresnel);
            uniformBuffer.Push("u_Metalness", m_Metalness);
            uniformBuffer.Push("u_Roughness", m_Roughness);
            uniformBuffer.Push("u_RadiancePrefilter", m_RadiancePrefilter);
            uniformBuffer.Push("u_EnvMapRotation", m_EnvMapRotation);
            m_Shader->Bind();
            m_Shader->UploadUniformBuffer(uniformBuffer);

            if (m_AlbedoTex) m_AlbedoTex->Bind(1);
            if (m_NormalTex) m_NormalTex->Bind(2);
            if (m_MetalnessTex) m_MetalnessTex->Bind(3);
            if (m_RoughnessTex) m_RoughnessTex->Bind(4);
            m_BRDFLUT->Bind(5);
            m_EnvRadiance->Bind(6);
            m_EnvIrradiance->Bind(7);
            m_EmuLUT->Bind(8);
            m_EavgLUT->Bind(9);
            Renderer::Submit(m_Mesh->GetVertexArray(), true);
        }

        model = glm::translate(glm::mat4(1.0f), { 0.0f, -1.0f, 0.0f }) * glm::scale(glm::mat4(1.0f), { 5.0f, 0.1f, 5.0f });

        {
            UniformBufferDeclaration<sizeof(glm::mat4) * 2, 2> uniformBuffer;
            uniformBuffer.Push("u_Model", model);
            uniformBuffer.Push("u_ViewProjection", m_Camera.GetViewProjectionMatrix());
            textureShader->Bind();
            textureShader->UploadUniformBuffer(uniformBuffer);
            m_CheckerboardTex->Bind(0);
            Renderer::Submit(cubeVertexArray);
        }
    }

    {
        index++;
        m_RenderPasses[index].FrameBuffer->Bind();
        Renderer::SetClearColor({ 0.2f, 0.1f, 0.11f, 1.f });
        Renderer::Clear();
        s_QuadShader->Bind();
        UniformBufferDeclaration<sizeof(float) + sizeof(int) * 2, 3> uniformBuffer;
        uniformBuffer.Push("u_Exposure", s_Exposure);
        uniformBuffer.Push("u_UseHDR", s_UseHDR ? 1 : 0);
        uniformBuffer.Push("u_UseGamma", s_UseGamma ? 1 : 0);
        s_QuadShader->UploadUniformBuffer(uniformBuffer);
        Texture2D::BindByID(m_RenderPasses[index - 1].FrameBuffer->GetColorAttachmentRendererID(), 0);
        Renderer::Submit(s_QuadVA, false);
        m_RenderPasses[index].FrameBuffer->Unbind();
    }

    Renderer::EndScene();
}

void PBRLayer::OnEvent(Event &e)
{
    m_Camera.OnEvent(e);
}

void PBRLayer::OnImGuiRender()
{
    static bool p_open = true;

    static bool opt_fullscreen_persistant = true;
    static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
    bool opt_fullscreen = opt_fullscreen_persistant;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    // When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Dockspace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
    }
    ImGui::Begin("Settings");
		if (ImGui::TreeNode("Shaders"))
		{
            const auto& shaders = Renderer::GetShaderLibrary()->GetShaders();
            for (const auto& [name, shader] : shaders)
            {
                /*if (ImGui::TreeNode(name.c_str()))
                {
                    std::string buttonName = "Reload##" + name;
                    if (ImGui::Button(buttonName.c_str()))
                        shader->Reload();
                    ImGui::TreePop();
                }*/
                ImGui::Text(name.c_str());
                std::string buttonName = "Reload##" + name;
                ImGui::SameLine();
                if (ImGui::Button(buttonName.c_str()))
                    shader->Reload();
            }
            ImGui::TreePop();
		}
        /*ImGui::RadioButton("Spheres", (int*)&m_Scene, (int)Scene::Spheres);
		ImGui::SameLine();
		ImGui::RadioButton("Model", (int*)&m_Scene, (int)Scene::Model);

		ImGui::ColorEdit4("Clear Color", m_ClearColor);

		ImGui::SliderFloat3("Light Dir", glm::value_ptr(m_Light.Direction), -1, 1);
		ImGui::ColorEdit3("Light Radiance", glm::value_ptr(m_Light.Radiance));
		ImGui::SliderFloat("Light Multiplier", &m_LightMultiplier, 0.0f, 5.0f);
		ImGui::SliderFloat("Exposure", &m_Exposure, 0.0f, 10.0f);
		auto cameraForward = m_Camera.GetForwardDirection();
		ImGui::Text("Camera Forward: %.2f, %.2f, %.2f", cameraForward.x, cameraForward.y, cameraForward.z);*/

		ImGui::Separator();
		{
			ImGui::Text("Mesh");
			std::string fullpath = m_Mesh ? m_Mesh->GetFilePath() : "None";
			size_t found = fullpath.find_last_of("/\\");
			std::string path = found != std::string::npos ? fullpath.substr(found + 1) : fullpath;
			ImGui::Text(path.c_str()); ImGui::SameLine();
			if (ImGui::Button("...##Mesh"))
			{
				std::string filename = Application::Get().OpenFile("");
                if (filename != "")
                    m_Mesh = CreateRef<Mesh>(filename);
			}
		}
		ImGui::Separator();

		ImGui::Text("Shader Parameters");
		//ImGui::Checkbox("Radiance Prefiltering", &m_RadiancePrefilter);

		ImGui::Separator();

		// Textures ------------------------------------------------------------------------------
		{
			// Albedo
			if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_AlbedoTex? (void*)m_AlbedoTex->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_AlbedoTex)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_AlbedoTex->GetFilePath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_AlbedoTex->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
                        if (filename != "")
                        {
                            m_UseAlbedoTex = true;
                            TextureSpecification sp;
                            sp.SRGB = true;
                            m_AlbedoTex = Texture2D::Create(filename, sp);
                        }
					}
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Checkbox("Use##AlbedoMap", &m_UseAlbedoTex);
				if (ImGui::Checkbox("sRGB##AlbedoMap", &m_AlbedoSRGB))
				{
					if (m_AlbedoTex)
						{
                            TextureSpecification sp;
                            sp.SRGB = m_AlbedoSRGB;
                            m_AlbedoTex = Texture2D::Create(m_AlbedoTex->GetFilePath(), sp);
                        }
				}
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::ColorEdit3("PureColor##Albedo", glm::value_ptr(m_Albedo), ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                ImGui::ColorEdit3("ColorTint##Albedo", glm::value_ptr(m_AlbedoTint), ImGuiColorEditFlags_NoInputs);
            }
		}
		{
			// Normals
			if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_NormalTex ? (void*)m_NormalTex->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_NormalTex)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_NormalTex->GetFilePath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_NormalTex->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
                        if (filename != "")
                        {
                            m_UseNormalTex = true;
                            m_NormalTex = Texture2D::Create(filename);
                        }
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##NormalMap", &m_UseNormalTex);
                ImGui::SameLine();
                ImGui::SliderFloat("Value##NormalStrengh", &m_NormalStrengh, 0.01f, 15.0f);
            }
		}
		{
			// Metalness
			if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_MetalnessTex ? (void*)m_MetalnessTex->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_MetalnessTex)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_MetalnessTex->GetFilePath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_MetalnessTex->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
                        if (filename != "")
                        {
                            m_UseMetalnessTex = true;
                            m_MetalnessTex = Texture2D::Create(filename);
                        }
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##MetalnessMap", &m_UseMetalnessTex);
				ImGui::SameLine();
				ImGui::SliderFloat("Value##MetalnessInput", &m_Metalness, 0.0f, 1.0f);
			}
		}
		{
			// Roughness
			if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
				ImGui::Image(m_RoughnessTex ? (void*)m_RoughnessTex->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), ImVec2(64, 64));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered())
				{
					if (m_RoughnessTex)
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted(m_RoughnessTex->GetFilePath().c_str());
						ImGui::PopTextWrapPos();
						ImGui::Image((void*)m_RoughnessTex->GetRendererID(), ImVec2(384, 384));
						ImGui::EndTooltip();
					}
					if (ImGui::IsItemClicked())
					{
						std::string filename = Application::Get().OpenFile("");
                        if (filename != "")
                        {
                            m_UseRoughnessTex = true;
                            m_RoughnessTex = Texture2D::Create(filename);
                        }
					}
				}
				ImGui::SameLine();
				ImGui::Checkbox("Use##RoughnessMap", &m_UseRoughnessTex);
				ImGui::SameLine();
				ImGui::SliderFloat("Value##RoughnessInput", &m_Roughness, 0.01f, 1.0f);
			}
		}

		ImGui::Separator();

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		auto viewportSize = ImGui::GetContentRegionAvail();
        m_RenderPasses[m_FinalPass].FrameBuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		m_Camera.SetProjection(45.0f, viewportSize.x, viewportSize.y, 0.1f, 10000.0f);
		ImGui::Image((void*)m_RenderPasses[m_FinalPass].FrameBuffer->GetColorAttachmentRendererID(), viewportSize, { 0, 1 }, { 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Docking"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Flag: NoSplit", "", (opt_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 opt_flags ^= ImGuiDockNodeFlags_NoSplit;
				if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (opt_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  opt_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
				if (ImGui::MenuItem("Flag: NoResize", "", (opt_flags & ImGuiDockNodeFlags_NoResize) != 0))                opt_flags ^= ImGuiDockNodeFlags_NoResize;
				if (ImGui::MenuItem("Flag: PassthruDockspace", "", (opt_flags & ImGuiDockNodeFlags_PassthruDockspace) != 0))       opt_flags ^= ImGuiDockNodeFlags_PassthruDockspace;
				if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (opt_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          opt_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
				ImGui::Separator();
				if (ImGui::MenuItem("Close DockSpace", NULL, false, p_open != NULL))
					p_open = false;
				ImGui::EndMenu();
			}
			ImGuiShowHelpMarker(
				"You can _always_ dock _any_ window into another by holding the SHIFT key while moving a window. Try it now!" "\n"
				"This demo app has nothing to do with it!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
				"ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
				"(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
			);

			ImGui::EndMenuBar();
		}

		ImGui::End();

    ImGui::Begin("Light");
    ImGui::SliderFloat3("Scale", glm::value_ptr(s_Scale), 0.001f, 5.0f);
    ImGui::SliderFloat3("Light Direction", glm::value_ptr(m_Light.Direction), -1.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", glm::value_ptr(m_Light.Color));
    ImGui::SliderFloat("Light Intensity", &m_Light.Intensity, 0.0f, 5.0f);
    ImGui::SliderFloat("Fresnel", &m_Fresnel, -10.0f, 10.0f);
    ImGui::SliderFloat("RadiancePrefilter", &m_RadiancePrefilter, 0.0f, 1.0f);
    ImGui::SliderFloat("EnvMapRotation", &m_EnvMapRotation, -360.0f, 360.0f);
    ImGui::SliderFloat("Exposure", &s_Exposure, 0.1f, 10.0f);
    ImGui::Checkbox("Use HDR", &s_UseHDR);
    ImGui::Checkbox("Use Gamma", &s_UseGamma);
    ImGui::Checkbox("Receive Environment", &m_ReceiveEnv);
    ImGui::SliderInt("Pass:", &m_FinalPass, 0, m_RenderPasses.size() - 1);
    ImGui::SameLine();
    ImGui::Text(GetFinalPass().Name.c_str());
    if (ImGui::Button("Reload"))
    {
        m_Shader->Reload();
    }
    ImGui::End();
}

//////////////////////////////////////////////////
// OLD
//////////////////////////////////////////////////
Sandbox3D::Sandbox3D()
    : Layer("Sandbox3D")
{ // Camera
    auto &window = Application::Get().GetWindow();
    window.SetVSync(false);

    m_PerspectiveCameraController = PerspectiveCameraController(45, (float)window.GetWidth(), (float)window.GetHeight(), 0.1f, 500.0f);

    // Cube
    m_CubeVA = VertexArray::Create();
    Renderer::GetShaderLibrary()->Load("assets/shaders/Texture.glsl");

    {
        Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(cubeVertices, sizeof(cubeVertices));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
        });
        Ref<IndexBuffer> indexBuffer(IndexBuffer::Create(cubeIndices, sizeof(cubeIndices) / sizeof(uint32_t)));
        m_CubeVA->AddVertexBuffer(vertexBuffer);
        m_CubeVA->SetIndexBuffer(indexBuffer);
    }

    TextureSpecification specification;
    specification.SRGB = true;
    // m_DefaultTexture = Texture2D::Create("assets/textures/Checkerboard.png", { ImageFormat::RGB });
    m_DefaultTexture = Texture2D::Create("assets/textures/Checkerboard.png", specification);
    m_LogoTexture = Texture2D::Create("assets/textures/ChernoLogo.png", {ImageFormat::RGB});
}

void Sandbox3D::OnUpdate(Timestep ts)
{
    m_PerspectiveCameraController.OnUpdate(ts);

    // RenderCommand::SetClearColor(0.1f, 0.1f, 0.11f, 1.f);
    // RenderCommand::Clear();

    Renderer::SetClearColor({0.1f, 0.1f, 0.11f, 1.f});
    Renderer::Clear();
    Renderer::StartScene(m_PerspectiveCameraController.GetCamera());

    // m_Camera->SetRotation(m_Camera->GetRotation() + glm::vec3(0.0f, 0.0f, 1.0f));

    const auto &textureShader = Renderer::GetShaderLibrary()->Get("Texture");

    const uint32_t size = sizeof(glm::vec3) + sizeof(int32_t);
    UniformBufferDeclaration<size, 2> uniformBuffer;
    uniformBuffer.Push("u_ColorTint", m_CubeColorTint);
    uniformBuffer.Push("u_Texture", 0);
    textureShader->Bind();
    textureShader->UploadUniformBuffer(uniformBuffer);

    m_DefaultTexture->Bind();

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    for (int y = -1; y <= 1; y++)
    {
        position.y = y * 1.2f;
        for (int x = -1; x <= 1; x++)
        {
            position.x = x * 1.2f;
            Renderer::Submit(m_CubeVA);
        }
    }

    Renderer::EndScene();
}

void Sandbox3D::OnEvent(Event &e)
{
    m_PerspectiveCameraController.OnEvent(e);
}

void Sandbox3D::OnImGuiRender()
{
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color Tint", glm::value_ptr(m_CubeColorTint));
    ImGui::End();
}
