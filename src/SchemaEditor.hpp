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
#include <utility>
#include <variant>
#include <vector>

namespace godot::structural_inspector {

class DefinitionReference {
public:
	std::variant<
			std::pair<std::vector<StructSchema::Field>*, size_t>,
			std::unique_ptr<Schema>*>
			data;

	DefinitionReference();
	DefinitionReference(std::vector<StructSchema::Field>* data, size_t idx);
	DefinitionReference(std::unique_ptr<Schema>* data);

	bool has_name();
	/// Safe to call regardless of the state, returns emtpy string if the definition does not have a name
	String get_name();
	/// Only safe to call when has_name() returns true
	String& get_name_ref();
	std::unique_ptr<Schema>& get_schema();
};

class ResourceSchemaInspectorProperty;
class ResourceSchemaEditor : public MarginContainer {
	GODOT_CLASS(ResourceSchemaEditor, MarginContainer)
private:
	enum NodeType {
		STRUCT,
		ARRAY,
		STRING,
		ENUM,
		INT,
		FLOAT,
		BOOL,
		UNKNOWN,
	};

	ResourceSchemaInspectorProperty* root;
	// There might be intermediate layout nodes (VBoxContainer, etc.) between this node and its logical parent ResourceSchemaEditor
	ResourceSchemaEditor* parent;
	VBoxContainer* contents;

	// Button for changing type of this node
	OptionButton* type_edit;

	HBoxContainer* toolbar;
	// Child of `toolbar`, used by STRUCT, ENUM
	NXButton* add;
	// Child of `toolbar`, used by STRUCT, ENUM
	NXButton* remove;
	// Child of `toolbar`, used by STRUCT, ENUM
	Label* message;

	// Text input for setting name of this node, used when parent is STRUCT
	LineEdit* field_name;
	// Used for ARRAY (should be min/max elements), INT, FLOAT
	SpinBox* min_value;
	SpinBox* max_value;
	// Used for STRING
	LineEdit* pattern;
	// Used for STRUCT, ARRAY, ENUM
	VBoxContainer* list;

	DefinitionReference definition;
	NodeType schema_id = UNKNOWN;

	bool removing_child = false;
	bool mouse_inside = false;

	bool _is_mouse_inside();
	void _select_type(int id, Schema* swap_out);
	Variant _get_key();

	void _input(Ref<InputEvent> event);
	void _notification(int what);
	void _type_selected(int id);
	void _add_list_item();
	void _toggle_remove_mode();
	void _field_name_set(const String& name);
	void _min_value_set(real_t value);
	void _max_value_set(real_t value);
	void _pattern_set(const String& pattern);
	void _child_clicked(ResourceSchemaEditor* child);

	void _enum_name_set(const String& name, Control* child);
	void _enum_id_set(int id, Control* child);

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceSchemaInspectorProperty* root, ResourceSchemaEditor* parent, DefinitionReference definition);

	String get_field_name() const;
	void set_field_name(const String& name);

	ResourceSchemaEditor* get_child_node(int i);
	Schema* get_schema();

	ResourceSchemaEditor();
	~ResourceSchemaEditor();
};

class ResourceSchemaInspectorProperty : public EditorProperty {
	GODOT_CLASS(ResourceSchemaInspectorProperty, EditorProperty)
private:
	std::vector<StructSchema::Field> schemas;
	Button* btn;
	VBoxContainer* properties;
	int selected_idx = -1;
	bool updating = false;

	void _toggle_editor_visibility();
	void _prop_clicked(ResourceSchemaEditor* node);

public:
	static void _register_methods();
	void _init();

	ResourceSchemaEditor* add_root_property_with(const String& name, std::unique_ptr<Schema> schema);
	ResourceSchemaEditor* add_root_property();
	void remove_root_property();
	void emit_something_changed();

	void update_property();

	ResourceSchemaInspectorProperty();
	~ResourceSchemaInspectorProperty();
};

} // namespace godot::structural_inspector
