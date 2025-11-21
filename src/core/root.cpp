#include <native/core/root.h>

const avk::renderpass& Root::default_renderpass()
{
	static const avk::renderpass default_pass = create_renderpass({
		avk::attachment::declare(
			OBSIDIAN_COLOR_FORMAT,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::color(0),
			avk::on_store::store
		),
		avk::attachment::declare(
			OBSIDIAN_DEPTH_FORMAT,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::depth_stencil,
			avk::on_store::dont_care
		)
	});

	return default_pass;
}
