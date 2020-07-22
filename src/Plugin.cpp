#include <ResourceLoader.hpp>

#include "Plugin.hpp"
#include "Property.hpp"
#include "Schema.hpp"

using namespace godot;
using namespace godot::structural_editor;

void InspectorPlugin::_register_methods() {
	register_method("can_handle", &InspectorPlugin::can_handle);
	register_method("parse_property", &InspectorPlugin::parse_property);
}

void InspectorPlugin::_init() {
}

void InspectorPlugin::_notification(int what) {
}

bool InspectorPlugin::can_handle(const Object* object) {
	return (object->is_class("Resource") && object->has_meta("resource_schema_path")) || object->is_class("ResourceSchema");
}

bool InspectorPlugin::parse_property(const Object* object, const int64_t type, const String path, const int64_t hint, const String hint_text, const int64_t usage) {
	if (object->is_class("ResourceSchema")) {
		add_property_editor(path, new ResourceSchemaProperty());
		return true;
	}

	String schema_path = object->get_meta("resource_schema_path");
	Ref<ResourceSchema> schema = ResourceLoader::get_singleton()->load(schema_path);
	if (schema.is_valid()) {
		// TODO
		return true;
	}

	return false;
}

InspectorPlugin::InspectorPlugin() {
}

InspectorPlugin::~InspectorPlugin() {
}

void Plugin::_register_methods() {
	register_method("_notification", &Plugin::_notification);
}

void Plugin::_init() {
}

void Plugin::_notification(int what) {
	switch (what) {
		case NOTIFICATION_ENTER_TREE: {
			inspector_plugin = Ref{ new InspectorPlugin() };
			add_inspector_plugin(inspector_plugin);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_inspector_plugin(inspector_plugin);
		} break;
	}
}

Plugin::Plugin() {
}

Plugin::~Plugin() {
}

size_t std::hash<String>::operator()(const String& str) const {
	return str.hash();
}
