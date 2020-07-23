#include "Schema.hpp"

using namespace godot;
using namespace godot::structural_editor;

void ResourceSchema::_register_methods() {
	register_property("properties", &ResourceSchema::properties, Array{}, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_EDITOR);
}

void ResourceSchema::_init() {
}

ResourceSchema::ResourceSchema() {
}

ResourceSchema::~ResourceSchema() {
}
