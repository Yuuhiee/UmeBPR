#include "umepch.h"
#include "PerspectiveCamera.h"

#include "Ume/Core/Input.h"
#include "Ume/Core/KeyCodes.h"
#include "Ume/Core/MouseButtonCodes.h"

#define M_PI 3.14159f

namespace Ume
{
    ///////////////////////////////////
    // Mine
    ///////////////////////////////////
    PerspectiveCamera::PerspectiveCamera(float fov, float width, float height, float zNear, float zFar)
        : m_ProjectionMatrix(glm::perspectiveFov(glm::radians(fov), width, height, zNear, zFar)), m_FOV(fov), m_ZNear(zNear), m_ZFar(zFar)
    {
        CalculateOrientation();
        CalculatePosition();
        CalculateMatrices();

        auto [x, y] = Input::GetMousePosition();
        m_MousePosition = { x, y };

        SetProjection(fov, width, height, zNear, zFar);
    }

    void PerspectiveCamera::Focus(const glm::vec3& point)
    {
    }

    // 左键| 前后平移focal，左右转视角
    // 中键| 上下左右平移focal
    // 右键| 转动视角
    
    void PerspectiveCamera::OnUpdate(Timestep ts)
    {
        auto [x, y] = Input::GetMousePosition();
        glm::vec2 position = { x, y };
        glm::vec2 delta = position - m_MousePosition;
        m_MousePosition = position;

        if (Input::IsKeyPressed(UME_KEY_LEFT_ALT) && Input::IsMouseButtonPressed(0))
        {
            UpdateEulerAngles({ delta.x * m_RotateSpeed * ts, delta.y * m_RotateSpeed * ts * 2 });
            CalculateOrientation();
            CalculatePosition();
            CalculateMatrices();
        }
        else if (Input::IsMouseButtonPressed(1))
        {
            UpdateEulerAngles({ delta.x * m_RotateSpeed * ts * 2, delta.y * m_RotateSpeed * ts * 4 });
            CalculateOrientation();
            CalculateFocalPoint();
            CalculateMatrices();
        }
        else if (Input::IsMouseButtonPressed(2))
        {
            m_Distance += delta.y * m_ZoomSpeed * 0.01f;

            if (m_Distance < 0.1f)
                m_Distance = 0.1f;

            CalculatePosition();
            CalculateMatrices();
        }

    }

    void PerspectiveCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(UME_BIND_EVENT_FN(PerspectiveCamera::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(UME_BIND_EVENT_FN(PerspectiveCamera::OnWindowResized));
    }

    bool PerspectiveCamera::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_Distance -= e.GetYOffset() * m_ZoomSpeed;

        if (m_Distance < 0.1f)
            m_Distance = 0.1f;

        CalculatePosition();
        CalculateMatrices();

        return false;
    }

    bool PerspectiveCamera::OnWindowResized(WindowResizeEvent& e)
    {
        if ((int)e.GetHeight() == 0) return false;
        SetProjection(m_FOV, (float)e.GetWidth(), (float)e.GetHeight(), m_ZNear, m_ZFar);
        return false;
    }

    void PerspectiveCamera::CalculateOrientation()
    {
        m_Orientation = glm::quat(glm::vec3(glm::radians(m_Pitch), glm::radians(m_Yaw), 0.0f));
    }

