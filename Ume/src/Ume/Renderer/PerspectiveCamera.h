#pragma once

#include "Camera.h"
#include "Ume/Core/Timestep.h"
#include "Ume/Events/MouseEvent.h"
#include "Ume/Events/ApplicationEvent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
namespace Ume
{
	class PerspectiveCamera : Camera
	{
	public:
		inline virtual const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	public:
		PerspectiveCamera() = default;
		~PerspectiveCamera() = default;
		PerspectiveCamera(float fov, float width, float height, float zNear, float zFar);
	public:
		void SetProjection(float fov, float width, float height, float zNear, float zFar);
		void Focus(const glm::vec3& point);
		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		void UpdateEulerAngles(const glm::vec2& delta);
		void CalculatePosition();
		void CalculateFocalPoint();
		void CalculateOrientation();
		void CalculateMatrices();
		glm::vec3 GetForward();
		glm::vec3 GetUp();
		glm::vec3 GetRight();
	private:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

		glm::vec3 m_FocalPoint = glm::vec3(0.0f);
		glm::vec3 m_Position = glm::vec3(0.0f);

		float m_Pitch = 0.0f, m_Yaw = -90.0f;
		float m_Distance = 5.0f;

		float m_ZoomSpeed = 1.0f;
		float m_RotateSpeed = 4.0f;

		glm::vec2 m_MousePosition = glm::vec2(0.0f);
		glm::quat m_Orientation;

		float m_FOV, m_ZNear, m_ZFar;
	};

	class PerspectiveCameraSample : Camera
	{
	public:
		PerspectiveCameraSample() = default;
		PerspectiveCameraSample(const glm::mat4& projectionMatrix);

		void Focus();
		void Update();

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

		virtual const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }

		glm::vec3 GetUpDirection();
		glm::vec3 GetRightDirection();
		glm::vec3 GetForwardDirection();
		const glm::vec3& GetPosition() const { return m_Position; }
	private:
		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition();
		glm::quat GetOrientation();
	private:
		glm::mat4 m_ProjectionMatrix, m_ViewMatrix, m_ViewProjectionMatrix;
		glm::vec3 m_Position, m_Rotation, m_FocalPoint;

		bool m_Panning, m_Rotating;
		glm::vec2 m_InitialMousePosition;
		glm::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_PanSpeed, m_RotationSpeed, m_ZoomSpeed;

		float m_Pitch, m_Yaw;
	};

	class PerspectiveCameraOld : public Camera
	{
	public:
		PerspectiveCameraOld() = default;
		PerspectiveCameraOld(float fov, float width, float height, float zNear, float zFar);
		~PerspectiveCameraOld() = default;

		void SetPosition(const glm::vec3& position) { m_Position = position; CalculateViewMatrix(); }
		void SetRotation(const glm::vec3& rotation);

		virtual void CalculateViewMatrix();

		inline const glm::vec3& GetPosition() const { return m_Position; }
		virtual inline const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }
	public:
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::vec3& GetForward() const { return m_Forward; }
		inline const glm::vec3& GetUp() const { return m_Up; }
		void SetProjection(float fov, float width, float height, float zNear, float zFar);
	private:
	private:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

		glm::vec3 m_Forward = { 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
	};
}
