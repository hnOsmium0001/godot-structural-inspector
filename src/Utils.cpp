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

size_t std::hash<String>::operator()(const String& str) const noexcept {
	return str.hash();
}
