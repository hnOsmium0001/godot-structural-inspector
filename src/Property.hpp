#pragma once

#include "Schema.hpp"
#include <Button.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <HBoxContainer.hpp>
#include <VBoxContainer.hpp>

namespace godot::structural_editor {

class DictionaryPropertyEditor : public VBoxContainer {
	GODOT_CLASS(DictionaryPropertyEditor, VBoxContainer)
private:
	const DictionaryField* schema;
	HBoxContainer* toolbar;
	VBoxContainer* elements;

public:
	static void _register_methods();
	void _init();

	DictionaryPropertyEditor(const DictionaryField* schema);
	~DictionaryPropertyEditor();
};

class DictionaryProperty : public EditorProperty {
	GODOT_CLASS(DictionaryProperty, EditorProperty)
private:
	Button* btn;
	DictionaryPropertyEditor* editor;

public:
	static void _register_methods();
	void _init();

	void update_property();

	DictionaryProperty();
	~DictionaryProperty();
};

class ArrayPropertyEditor : public VBoxContainer {
	GODOT_CLASS(ArrayPropertyEditor, VBoxContainer)
private:
	const ArrayField* schema;
	HBoxContainer* toolbar;
	VBoxContainer* elements;

public:
	static void _register_methods();
	void _init();

	ArrayPropertyEditor(const ArrayField* schema);
	~ArrayPropertyEditor();
};

class ArrayProperty : public EditorProperty {
	GODOT_CLASS(ArrayProperty, EditorProperty)
private:
	Button* btn;
	ArrayPropertyEditor* editor;

public:
	static void _register_methods();
	void _init();

	void update_property();

	ArrayProperty();
	~ArrayProperty();
};

class ResourceSchemaProperty : public EditorProperty {
	GODOT_CLASS(ResourceSchemaProperty, EditorProperty)
private:
	Button* btn;
	ArrayPropertyEditor* properties;

	void _toggle_editor_visibility();
	void _add_property();
	void _update_property(int idx);

public:
	static void _register_methods();
	void _init();

	void update_property();

	ResourceSchemaProperty();
	~ResourceSchemaProperty();
};

} // namespace godot::structural_editor
