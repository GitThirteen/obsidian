#pragma once

#include <native/core/include.h>

template<size_t N = 2>
class FrameManager
{
public:
	FrameManager(ResourceManager& resources) : m_resources(resources) { }

	auto initialize() -> void
	{
		assert(m_resources.command_pool());

		auto& pool = m_resources.command_pool();
		for (auto& frame : m_frames)
		{
			frame = pool->alloc_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		}
	}

	auto ready() -> bool
	{
		auto [success, image_index] = m_resources.next_image(m_curr_frame_index);

		if (!success)
		{
			return false;
		}

		m_curr_image_index = image_index;
		return true;
	}

	auto submit() -> void
	{
		auto& cmd_buffer = current_command_buffer();

		m_resources.submit(
			m_curr_frame_index,
			m_curr_image_index,
			cmd_buffer
		);

		m_curr_frame_index = (m_curr_frame_index + 1) % N;
	}

	auto current_command_buffer() -> avk::command_buffer&
	{
		return this->m_frames[this->m_curr_frame_index];
	}

	auto current_image_index() const -> uint32_t { return m_curr_image_index; }
	auto current_frame_index() const -> uint32_t { return m_curr_frame_index; }

private:
	ResourceManager& m_resources;
	std::array<avk::command_buffer, N> m_frames;
	uint32_t m_curr_frame_index = 0;
	uint32_t m_curr_image_index = 0;
};