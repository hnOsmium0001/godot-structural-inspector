#include "Utils.hpp"

#include <InputEvent.hpp>
#include <InputEventMouseMotion.hpp>
#include <algorithm>

using namespace godot;
using namespace godot::structural_inspector;

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

void ListContainer::_reorder() {
	auto size = get_size();

	// TODO handle size flags
	if (stretch) {
		// FIXME these should be `int` in 4.0
		float combined_length_a = 0;
		int child_count = 0;
		for (int i = 0; i < get_child_count(); ++i) {
			auto child = Object::cast_to<Control>(get_child(i));
			if (!child) continue;
			if (!child->is_visible()) continue;

			auto csize = child->get_combined_minimum_size();
			switch (dir) {
				case HORIZONTAL: {
					combined_length_a += csize.x;
				} break;
				case VERTICAL: {
					combined_length_a += csize.y;
				} break;
			}

			++child_count;
		}

		float combined_length = dir == HORIZONTAL ? size.x : size.y;
		float unit_length = combined_length / child_count;
		Point2 next_pos{ 0, 0 };
		for (int i = 0; i < get_child_count(); ++i) {
			auto child = Object::cast_to<Control>(get_child(i));
			if (!child) continue;
			if (!child->is_visible()) continue;

			auto csize = child->get_combined_minimum_size();
			switch (dir) {
				case HORIZONTAL: {
					child->set_position(next_pos);
					// TODO take child minimum size into account
					child->set_size({ unit_length, size.y });
					next_pos = { next_pos.x + unit_length + separation, next_pos.y };
				} break;
				case VERTICAL: {
					child->set_position(next_pos);
					child->set_size({ size.x, unit_length });
					next_pos = { next_pos.x, next_pos.y + unit_length + separation };
				} break;
			}
		}
	} else {
		Point2 next_pos{ 0, 0 };
		for (int i = 0; i < get_child_count(); ++i) {
			auto child = Object::cast_to<Control>(get_child(i));
			if (!child) continue;
			if (!child->is_visible()) continue;

			auto csize = child->get_combined_minimum_size();

			switch (dir) {
				case HORIZONTAL: {
					child->set_position(next_pos);
					child->set_size({ csize.x, size.y });
					next_pos = { next_pos.x + csize.x + separation, next_pos.y };
				} break;
				case VERTICAL: {
					child->set_position(next_pos);
					child->set_size({ size.x, csize.y });
					next_pos = { next_pos.x, next_pos.y + csize.y + separation };
				} break;
			}
		}
	}
}

void ListContainer::_notification(int what) {
	switch (what) {
		case NOTIFICATION_SORT_CHILDREN: {
			_reorder();
		} break;
	}
}

Size2 ListContainer::_get_minimum_size() {
	Size2 min_size;
	for (int i = 0; i < get_child_count(); ++i) {
		auto child = Object::cast_to<Control>(get_child(i));
		if (!child) continue;
		if (!child->is_visible()) continue;

		auto csize = child->get_combined_minimum_size();
		switch (dir) {
			case HORIZONTAL: {
				min_size.x += csize.x + separation;
				min_size.y = std::max(min_size.y, csize.y);
			} break;
			case VERTICAL: {
				min_size.x = std::max(min_size.x, csize.x);
				min_size.y += csize.y + separation;
			} break;
		}
	}
	min_size.x -= separation;
	min_size.y -= separation;
	return min_size;
}

void ListContainer::_register_methods() {
	// register_method("_notification", &ListContainer::_notification);
	// register_method("_get_minimum_size", &ListContainer::_get_minimum_size);
}

void ListContainer::_init() {
}

ListContainer::Direction ListContainer::get_direction() const {
	return dir;
}

void ListContainer::set_direction(Direction dir) {
	this->dir = dir;
	queue_sort();
}

int ListContainer::get_separation() const {
	return separation;
}

void ListContainer::set_separation(int separation) {
	this->separation = separation;
	queue_sort();
}

bool ListContainer::is_stretching() const {
	return stretch;
}

void ListContainer::set_stretching(bool stretch) {
	this->stretch = stretch;
	queue_sort();
}

ListContainer::ListContainer() {
}

ListContainer::~ListContainer() {
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
