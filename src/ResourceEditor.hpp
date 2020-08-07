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

class ResourceEditor {
protected:
	ResourceInspectorProperty* root;
	ResourceEditor* parent;
	Variant key;

protected:
	virtual void push_node_key();

public:
	virtual void write(const Variant& value);
	virtual void write(const std::function<auto(const Variant&)->Variant>& mapper);
	virtual void read(const Variant& value) = 0;

	virtual Variant get_key() const;
	virtual void set_key(const Variant& value);
};

class StructEditor : public VBoxContainer, public ResourceEditor {
	GODOT_CLASS(StructEditor, VBoxContainer)
private:
	const StructSchema* schema;
	HBoxContainer* toolbar;
	Label* title;
	VBoxContainer* fields;

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const StructSchema* schema, const Variant& key);

	void read(const Variant& value) override;

	StructEditor();
	~StructEditor();
};

class ArrayEditor : public VBoxContainer, public ResourceEditor {
	GODOT_CLASS(ArrayEditor, VBoxContainer)

	friend class ArraySchema;

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

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceInspectorProperty* root, ResourceEditor* parent, const ArraySchema* schema, const Variant& key);

	void read(const Variant& value) override;

	ArrayEditor();
	~ArrayEditor();
};

class ValueEditor : public HBoxContainer, public ResourceEditor {
	GODOT_CLASS(ValueEditor, HBoxContainer)
private:
	const Schema* schema;
	Control* edit;

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
	Control* editor;
	ResourceEditor* ieditor;
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
	void value_mutated();

	void update_property();

	ResourceInspectorProperty();
	~ResourceInspectorProperty();
};

} // namespace godot::structural_inspector
