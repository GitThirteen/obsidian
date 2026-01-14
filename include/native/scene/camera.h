#pragma once

#include <native/core/include.h>

enum class CameraType
{
	Static,		// A static camera cannot be moved. Once defined, it stays at a single place.
	Arcball,	// An arcball camera can be rotated around a fixed point in the scene using the mouse.
	User,		// A user camera can be moved by the user using WASD keys + mouse.
	Dynamic		// A dynamic camera cannot be moved by the user, but can be used via manual instructions (keyframes).
};

struct CameraOptions
{
	float yaw = -90.0f;
	float pitch = 0.0f;
	float fov = 45.0f;
	float aspect_ratio = 16.0f / 9.0f;
	float near_plane = 0.1f;
};

struct CameraVectors
{
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
};

// Camera Base Class

class Camera
{
public:
	virtual ~Camera() = default;

	glm::mat4 get_view_matrix() const;
	glm::mat4 get_projection_matrix() const;
	glm::mat4 get_view_projection() const;

	virtual void update(double deltaTime) = 0;

	void set_position(glm::vec3 pos);
	void look_at(glm::vec3 target);
	void set_aspect_ratio(float width, float height);
	auto position() const -> const glm::vec3 { return m_cam_vectors.position; }

protected:
	CameraType m_type;
	CameraVectors m_cam_vectors;
	CameraOptions m_options;

	Camera(CameraType type, glm::vec3 position, float fov = 45.0f);
	void update_vectors();
};

// Static Camera

class StaticCamera : public Camera
{
public:
	StaticCamera(glm::vec3 position, glm::vec3 target);
	void update(double dt) override;
};

// Arcball Camera

class ArcballCamera : public Camera
{
public:
	ArcballCamera(glm::vec3 target, float radius, GLFWwindow* glfw_window);

	void update(double dt) override;

private:
	GLFWwindow* m_glfw_window;

	glm::vec2 spherical;

	float m_rotate_speed = 0.2f;
	float m_zoom_speed = 0.05f;

	float m_min_radius = 0.5f;
	float m_max_radius = 50.0f;

	glm::vec3 m_target;
	float m_radius;

	bool m_rotating = false;
	bool m_zooming = false;
	double m_last_x = 0.0;
	double m_last_y = 0.0;

	void process_input();
	void update_position_from_spherical();
};

// User Camera

class UserCamera : public Camera
{
public:
	UserCamera(glm::vec3 position, GLFWwindow* glfw_window);
	void update(double dt) override;

	float m_speed = 5.0f;
	float m_sensitivity = 0.1f;

private:
	GLFWwindow* m_glfw_window;
	
	bool m_first_mouse = true;
	double m_last_x = 0.0;
	double m_last_y = 0.0;

	// These 2 should go into the eventmanager
	void process_keyboard(double dt);
	void process_mouse();
};

// Dynamic Camera

enum class KeyframeTransition
{
	Linear,
	EaseInSine,
	EaseInCubic,
	EaseOutSine,
	EaseOutCubic,
	EaseInOutSine,
	EaseInOutCubic
};

struct Keyframe
{
	glm::vec3 world_position;
	glm::vec3 target;
	float time; // in seconds
	KeyframeTransition transition; // the transition to this keyframe
};

class DynamicCamera : public Camera
{
public:
	DynamicCamera(glm::vec3 start_position);

	void add_keyframe(float time, const glm::vec3& world_pos, const glm::vec3& target, KeyframeTransition transition);
	void add_keyframe(const Keyframe& keyframe);

	void play();
	void pause();
	void reset();

	void update(double dt) override;

private:
	std::vector<Keyframe> m_path;

	bool m_playing = false;
	float m_curr_time = 0.0f;
	size_t m_curr_index = 0;

	void interpolate();
	static float ease(float t, KeyframeTransition type);
	void sort_keyframes();
};

struct CameraHandler
{
	std::vector<std::unique_ptr<Camera>> cameras;
	size_t m_curr_index = 0;
};