    glm::vec3 PerspectiveCamera::GetForward()
    {
        return glm::rotate(m_Orientation, glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 PerspectiveCamera::GetUp()
    {
        return glm::rotate(m_Orientation, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 PerspectiveCamera::GetRight()
    {
        return glm::rotate(m_Orientation, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    void PerspectiveCamera::CalculatePosition()
    {
        m_Position = m_FocalPoint - GetForward() * m_Distance;
    }

    void PerspectiveCamera::CalculateFocalPoint()
    {
        m_FocalPoint = m_Position + GetForward() * m_Distance;
    }

    void PerspectiveCamera::CalculateMatrices()
    {
        m_ViewMatrix = glm::lookAt(m_Position, GetForward(), GetUp());
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::SetProjection(float fov, float width, float height, float zNear, float zFar)
    {
        m_ProjectionMatrix = glm::perspectiveFov(glm::radians(fov), width, height, zNear, zFar);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::UpdateEulerAngles(const glm::vec2& delta)
    {
        m_Yaw -= delta.x;
        m_Pitch -= delta.y;

        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        else if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        if (m_Yaw >= 360.0f)
            m_Yaw -= 360.0f;
        else if (m_Yaw <= -360.0f)
            m_Yaw += 360.0f;
    }

    ///////////////////////////////////
    // Sample
    ///////////////////////////////////

    PerspectiveCameraSample::PerspectiveCameraSample(const glm::mat4& projectionMatrix)
        : m_ProjectionMatrix(projectionMatrix)
    {
        // Sensible defaults
        m_PanSpeed = 0.0015f;
        m_RotationSpeed = 0.002f;
        m_ZoomSpeed = 0.2f;

        m_Position = { -100, 100, 100 };
        m_Rotation = glm::vec3(90.0f, 0.0f, 0.0f);

        m_FocalPoint = glm::vec3(0.0f);
        m_Distance = glm::distance(m_Position, m_FocalPoint);

        m_Yaw = 3.0f * (float)M_PI / 4.0f;
        m_Pitch = M_PI / 4.0f;
    }

    void PerspectiveCameraSample::Focus()
    {
    }

    void PerspectiveCameraSample::Update()
    {
        if (Input::IsKeyPressed(UME_KEY_LEFT_ALT))
        {
            const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
            glm::vec2 delta = mouse - m_InitialMousePosition;
            m_InitialMousePosition = mouse;

            if (Input::IsMouseButtonPressed(UME_MOUSE_BUTTON_MIDDLE))
                MousePan(delta);
            else if (Input::IsMouseButtonPressed(UME_MOUSE_BUTTON_LEFT))
                MouseRotate(delta);
            else if (Input::IsMouseButtonPressed(UME_MOUSE_BUTTON_RIGHT))
                MouseZoom(delta.y);
        }

        m_Position = CalculatePosition();

        glm::quat orientation = GetOrientation();
        m_Rotation = glm::eulerAngles(orientation) * (180.0f / (float)M_PI);
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)) * glm::toMat4(glm::conjugate(orientation)) * glm::translate(glm::mat4(1.0f), -m_Position);
    }

    void PerspectiveCameraSample::MousePan(const glm::vec2& delta)
    {
        m_FocalPoint += -GetRightDirection() * delta.x * m_PanSpeed * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * m_PanSpeed * m_Distance;
    }

    void PerspectiveCameraSample::MouseRotate(const glm::vec2& delta)
    {
        float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yawSign * delta.x * m_RotationSpeed;
        m_Pitch += delta.y * m_RotationSpeed;
    }

    void PerspectiveCameraSample::MouseZoom(float delta)
    {
        m_Distance -= delta * m_ZoomSpeed;
        if (m_Distance < 1.0f)
        {
            m_FocalPoint += GetForwardDirection();
            m_Distance = 1.0f;
        }
    }

    glm::vec3 PerspectiveCameraSample::GetUpDirection()
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 PerspectiveCameraSample::GetRightDirection()
    {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 PerspectiveCameraSample::GetForwardDirection()
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 PerspectiveCameraSample::CalculatePosition()
    {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat PerspectiveCameraSample::GetOrientation()
    {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    /////////////////////////////
    // OldCamera
    /////////////////////////////



    const static glm::vec3 Forwad = { 0.0f, 0.0f, -1.0f };
    const static glm::vec3 Up = { 0.0f, 1.0f, 0.0f };

    PerspectiveCameraOld::PerspectiveCameraOld(float fov, float width, float height, float zNear, float zFar)
    {
        m_ProjectionMatrix = glm::perspectiveFov<float>(glm::radians(fov), width, height, zNear, zFar);
        CalculateViewMatrix();
    }

    void PerspectiveCameraOld::SetRotation(const glm::vec3& rotation)
    {
        glm::mat4 rotationMatrix(1.0f);
        rotationMatrix = glm::rotate<float>(rotationMatrix, glm::radians(-rotation.x), { 1.0f, 0.0f, 0.0f });
        rotationMatrix = glm::rotate<float>(rotationMatrix, glm::radians(-rotation.y), { 0.0f, 1.0f, 0.0f });
        rotationMatrix = glm::rotate<float>(rotationMatrix, glm::radians(-rotation.z), { 0.0f, 0.0f, 1.0f });
        m_Forward = (glm::mat3)rotationMatrix * Forwad;
        m_Up = (glm::mat3)rotationMatrix * Up;

        CalculateViewMatrix();
    }

    void PerspectiveCameraOld::CalculateViewMatrix()
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
    void PerspectiveCameraOld::SetProjection(float fov, float width, float height, float zNear, float zFar)
    {
        if ((int)height == 0) return;
        m_ProjectionMatrix = glm::perspectiveFov<float>(glm::radians(fov), width, height, zNear, zFar);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}
