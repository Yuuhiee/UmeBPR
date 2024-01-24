#include "umepch.h"
#include "Application.h"

#include "Ume/Renderer/Renderer.h"
#include "Ume/Renderer/Framebuffer.h"

#include "Ume/Core/KeyCodes.h"
#include "Ume/Core/Timestep.h"

#include "GLFW/glfw3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>

#include <commdlg.h>

namespace Ume
{
    Application *Application::s_Instance = nullptr;

    Application::Application()
    {
        UME_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = Scope<Window>(Window::Create());
        m_Window->SetEventCallback(UME_BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        Renderer::Init();
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = (float)glfwGetTime();
            Timestep timestep(time - m_LastFrameTime);
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                for (Layer *layer : m_LayerStack)
                    layer->OnUpdate(timestep);
            }

            UME_RENDER_S({ self->RenderImGui(); });

            Renderer::Render();

            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(UME_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<KeyPressedEvent>(UME_BIND_EVENT_FN(Application::OnKeyPressed));
        dispatcher.Dispatch<WindowResizeEvent>(UME_BIND_EVENT_FN(Application::OnWindowResize));

        // UME_CORE_TRACE(e);

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer *layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer *layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        for (auto& fb : FramebufferPool::Get()->GetPool())
            fb->Resize(e.GetWidth(), e.GetHeight());

        return false;
    }

    bool Application::OnKeyPressed(KeyPressedEvent &e)
    {
        if (e.GetKeyCode() == UME_KEY_ESCAPE)
        {
            m_Running = false;
            return true;
        }
        return false;
    }
    void Application::RenderImGui()
    {
        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack)
            layer->OnImGuiRender();
        m_ImGuiLayer->End();
    }

    std::string Application::OpenFile(const std::string& filter) const
    {
        OPENFILENAMEA ofn;       // common dialog box structure
        CHAR szFile[260] = { 0 };       // if using TCHAR macros
        
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        
        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::string();
    }
}