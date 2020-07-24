#include "Plugin.hpp"
#include "Property.hpp"

#include <ResourceLoader.hpp>
#include <Script.hpp>

using namespace godot;
using namespace godot::structural_editor;

void InspectorPlugin::_register_methods() {
	register_method("_notification", &InspectorPlugin::_notification);
	register_method("can_handle", &InspectorPlugin::can_handle);
	register_method("parse_property", &InspectorPlugin::parse_property);
}

void InspectorPlugin::_init() {
}

void InspectorPlugin::_notification(int what) {
}

bool InspectorPlugin::can_handle(const Object* object) {
	auto script = Ref{ static_cast<Script*>(object->get_script()) };
	return (script.is_valid() && script->get_script_constant_map().has("resource_schema_path")) ||
		   Object::cast_to<ResourceSchema>(object) != nullptr;
}

bool InspectorPlugin::parse_property(const Object* object, const int64_t type, const String path, const int64_t hint, const String hint_text, const int64_t usage) {
	if (Object::cast_to<ResourceSchema>(object) != nullptr) {
		if (path == "properties") {
			add_property_editor(path, ResourceSchemaInspectorProperty::_new());
			return true;
		} else {
			return false;
		}
	}

	auto script = Ref{ static_cast<Script*>(object->get_script()) };
	// Since the object passed the test in `can_handle`, and it's not a ResourceSchema, it must have a valid script attached
	String schema_path = script->get_script_constant_map()["resource_schema_path"];
	Ref<ResourceSchema> schema = ResourceLoader::get_singleton()->load(schema_path);
	if (schema.is_valid()) {
		auto info = schema->get_info();
		auto iter = info.find(path);
		if (iter != info.end()) {
			add_property_editor(path, iter->second->create_property());
			return true;
		}
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
			inspector_plugin = Ref{ InspectorPlugin::_new() };
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

size_t std::hash<String>::operator()(const String& str) const noexcept {
	return str.hash();
}
