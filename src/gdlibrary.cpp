
#include "Plugin.hpp"
#include "Property.hpp"

#include <Godot.hpp>

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options* o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options* o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void* handle) {
	godot::Godot::nativescript_init(handle);

	using namespace godot::structural_editor;
	godot::register_tool_class<Plugin>();
	godot::register_tool_class<InspectorPlugin>();
	godot::register_tool_class<ResourceSchema>();
	godot::register_tool_class<NXButton>();
	godot::register_tool_class<StructEditor>();
	godot::register_tool_class<ArrayEditor>();
	godot::register_tool_class<CommonInspectorProperty>();
	godot::register_tool_class<ResourceSchemaNode>();
	godot::register_tool_class<ResourceSchemaInspectorProperty>();
}
