#pragma once

#include <native/core/include.h>

struct ShardMetadata
{
	vk::Format format = vk::Format::eUndefined;
	avk::attachment_load_config load_op = avk::on_load::dont_care;
	avk::attachment_store_config store_op = avk::on_store::dont_care;
	avk::subpass_usages usage = avk::usage::unused;
	avk::layout::image_layout layout = avk::layout::undefined;
	std::array<float, 4> clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };

	static auto default_color_attachment() -> ShardMetadata;
	static auto default_depth_attachment() -> ShardMetadata;
};

struct Shard
{
	std::vector<ShardMetadata> attachments;
	ShardMetadata depth_attachment;
	std::vector<std::string> pipelines;
	bool is_swapchain_target = false;

	std::vector<avk::image> internal_images;
	std::vector<avk::image_view> internal_views;
	avk::image internal_depth_image;
	avk::image_view internal_depth_view;
};