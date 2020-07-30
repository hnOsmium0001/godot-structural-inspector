#include "Schema.hpp"
#include "Property.hpp"

#include <PopupMenu.hpp>
#include <CheckBox.hpp>
#include <Control.hpp>
#include <HBoxContainer.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <OptionButton.hpp>
#include <SpinBox.hpp>

using namespace godot;
using namespace godot::structural_inspector;

std::unique_ptr<Schema> Schema::clone_uptr() const {
	return std::unique_ptr<Schema>(clone());
}

Variant StructSchema::create_value() const {
	return Dictionary{};
}

Control* StructSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto editor = StructEditor::_new();
	editor->_custom_init(this, target, path);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

void StructSchema::update_edit(Control* edit, const Variant& data) const {
	auto editor = static_cast<StructEditor*>(edit);
	Dictionary data_source = data;
	for (int i = 0; i < fields.size(); ++i) {
		auto& field = fields[i];
		field.def->update_edit(
				static_cast<Control*>(editor->fields->get_child(i)),
				data_source[field.name]);
	}
}

EditorProperty* StructSchema::create_property() const {
	auto prop = CommonInspectorProperty::_new();
	auto editor = StructEditor::_new();
	editor->_custom_init(this, prop, Array::make());
	prop->_custom_init(this, editor);
	return prop;
}

StructSchema* StructSchema::clone() const {
	auto that = new StructSchema();
	for (auto& [name, field] : fields) {
		that->fields.push_back({ name, field->clone_uptr() });
	}
	return that;
}

Variant ArraySchema::create_value() const {
	return Array{};
}

Control* ArraySchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto editor = ArrayEditor::_new();
	editor->_custom_init(this, target, path);
	if (name) {
		editor->set_property_name(*name);
	}
	return editor;
}

void ArraySchema::update_edit(Control* edit, const Variant& data) const {
	auto editor = static_cast<ArrayEditor*>(data);
	Array data_source = data;
	for (int i = 0; i < data_source.size(); ++i) {
		auto field = static_cast<Control*>(editor->elements->get_child(i));
		element_type->update_edit(field, data_source[i]);
	}
}

EditorProperty* ArraySchema::create_property() const {
	auto prop = CommonInspectorProperty::_new();
	auto editor = ArrayEditor::_new();
	editor->_custom_init(this, prop, Array::make());
	prop->_custom_init(this, editor);
	return prop;
}

ArraySchema* ArraySchema::clone() const {
	auto that = new ArraySchema();
	that->element_type = element_type->clone_uptr();
	that->min_elements = min_elements;
	that->max_elements = max_elements;
	return that;
}

Variant StringSchema::create_value() const {
	return "";
}

Control* StringSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	// TODO filtering support

	auto edit = LineEdit::_new();
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("text_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void StringSchema::update_edit(Control* edit, const Variant& data) const {
	String text = data;
	if (auto line_edit = Object::cast_to<LineEdit>(edit)) {
		line_edit->set_text(data);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto line_edit = static_cast<LineEdit*>(editor->get_child(1));
		line_edit->set_text(text);
		return;
	}
}

EditorProperty* StringSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

StringSchema* StringSchema::clone() const {
	auto that = new StringSchema();
	if (pattern.is_valid()) {
		that->pattern = Ref{ RegEx::_new() };
		// Must success, since our pattern is valid
		that->pattern->compile(pattern->get_pattern());
	}
	return that;
}

Variant EnumSchema::create_value() const {
	return 0;
}

Control* EnumSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = OptionButton::_new();
	for (auto& [name, id] : elements) {
		edit->get_popup()->add_item(name, id);
	}
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("item_selected", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void EnumSchema::update_edit(Control* edit, const Variant& data) const {
	int id = data;
	if (auto option_btn = Object::cast_to<OptionButton>(edit)) {
		option_btn->select(option_btn->get_item_index(id));
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto option_btn = static_cast<OptionButton*>(editor->get_child(1));
		option_btn->select(option_btn->get_item_index(id));
		return;
	}
}

EditorProperty* EnumSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

EnumSchema* EnumSchema::clone() const {
	auto that = new EnumSchema();
	for (auto& element : elements) {
		that->elements.push_back(element);
	}
	return that;
}

Variant IntSchema::create_value() const {
	return 0;
}

Control* IntSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = SpinBox::_new();
	edit->set_min(static_cast<real_t>(min_value));
	edit->set_max(static_cast<real_t>(max_value));
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void IntSchema::update_edit(Control* edit, const Variant& data) const {
	int value = data;
	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
		spin_box->set_value(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
		spin_box->set_value(value);
		return;
	}
}

EditorProperty* IntSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

IntSchema* IntSchema::clone() const {
	auto that = new IntSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

Variant FloatSchema::create_value() const {
	return 0.0;
}

Control* FloatSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = SpinBox::_new();
	edit->set_min(min_value);
	edit->set_max(max_value);
	edit->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);
	edit->connect("value_changed", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void FloatSchema::update_edit(Control* edit, const Variant& data) const {
	real_t value = data;
	if (auto spin_box = Object::cast_to<SpinBox>(edit)) {
		spin_box->set_value(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto spin_box = static_cast<SpinBox*>(editor->get_child(1));
		spin_box->set_value(value);
		return;
	}
}

EditorProperty* FloatSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

FloatSchema* FloatSchema::clone() const {
	auto that = new FloatSchema();
	that->min_value = min_value;
	that->max_value = max_value;
	return that;
}

Variant BoolSchema::create_value() const {
	return false;
}

Control* BoolSchema::create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const {
	auto edit = CheckBox::_new();
	edit->connect("toggled", target, "update_value", path);
	if (name) {
		auto label = Label::_new();
		label->set_text(*name);
		label->set_h_size_flags(Control::SIZE_FILL | Control::SIZE_EXPAND);

		auto box = HBoxContainer::_new();
		box->add_child(label);
		box->add_child(edit);
		return box;
	} else {
		return edit;
	}
}

void BoolSchema::update_edit(Control* edit, const Variant& data) const {
	bool value = data;
	if (auto checkbox = Object::cast_to<CheckBox>(edit)) {
		checkbox->set_pressed(value);
		return;
	}
	if (auto editor = Object::cast_to<HBoxContainer>(edit)) {
		auto checkbox = static_cast<CheckBox*>(editor->get_child(1));
		checkbox->set_pressed(value);
		return;
	}
}

EditorProperty* BoolSchema::create_property() const {
	return CommonInspectorProperty::_new();
}

BoolSchema* BoolSchema::clone() const {
	return new BoolSchema();
}
