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
#include <functional>
#include <memory>
#include <unordered_map>

namespace godot::structural_inspector {

class ResourceEditor : public ListContainer {
	GODOT_CLASS(ResourceEditor, ListContainer)
protected:
	ResourceInspectorProperty* root;
	ResourceEditor* parent;
	Variant key;

protected:
	virtual void push_node_key();

public:
	static void _register_methods();
	void _init();

	virtual void write(const Variant& value);
	virtual void write(const std::function<auto(const Variant&)->Variant>& mapper);
	virtual void read(const Variant& value);

	virtual Variant get_key() const;
	virtual void set_key(const Variant& value);

	ResourceEditor();
	~ResourceEditor();
};

class StructEditor : public ResourceEditor {
	GODOT_CLASS(StructEditor, ResourceEditor)
private:
	const StructSchema* schema;
	HBoxContainer* toolbar;
	Label* title;
	VBoxContainer* fields;

	void _notification(int what);
	Size2 _get_minimum_size();

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const StructSchema* schema, const Variant& key);

	void read(const Variant& value) override;

	StructEditor();
	~StructEditor();
};

class ArrayEditor : public ResourceEditor {
	GODOT_CLASS(ArrayEditor, ResourceEditor)
private:
	const ArraySchema* schema;
	HBoxContainer* toolbar;
	Label* title;
	EditorIconButton* add;
	EditorIconButton* remove;
	VBoxContainer* elements;
	int64_t selected_idx = -1;

	ResourceEditor* _get_editor_at(int idx);

	void _element_gui_input(Ref<InputEvent> event, Control* element);
	void _add_element();
	void _remove_element();
	void _notification(int what);
	Size2 _get_minimum_size();

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const ArraySchema* schema, const Variant& key);

	void read(const Variant& value) override;

	ArrayEditor();
	~ArrayEditor();
};

class ValueEditor : public ResourceEditor {
	GODOT_CLASS(ValueEditor, ResourceEditor)
private:
	const Schema* schema;
	Control* edit;

	void _notification(int what);
	Size2 _get_minimum_size();

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const Schema* schema, const Variant& key);

	void read(const Variant& value) override;

	ValueEditor();
	~ValueEditor();
};

class ResourceInspectorProperty : public EditorProperty {
	GODOT_CLASS(ResourceInspectorProperty, EditorProperty)
private:
	std::unique_ptr<Schema> schema;
	Button* btn;
	ResourceEditor* editor;
	bool updating = false;

	Variant staging_key;
	Variant staging_value;

	void _toggle_editor_visibility();

public:
	static void _register_methods();
	void _init();
	void _custom_init(std::unique_ptr<Schema> schema);

	Variant get_current_value();
	void set_current_value(const Variant& value);
	void push_key(const Variant& key);
	void clear_keys();

	void update_property();

	ResourceInspectorProperty();
	~ResourceInspectorProperty();
};

} // namespace godot::structural_inspector
