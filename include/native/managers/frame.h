#pragma once

#include <native/core/obsidian/include.h>

template<size_t N = 2>
class FrameManager
{
public:
	FrameManager(Root& root, WindowManager& window) : m_root(root), m_window(window)
	{
		for (auto& frame : this->m_frames)
		{
			frame = this->m_root.command_pool()->alloc_command_buffer(
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit
			);
		}
	}

	bool ready()
	{
		this->m_window.wait_for_fence(this->m_curr_frame_index);
		auto [success, image_index] = this->m_window.acquire_next_image(this->m_curr_frame_index);
		if (!success)
		{
			return false;
		}

		this->m_curr_image_index = image_index;
		this->m_window.reset_fence(this->m_curr_frame_index);
		return true;
	}

	void submit()
	{
		const auto& queue = this->m_root.queue();

		auto wait_semaphore = this->m_window.get_image_available_semaphore(this->m_curr_frame_index);
		auto sign_semaphore = this->m_window.get_render_finished_semaphore(this->m_curr_image_index);
		auto fence = this->m_window.get_fence(this->m_curr_frame_index);

		vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::CommandBuffer vk_cmd_buffer = this->m_frames[this->m_curr_frame_index].get().handle();

		vk::SubmitInfo submit_info;
		submit_info.setWaitSemaphoreCount(1);
		submit_info.setSignalSemaphoreCount(1);
		submit_info.setCommandBufferCount(1);
		submit_info.setPWaitSemaphores(&wait_semaphore);
		submit_info.setPSignalSemaphores(&sign_semaphore);
		submit_info.setPCommandBuffers(&vk_cmd_buffer);
		submit_info.setPWaitDstStageMask(&wait_stage);

		this->m_root.queue().handle().submit(submit_info, fence);
		this->m_window.present(this->m_curr_frame_index, this->m_curr_image_index, this->m_root.queue().handle());
		this->m_curr_frame_index = (this->m_curr_frame_index + 1) % this->m_window.in_flight_frame_count();
	}

	avk::command_buffer& current_command_buffer()
	{
		return this->m_frames[this->m_curr_frame_index];
	}

	uint32_t current_image_index() const { return m_curr_image_index; }

private:
	Root& m_root;
	WindowManager& m_window;
	std::array<avk::command_buffer, N> m_frames;
	int m_curr_frame_index = 0;
	uint32_t m_curr_image_index = 0;
};