#include <native/core/scene/camera.h>

// =============== //
// ABSTRACT CAMERA //
// =============== //

glm::mat4 Camera::get_view_matrix() const
{
	return glm::lookAt(
		this->m_cam_vectors.position,
		this->m_cam_vectors.position + this->m_cam_vectors.front,
		this->m_cam_vectors.up
	);
}

glm::mat4 Camera::get_projection_matrix() const
{
	auto proj = glm::perspective(
		glm::radians(this->m_options.fov),
		this->m_options.aspect_ratio,
		this->m_options.near_plane,
		this->m_options.far_plane
	);

	// We're flipping the y-axis because glm assumes +y to be up, but vulkan assumes +y to be down
	proj[1][1] *= -1;
	return proj;
}

glm::mat4 Camera::get_view_projection() const
{
	return get_projection_matrix() * get_view_matrix();
}

void Camera::set_position(glm::vec3 pos)
{
	this->m_cam_vectors.position = pos;
	update_vectors();
}

void Camera::look_at(glm::vec3 target)
{
	glm::vec3 direction = glm::normalize(target - this->m_cam_vectors.position);

	this->m_options.pitch = glm::degrees(asin(direction.y));
	this->m_options.yaw = glm::degrees(atan2(direction.z, direction.x));

	update_vectors();
}

void Camera::set_aspect_ratio(float width, float height)
{
	if (height > 0)
	{
		this->m_options.aspect_ratio = width / height;
	}
}

Camera::Camera(CameraType type, glm::vec3 position, float fov) : m_type(type)
{
	this->m_cam_vectors.position = position;
	this->m_options.fov = fov;

	update_vectors();
}

void Camera::update_vectors()
{
	auto yaw = this->m_options.yaw;
	auto pitch = this->m_options.pitch;

	glm::vec3 f{}; // very creative name
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	auto& front = this->m_cam_vectors.front;
	auto& right = this->m_cam_vectors.right;
	auto& up = this->m_cam_vectors.up;
	
	front = glm::normalize(f);
	right = glm::normalize(glm::cross(front, up));
	up = glm::normalize(glm::cross(right, front));
}

// ============= //
// STATIC CAMERA //
// ============= //

StaticCamera::StaticCamera(glm::vec3 position, glm::vec3 target) : Camera(CameraType::Static, position)
{
	look_at(target);
}

void StaticCamera::update(double dt)
{ 
	return;
}

UserCamera::UserCamera(glm::vec3 position, GLFWwindow* glfw_window) : Camera(CameraType::User, position)
{
	this->m_glfw_window = glfw_window;
}

void UserCamera::update(double dt)
{
	process_keyboard(dt);
	process_mouse();
}

void UserCamera::process_keyboard(double dt)
{
	float velocity = this->m_speed * (float) dt;
	const glm::vec3& front = this->m_cam_vectors.front;
	glm::vec3& position = this->m_cam_vectors.position;

	if (glfwGetKey(this->m_glfw_window, GLFW_KEY_W) == GLFW_PRESS) position += front * velocity;
	if (glfwGetKey(this->m_glfw_window, GLFW_KEY_S) == GLFW_PRESS) position -= front * velocity;
	if (glfwGetKey(this->m_glfw_window, GLFW_KEY_A) == GLFW_PRESS) position -= front * velocity;
	if (glfwGetKey(this->m_glfw_window, GLFW_KEY_D) == GLFW_PRESS) position += front * velocity;
}

