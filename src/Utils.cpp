#include "Utils.hpp"

#include <InputEvent.hpp>
#include <InputEventMouseMotion.hpp>
#include <Texture.hpp>

using namespace godot;
using namespace godot::structural_inspector;

void EditorIconButton::_notification(int what) {
	switch (what) {
		case NOTIFICATION_READY: {
			call_deferred("_apply_icon");
		} break;
		default: {
		} break;
	}
}

void EditorIconButton::_apply_icon() {
	set_button_icon(get_icon(icon_name, "EditorIcons"));
}

void EditorIconButton::_register_methods() {
	register_method("_notification", &EditorIconButton::_notification);
	register_method("_apply_icon", &EditorIconButton::_apply_icon);
}

void EditorIconButton::_init() {
	set_flat(true);
}

void EditorIconButton::_custom_init(const String& icon_name) {
	this->icon_name = icon_name;
}

EditorIconButton::EditorIconButton() {
}

EditorIconButton::~EditorIconButton() {
}

void BorderedContainer::_notification(int what) {
	switch (what) {
		case NOTIFICATION_RESIZED: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
			// First is regular border color, second is hovered border color
			static const Color colors[] = { Color::hex(0x262C3BFF), Color::hex(0x333B4FFF) };
			draw_rect(Rect2{ 4, 4, get_size().x - 4 * 2, get_size().y - 4 * 2 }, colors[mouse_inside], false, 2.0F);
		} break;
	}
}

void BorderedContainer::_input(Ref<InputEvent> event) {
	if (auto mm = Object::cast_to<InputEventMouseMotion>(event.ptr())) {
		bool current = get_global_rect().has_point(get_global_mouse_position());
		if (mouse_inside != current) {
			mouse_inside = current;
			update();
		}
		return;
	}
}

void BorderedContainer::_register_methods() {
	register_method("_notification", &BorderedContainer::_notification);
	register_method("_input", &BorderedContainer::_input);
}

void BorderedContainer::_init() {
	add_constant_override("margin_left", 8);
	add_constant_override("margin_top", 8);
	add_constant_override("margin_right", 8);
	add_constant_override("margin_bottom", 8);
}

BorderedContainer::BorderedContainer() {
}

BorderedContainer::~BorderedContainer() {
}

// void ListContainer::_reorder() {
// 	// using ComputeNext = Point2(*)(Point2, Size2);
// 	// static const auto compute_next_pos_horizontal = [](Point2 prev, Size2 size) {
// 	// 	return Point2{ prev.x + size.x, prev.y };
// 	// };
// 	// static const auto compute_next_pos_vertical = [](Point2 prev, Size2 size) {
// 	// 	return Point2{ prev.x, prev.y + size.y };
// 	// };

// 	auto size = get_size();

// 	// ComputeNext compute_next_pos = (dir == HORIZONTAL) ? compute_next_pos_horizontal : compute_next_pos_vertical;

// 	Point2 next_pos{ 0, 0 };
// 	for (int i = 0; i < get_child_count(); ++i) {
// 		auto child = Object::cast_to<Control>(get_child(i));
// 		if (!child) continue;

// 		auto csize = child->get_combined_minimum_size();
// 		child->set_position(next_pos);

// 		switch (dir) {
// 			case HORIZONTAL: {
// 				next_pos = { next_pos.x + csize.x + separation, next_pos.y };
// 			} break;
// 			case VERTICAL: {
// 				next_pos = { next_pos.x, next_pos.y + csize.y + separation };
// 			} break;
// 		}
// 	}

// 	set_size(next_pos);
// }

// void ListContainer::_notification(int what) {
// 	switch (what) {
// 		case NOTIFICATION_SORT_CHILDREN: {
// 			_reorder();
// 		} break;
// 		case NOTIFICATION_DRAW: {
// 			draw_rect(Rect2{ 0, 0, get_size().x, get_size().y }, Color::hex(0xFFFF00FF), false, 2.0F);
// 		} break;
// 		case NOTIFICATION_RESIZED: {
// 			update();
// 		} break;
// 	}
// }

// void ListContainer::_register_methods() {
// 	register_method("_notification", &ListContainer::_notification);
// 	// register_method("get_minimum_size", &ListContainer::get_minimum_size);
// }

// void ListContainer::_init() {
// }

// ListContainer::Direction ListContainer::get_direction() const {
// 	return dir;
// }

// void ListContainer::set_direction(Direction dir) {
// 	this->dir = dir;
// 	queue_sort();
// }

// int ListContainer::get_separation() const {
// 	return separation;
// }

// void ListContainer::set_separation(int separation) {
// 	this->separation = separation;
// 	queue_sort();
// }

// bool ListContainer::is_stretching() const {
// 	return stretch;
// }

// void ListContainer::set_stretching(bool stretch) {
// 	this->stretch = stretch;
// 	queue_sort();
// }

// // Size2 ListContainer::get_minimum_size() {
// // 	Size2 result;
// // 	for (int i = 0; i < get_child_count(); ++i) {
// // 		auto child = Object::cast_to<Control>(get_child(i));
// // 		if (!child) continue;

// // 		auto csize = child->get_size();
// // 		switch (dir) {
// // 			case HORIZONTAL: {
// // 				result.x += csize.x + separation;
// // 			} break;
// // 			case VERTICAL: {
// // 				result.y += csize.y + separation;
// // 			} break;
// // 		}
// // 	}
// // 	return result;
// // }

// ListContainer::ListContainer() {
// }

// ListContainer::~ListContainer() {
// }

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
