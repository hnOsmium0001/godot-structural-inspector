#include "Property.hpp"

#include <CheckBox.hpp>
#include <InputEventMouseButton.hpp>
#include <LineEdit.hpp>
#include <MenuButton.hpp>
#include <OptionButton.hpp>
#include <PopupMenu.hpp>
#include <SpinBox.hpp>

using namespace godot;
using namespace godot::structural_editor;

Control* StructSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto editor = new StructEditor(this, target);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

Control* ArraySchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto editor = new ArrayEditor(this, target);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

Control* StringSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	// TODO filtering support

	auto edit = new LineEdit();
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("text_changed", target, "update_value", path);
	if (name) {
		auto label = new Label();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = new HBoxContainer();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

Control* EnumSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto edit = new OptionButton();
	for (auto& [name, id] : elements) {
		edit->get_popup()->add_item(name, id);
	}
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("item_selected", target, "update_value", path);
	if (name) {
		auto label = new Label();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = new HBoxContainer();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

Control* IntSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto edit = new SpinBox();
	edit->set_min(static_cast<real_t>(min_value));
	edit->set_max(static_cast<real_t>(max_value));
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = new Label();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = new HBoxContainer();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

Control* RealSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto edit = new SpinBox();
	edit->set_min(min_value);
	edit->set_max(max_value);
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = new Label();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = new HBoxContainer();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

Control* BoolSchema::create_edit(const String* name, const Object* target, const Array& path) const {
	auto edit = new CheckBox();
	edit->connect("toggled", target, "update_value", path);
	if (name) {
		auto label = new Label();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = new HBoxContainer();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void StructEditor::_register_methods() {
	register_method("has_proerpty_name", &StructEditor::has_property_name);
	register_method("get_property_name", &StructEditor::get_property_name);
	register_method("set_property_name", &StructEditor::set_property_name);
}

void StructEditor::_init() {
	toolbar = new HBoxContainer();
	add_child(toolbar);

	title = new Label();
	toolbar->add_child(title);

	fields = new VBoxContainer();
	for (auto& [name, schema] : schema->fields) {
		auto path = this->path;
		path.append(name);
		auto field = schema->create_edit(&name, notif_target, path);
		fields->add_child(field);
	}
	add_child(fields);
}

bool StructEditor::has_property_name() const {
	return title->is_visible();
}

String StructEditor::get_property_name() const {
	return title->get_text();
}

void StructEditor::set_property_name(const String& name) {
	title->set_visible(true);
	title->set_text(name);
}

StructEditor::StructEditor() {
}

StructEditor::StructEditor(const StructSchema* schema, const Object* notif_target) {
	this->schema = schema;
	this->notif_target = notif_target;
}

StructEditor::~StructEditor() {
}

void ArrayEditor::_element_gui_input(Ref<InputEvent> event, Control* element) {
	Ref<InputEventMouseButton> mb = event;
	if (mb.is_valid() && element->get_global_rect().has_point(get_global_mouse_position())) {
		selected_elm = element->get_index();
		remove->set_disabled(false);
		return;
	}
}

void ArrayEditor::_add_element() {
	auto path = this->path;
	path.append(elements->get_child_count());
	auto element = schema->create_edit(nullptr, notif_target, path);
	elements->add_child(element);

	Array binds{};
	binds.append(element);
	element->connect("gui_input", this, "_element_gui_input", binds);
}

void ArrayEditor::_remove_element() {
	if (selected_elm != -1) {
		elements->get_child(selected_elm)->queue_free();
		selected_elm = -1;
		remove->set_disabled(true);
	}
}

void ArrayEditor::_register_methods() {
	register_method("_element_gui_input", &ArrayEditor::_element_gui_input);
	register_method("_add_element", &ArrayEditor::_add_element);
	register_method("_remove_element", &ArrayEditor::_remove_element);
	register_method("has_proerpty_name", &ArrayEditor::has_property_name);
	register_method("get_property_name", &ArrayEditor::get_property_name);
	register_method("set_property_name", &ArrayEditor::set_property_name);
}

void ArrayEditor::_init() {
	toolbar = new HBoxContainer();
	add_child(toolbar);

	title = new Label();
	toolbar->add_child(title);
	add = new Button();
	add->connect("pressed", this, "_add_element");
	toolbar->add_child(add);
	remove = new Button();
	remove->connect("pressed", this, "_remove_element");
	toolbar->add_child(remove);

	elements = new VBoxContainer();
	add_child(elements);
}

bool ArrayEditor::has_property_name() const {
	return title->is_visible();
}

String ArrayEditor::get_property_name() const {
	return title->get_text();
}

void ArrayEditor::set_property_name(const String& name) {
	title->set_visible(true);
	title->set_text(name);
}

ArrayEditor::ArrayEditor() {
}

ArrayEditor::ArrayEditor(const ArraySchema* schema, const Object* notif_target) {
	this->schema = schema;
	this->notif_target = notif_target;
}

ArrayEditor::~ArrayEditor() {
}

namespace {
Variant find_object(const Variant& root, const Array& path, int distance) {
	// Either `Array` or `Dictionary`
	auto current_object = root;
	for (int i = 0; i < path.size() - distance; ++i) {
		auto& prop_name = path[i];
		switch (prop_name.get_type()) {
			case Variant::STRING: {
				String name = prop_name;
				Dictionary dict = current_object;
				current_object = dict[name];
			} break;
			case Variant::INT: {
				int idx = prop_name;
				Array array = current_object;
				current_object = array[idx];
			} break;
			default: {
				// Invalid `prop_name` type
				return Variant{ static_cast<Object*>(nullptr) };
			}
		}
	}
	return current_object;
}

void update_value(const Variant& root, const Array& path, const Variant& value) {
	auto object = find_object(root, path, 1);
	auto last_prop_name = path[path.size() - 1];
	switch (last_prop_name.get_type()) {
		case Variant::STRING: {
			String name = last_prop_name;
			Dictionary dict = object;
			dict[name] = value;
		} break;
		case Variant::INT: {
			int idx = last_prop_name;
			Array array = object;
			array[idx] = value;
		} break;
		default: {
			// Invalid `prop_name` type
			return;
		} break;
	}
}
} // namespace

void StructProperty::_register_methods() {
	register_method("update_property", &StructProperty::update_property);
}

void StructProperty::_init() {
}

void StructProperty::update_value(Variant value, const Array& path) {
	::update_value(get_edited_object()->get(get_edited_property()), path, value);
}

void StructProperty::add_array_element(int pos, Variant elm, const Array& path) {
	Array array = ::find_object(get_edited_object()->get(get_edited_property()), path, 0);
	array.insert(pos, elm);
}

void StructProperty::remove_array_element(int idx, const Array& path) {
	Array array = ::find_object(get_edited_object()->get(get_edited_property()), path, 0);
	array.remove(idx);
}

void StructProperty::update_property() {
}

StructProperty::StructProperty() {
}

StructProperty::~StructProperty() {
}

void ArrayProperty::_register_methods() {
	register_method("update_property", &ArrayProperty::update_property);
}

void ArrayProperty::_init() {
}

void ArrayProperty::update_value(Variant value, const Array& path) {
	::update_value(get_edited_object()->get(get_edited_property()), path, value);
}

void ArrayProperty::add_array_element(int pos, Variant elm, const Array& path) {
	Array array = ::find_object(get_edited_object()->get(get_edited_property()), path, 0);
	array.insert(pos, elm);
}

void ArrayProperty::remove_array_element(int idx, const Array& path) {
	Array array = ::find_object(get_edited_object()->get(get_edited_property()), path, 0);
	array.remove(idx);
}

void ArrayProperty::update_property() {
}

ArrayProperty::ArrayProperty() {
}

ArrayProperty::~ArrayProperty() {
}

void ResourceSchemaProperty::_toggle_editor_visibility() {
	if (properties->is_visible()) {
		remove_child(properties);
		set_bottom_editor(nullptr);
		properties->set_visible(false);
	} else {
		add_child(properties);
		set_bottom_editor(properties);
		properties->set_visible(true);
	}
}

void ResourceSchemaProperty::_register_methods() {
	register_method("update_property", &ResourceSchemaProperty::update_property);
}

void ResourceSchemaProperty::_init() {
	btn = new Button();
	btn->connect("pressed", this, "_toggle_editor_visibility");
	add_child(btn);

	// TODO
	//properties = new ArrayPropertyEditor();
}

void ResourceSchemaProperty::update_property() {
}

ResourceSchemaProperty::ResourceSchemaProperty() {
}

ResourceSchemaProperty::~ResourceSchemaProperty() {
}
