#include "Plugin.hpp"
#include "ResourceEditor.hpp"
#include "SchemaEditor.hpp"

#include <GodotGlobal.hpp>
#include <Object.hpp>
#include <ResourceLoader.hpp>
#include <Script.hpp>

#include <JSON.hpp>

using namespace godot;
using namespace godot::structural_inspector;

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
	if (Object::cast_to<ResourceSchema>(object) != nullptr) {
		return true;
	}

	auto script = Ref{ static_cast<Script*>(object->get_script()) };
	if (script.is_valid()) {
		auto script_path = script->get_path();
		auto schema_path = script_path.substr(0, script_path.find_last(".")) + ".schema.tres";
		if (ResourceLoader::get_singleton()->exists(schema_path)) {
			auto resource = ResourceLoader::get_singleton()->load(schema_path);
			return Object::cast_to<ResourceSchema>(resource.ptr()) != nullptr;
		}

		// TODO allow custom schema path
		// if (script->get_script_constant_map().has("resource_schema_path")) {
		// 	String path = script->get_script_constant_map()["resource_schema_path"];
		// 	Ref<ResourceSchema> schema = ResourceLoader::get_singleton()->load(path);
		// 	return schema.is_valid();
		// }
	}

	return false;
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

	// Since the object passed the test in `can_handle`, and it's not a ResourceSchema, it must have a valid script attached
	auto script = Ref{ static_cast<Script*>(object->get_script()) };
	auto script_path = script->get_path();
	auto schema_path = script_path.substr(0, script_path.find_last(".")) + ".schema.tres";
	Ref<ResourceSchema> ref = ResourceLoader::get_singleton()->load(schema_path);
	if (ref.is_valid()) {
		if (auto res = Object::cast_to<ResourceSchema>(ref.ptr())) {
			if (auto schema = res->compute_info_for(path)) {
				auto prop = ResourceInspectorProperty::_new();
				prop->_custom_init(std::move(schema));

				add_property_editor(path, prop);
				return true;
			}
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
