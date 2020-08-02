#pragma once

#include "Schema.hpp"

#include <EditorInspectorPlugin.hpp>
#include <EditorPlugin.hpp>
#include <Godot.hpp>
#include <unordered_map>

namespace godot::structural_inspector {

class InspectorPlugin : public EditorInspectorPlugin {
	GODOT_CLASS(InspectorPlugin, EditorInspectorPlugin)
public:
	static void _register_methods();
	void _init();
	void _notification(int what);

	bool can_handle(const Object* object);
	bool parse_property(const Object* object, const int64_t type, const String path, const int64_t hint, const String hint_text, const int64_t usage);

	void invalidate_schema(const String& schema_path);

	InspectorPlugin();
	~InspectorPlugin();
};

class Plugin : public EditorPlugin {
	GODOT_CLASS(Plugin, EditorPlugin)
private:
	Ref<InspectorPlugin> inspector_plugin;

public:
	static void _register_methods();
	void _init();
	void _notification(int what);

	Plugin();
	~Plugin();
};

} // namespace godot::structural_inspector