void UserCamera::process_mouse()
{
	if (glfwGetMouseButton(this->m_glfw_window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
		this->m_first_mouse = true;
		return;
	}

	double x_pos, y_pos;
	glfwGetCursorPos(this->m_glfw_window, &x_pos, &y_pos);

	if (this->m_first_mouse) {
		this->m_last_x = x_pos;
		this->m_last_y = y_pos;
		this->m_first_mouse = false;
	}

	float xoffset = (float)(x_pos - this->m_last_x);
	float yoffset = (float)(this->m_last_y - y_pos);
	this->m_last_x = x_pos;
	this->m_last_y = y_pos;

	xoffset *= m_sensitivity;
	yoffset *= m_sensitivity;

	this->m_options.yaw += xoffset;
	this->m_options.pitch += yoffset;

	this->m_options.pitch = std::clamp(this->m_options.pitch, -89.0f, 89.0f);

	update_vectors();
}

// ============== //
// DYNAMIC CAMERA //
// ============== //

DynamicCamera::DynamicCamera(glm::vec3 start_position) : Camera(CameraType::Dynamic, start_position) { }

void DynamicCamera::add_keyframe(float time, const glm::vec3& world_pos, const glm::vec3& target, KeyframeTransition transition)
{
	this->m_path.emplace_back(world_pos, target, time, transition);
	sort_keyframes();
}

void DynamicCamera::add_keyframe(const Keyframe& keyframe)
{
	this->m_path.push_back(keyframe);
	sort_keyframes();
}

void DynamicCamera::play()
{
	this->m_playing = true;
}

void DynamicCamera::pause()
{
	this->m_playing = false;
}

void DynamicCamera::reset()
{
	this->m_curr_time = 0.0f;
	this->m_curr_index = 0;

	if (!this->m_path.empty()) {
		set_position(this->m_path.front().world_position);
		look_at(this->m_path.front().target);
	}
}

void DynamicCamera::update(double dt)
{
	if (!this->m_playing || this->m_path.empty()) return;

	this->m_curr_time += static_cast<float>(dt);

	if (m_curr_time <= this->m_path.front().time) {
		set_position(this->m_path.front().world_position);
		look_at(this->m_path.front().target);
		return;
	}

	interpolate();
}

float DynamicCamera::ease(float t, KeyframeTransition type)
{
	t = std::clamp(t, 0.0f, 1.0f); // this should never happen, but let's be defensive

	// The tl;dr is that linear is linear, sine is gentler, and cubic is "sharper"
	switch (type)
	{
	case KeyframeTransition::Linear:
		return t;

	case KeyframeTransition::EaseInSine:
		return 1.0f - cos((t * OBS_PI) * 0.5f);

	case KeyframeTransition::EaseOutSine:
		return sin((t * OBS_PI) * 0.5f);

	case KeyframeTransition::EaseInOutSine:
		return -(cos(OBS_PI * t) - 1.0f) * 0.5f;

	case KeyframeTransition::EaseInCubic:
		return t * t * t;

	case KeyframeTransition::EaseOutCubic:
		return 1.0f - pow(1.0f - t, 3.0f);

	case KeyframeTransition::EaseInOutCubic:
		return t < 0.5f
			? 4.0f * t * t * t
			: 1.0f - pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;

	default:
		return t;
	}

	return t;
}

void DynamicCamera::interpolate()
{
	if (this->m_path.size() < 2) return;

	// In case the time overflows and is negative, let's reset
	if (m_curr_time < m_path[m_curr_index].time) {
		reset();
		return;
	}

	// Snap to last and stop
	if (m_curr_index >= m_path.size() - 1) {
		set_position(m_path.back().world_position);
		look_at(m_path.back().target);
		return;
	}

	// Again, let's be defensive and skip ahead if the time is already past the next segment
	while (m_curr_index < m_path.size() - 1 &&
		m_curr_time >= m_path[m_curr_index + 1].time)
	{
		m_curr_index++;
	}

	const Keyframe& frame_start = m_path[m_curr_index];
	const Keyframe& frame_end = m_path[m_curr_index + 1];

	float segment_delta = frame_end.time - frame_start.time;
	if (segment_delta <= OBS_EPSILON) {
		set_position(frame_end.world_position);
		return;
	}

	float t = (m_curr_time - frame_start.time) / segment_delta;
	t = ease(t, frame_end.transition);

	glm::vec3 new_pos = glm::mix(frame_start.world_position, frame_end.world_position, t);
	glm::vec3 new_target = glm::mix(frame_start.target, frame_end.target, t);

	set_position(new_pos);
	look_at(new_target);
}

void DynamicCamera::sort_keyframes()
{
	std::sort(m_path.begin(), m_path.end(),
		[](const Keyframe& a, const Keyframe& b) {
			return a.time < b.time;
		}
	);
}