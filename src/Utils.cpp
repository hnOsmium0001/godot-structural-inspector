#include "Utils.hpp"

#include <Texture.hpp>

using namespace godot;
using namespace godot::structural_inspector;

void NXButton::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			call_deferred("_apply_icon");
		} break;
		default: {
		} break;
	}
}

void NXButton::_apply_icon() {
	set_button_icon(get_icon(icon_name, "EditorIcons"));
}

void NXButton::_register_methods() {
	register_method("_notification", &NXButton::_notification);
	register_method("_apply_icon", &NXButton::_apply_icon);
}

void NXButton::_init() {
	set_flat(true);
}

void NXButton::_custom_init(const String& icon_name) {
	this->icon_name = icon_name;
}

NXButton::NXButton() {
}

NXButton::~NXButton() {
}

String godot::structural_inspector::format_variant(const Variant& variant) {
	switch (variant.get_type()) {
		case Variant::DICTIONARY: return "dictionary";
		case Variant::ARRAY: return "array";
		case Variant::STRING: return "string";
		case Variant::INT: return "int";
		case Variant::REAL: return "float";
		case Variant::BOOL: return "bool";
		case Variant::OBJECT: return "object";
		case Variant::NIL: return "nil";
		case Variant::RECT2: return "rect2";
		case Variant::RECT3: return "aabb";
		case Variant::VECTOR2: return "vector2";
		case Variant::VECTOR3: return "vector3";
		case Variant::BASIS: return "basis";
		case Variant::TRANSFORM: return "transform";
		case Variant::TRANSFORM2D: return "transform2d";
		case Variant::QUAT: return "quaternion";
		case Variant::PLANE: return "plane";
		case Variant::COLOR: return "color";
		case Variant::NODE_PATH: return "node_path";
		default: return "unknown";
	}
}

size_t std::hash<String>::operator()(const String& str) const noexcept {
	return str.hash();
}
