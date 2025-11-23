#include <native/core/obsidian/root.h>

const avk::renderpass& Root::default_renderpass()
{
	static const avk::renderpass default_pass = create_renderpass({
		avk::attachment::declare(
			OBSIDIAN_COLOR_FORMAT,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::color(0),
			avk::on_store::store.in_layout(avk::layout::present_src)
		).set_clear_color({ 1.0f, 0.0f, 1.0f, 1.0f }),
		avk::attachment::declare(
			OBSIDIAN_DEPTH_FORMAT,
			avk::on_load::clear.from_previous_layout(avk::layout::undefined),
			avk::usage::depth_stencil,
			avk::on_store::dont_care
		)
	});

	return default_pass;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Root::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data)
{
	auto tl_severity = severity_str_mapper(severity);
	auto tl_type = type_str_mapper(type);

	std::vector<std::string> objects;
	for (uint32_t i = 0; i < p_callback_data->objectCount; ++i)
	{
		objects.push_back(std::to_string(p_callback_data->pObjects[i].objectHandle));
	}

	std::string tl_objects = objects.empty() ? "" : std::accumulate(std::begin(objects), std::end(objects), std::string(),
		[](std::string ss, std::string s) {
			return ss.empty() ? s : ss + "," + s;
		});

	std::string output = "Debug callback: " + std::string(p_callback_data->pMessage) + ", severity: " + tl_severity + ", type: " + tl_type + ".";
	if (!objects.empty())
	{
		output += " Objects: " + tl_objects;
	}

	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		LOG_S(INFO) << output;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		LOG_S(WARNING) << output;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		LOG_S(ERROR) << output;
		break;
	}

	return VK_FALSE;
}

void Root::create_debug_callback()
{
	VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = //VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = &debug_callback,
		.pUserData = NULL
	};

	PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger = VK_NULL_HANDLE;
	vk_create_debug_utils_messenger = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->m_instance.get(), "vkCreateDebugUtilsMessengerEXT");
	if (!vk_create_debug_utils_messenger)
	{
		LOG_S(ERROR) << "Unable to find address of vk_create_debug_utils_messenger.";
		exit(-1);
	}

	VkResult result = vk_create_debug_utils_messenger(this->m_instance.get(), &messenger_create_info, NULL, &m_debug_messenger);
}

const char* Root::severity_str_mapper(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		return "Verbose";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		return "Info";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		return "Warning";
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		return "Error";
	default:
		LOG_S(ERROR) << "Invalid severity code in debug callback.";
	}

	return "";
}

const char* Root::type_str_mapper(VkDebugUtilsMessageTypeFlagsEXT type)
{
	switch (type)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		return "General";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		return "Validation";
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		return "Performance";
	default:
		LOG_S(ERROR) << "Invalid type code in debug callback.";
	}

	return "";
}
