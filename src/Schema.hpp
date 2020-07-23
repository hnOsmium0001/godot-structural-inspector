#pragma once

#include <Godot.hpp>
#include <Resource.hpp>

namespace godot::structural_editor {

class ResourceSchema : public Resource {
	GODOT_CLASS(ResourceSchema, Resource)
private:
	Array properties;

public:
	static void _register_methods();
	void _init();

	ResourceSchema();
	~ResourceSchema();
};

} // namespace godot::structural_editor
