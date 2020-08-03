#pragma once

#include "Schema.hpp"
#include "Utils.hpp"

#include <Button.hpp>
#include <Control.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <HBoxContainer.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <MarginContainer.hpp>
#include <OptionButton.hpp>
#include <SpinBox.hpp>
#include <VBoxContainer.hpp>
#include <memory>
#include <unordered_map>

namespace godot::structural_inspector {

class ResourceEditor {
public:
	virtual void propagate_value_update(const Variant& value) = 0;
	virtual void update_from(const Variant& value) = 0;
};

class StructEditor : public VBoxContainer, public ResourceEditor {
	GODOT_CLASS(StructEditor, VBoxContainer)
private:
	ResourceInspectorProperty* root;
	ResourceEditor* parent;
	const StructSchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	VBoxContainer* fields;

	Variant key;

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const StructSchema* schema, const Variant& key);

	void propagate_value_update(const Variant& value) override;
	void update_from(const Variant& value) override;

	StructEditor();
	~StructEditor();
};

class ArrayEditor : public VBoxContainer, public ResourceEditor {
	GODOT_CLASS(ArrayEditor, VBoxContainer)

	friend class ArraySchema;

private:
	ResourceInspectorProperty* root;
	ResourceEditor* parent;
	const ArraySchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	NXButton* add;
	NXButton* remove;
	VBoxContainer* elements;
	int64_t selected_idx = -1;

	Variant key;

	void _element_gui_input(Ref<InputEvent> event, Control* element);
	void _add_element();
	void _remove_element();

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const ArraySchema* schema, const Variant& key);

	void propagate_value_update(const Variant& value) override;
	void update_from(const Variant& value) override;

	ArrayEditor();
	~ArrayEditor();
};

class ValueEditor : public HBoxContainer, public ResourceEditor {
	GODOT_CLASS(ValueEditor, HBoxContainer)
private:
	ResourceInspectorProperty* root;
	ResourceEditor* parent;

	Label* label;
	Control* edit;

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const Schema* schema, const Variant& key);

	void propagate_value_update(const Variant& value) override;
	void update_from(const Variant& value) override;

	ValueEditor();
	~ValueEditor();
};

class ResourceInspectorProperty : public EditorProperty {
	GODOT_CLASS(ResourceInspectorProperty, EditorProperty)
private:
	std::unique_ptr<Schema> schema;
	Button* btn;
	Control* editor;
	bool updating = false;

	Variant current_value;
	Array keys_path;

	void _toggle_editor_visibility();

public:
	static void _register_methods();
	void _init();
	void _custom_init(std::unique_ptr<Schema> schema);

	Variant get_current_value();
	void set_current_value(const Variant& value);
	void push_key(const Variant& key);
	void clear_keys();
	void value_mutated();

	void update_property();

	ResourceInspectorProperty();
	~ResourceInspectorProperty();
};

} // namespace godot::structural_inspector
