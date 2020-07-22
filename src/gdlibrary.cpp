#include "Plugin.hpp"
#include "Property.hpp"
#include "Schema.hpp"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	using namespace godot::structural_editor;
	godot::register_class<Plugin>();
	godot::register_class<InspectorPlugin>();
	godot::register_class<ResourceSchema>();
	godot::register_class<DictionaryPropertyEditor>();
	godot::register_class<DictionaryProperty>();
	godot::register_class<ArrayPropertyEditor>();
	godot::register_class<ArrayProperty>();
	godot::register_class<ResourceSchemaProperty>();
}
