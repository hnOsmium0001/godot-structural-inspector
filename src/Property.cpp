#include "Property.hpp"

#include <MenuButton.hpp>

using namespace godot;
using namespace godot::structural_editor;

void DictionaryPropertyEditor::_register_methods() {
}

void DictionaryPropertyEditor::_init() {
	toolbar = new HBoxContainer();
	{
		auto add = new MenuButton();
		add->set_button_icon(get_icon("Add", "EditorIcon"));
		for (auto& [name, field] : schema->fields) {

		}
		toolbar->add_child(add);
	}
	add_child(toolbar);
}

void DictionaryProperty::_register_methods() {
	register_method("update_property", &DictionaryProperty::update_property);
}

void DictionaryProperty::_init() {
}

DictionaryPropertyEditor::DictionaryPropertyEditor(const DictionaryField* schema) {
	this->schema = schema;
}

DictionaryPropertyEditor::~DictionaryPropertyEditor() {
}

void DictionaryProperty::update_property() {
}

DictionaryProperty::DictionaryProperty() {
}

DictionaryProperty::~DictionaryProperty() {
}

void ArrayPropertyEditor::_register_methods() {
}

void ArrayPropertyEditor::_init() {
}

ArrayPropertyEditor::ArrayPropertyEditor(const ArrayField* schema) {
	this->schema = schema;
}

ArrayPropertyEditor::~ArrayPropertyEditor() {
}

void ArrayProperty::_register_methods() {
	register_method("update_property", &ArrayProperty::update_property);
}

void ArrayProperty::_init() {
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
