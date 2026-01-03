#include "native/graphics/shard.h"

auto ShardMetadata::default_color_attachment() -> ShardMetadata
{
	ShardMetadata data;
	data.format = OBN_DEFAULT_COLOR_FORMAT;
	data.load_op = avk::on_load::clear;
	data.store_op = avk::on_store::store;
	data.usage = avk::usage::color(0);
	data.layout = avk::layout::color_attachment_optimal;
	data.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	return data;
}

auto ShardMetadata::default_depth_attachment() -> ShardMetadata
{
	ShardMetadata data;
	data.format = OBN_DEFAULT_DEPTH_FORMAT;
	data.load_op = avk::on_load::clear;
	data.store_op = avk::on_store::dont_care;
	data.usage = avk::usage::depth_stencil;
	data.layout = avk::layout::depth_stencil_attachment_optimal;
	return data;
}