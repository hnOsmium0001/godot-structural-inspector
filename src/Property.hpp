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
#include <Resource.hpp>
#include <SpinBox.hpp>
#include <VBoxContainer.hpp>
#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace godot::structural_inspector {

class ResourceSchema : public Resource {
	GODOT_CLASS(ResourceSchema, Resource)
private:
	Array properties;
	// std::unique_ptr is incompatible with it. Schemas needs to be manually freed.
	mutable std::unordered_map<String, Schema*> info_cache;
	mutable bool initialized = false;

public:
	static void _register_methods();
	void _init();

	const std::unordered_map<String, Schema*>& get_info() const;
	std::unordered_map<String, Schema*> get_info_copy() const;
	void set_info(Iterator<std::pair<String, Schema*>>& data);

	ResourceSchema();
	~ResourceSchema();
};

class StructEditor : public VBoxContainer {
	GODOT_CLASS(StructEditor, VBoxContainer)

	friend class StructSchema;

private:
	CommonInspectorProperty* notif_target;
	const StructSchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	VBoxContainer* fields;

	Array path;

public:
	static void _register_methods();
	void _init();
	void _custom_init(const StructSchema* schema, CommonInspectorProperty* notif_target, const Array& path);

	bool has_property_name() const;
	String get_property_name() const;
	void set_property_name(const String& name);

	StructEditor();
	~StructEditor();
};

class ArrayEditor : public VBoxContainer {
	GODOT_CLASS(ArrayEditor, VBoxContainer)

	friend class ArraySchema;

private:
	CommonInspectorProperty* notif_target;
	const ArraySchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	NXButton* add;
	NXButton* remove;
	VBoxContainer* elements;
	int64_t selected_idx = -1;

	Array path;

	void _element_gui_input(Ref<InputEvent> event, Control* element);
	void _add_element();
	void _remove_element();

public:
	static void _register_methods();
	void _init();
	void _custom_init(const ArraySchema* schema, CommonInspectorProperty* notif_target, const Array& path);

	bool has_property_name() const;
	String get_property_name() const;
	void set_property_name(const String& name);

	ArrayEditor();
	~ArrayEditor();
};

class CommonInspectorProperty : public EditorProperty {
	GODOT_CLASS(CommonInspectorProperty, EditorProperty)
private:
	const Schema* schema;
	Button* btn;
	Control* editor;
	bool updating = false;

	void _toggle_editor_visibility();

public:
	static void _register_methods();
	void _init();
	void _custom_init(const Schema* schema, Control* editor);

	Variant find_object(const Array& path, int distance);
	void update_value(Variant value, const Array& path);
	void add_array_element(int pos, Variant elm, const Array& path);
	void remove_array_element(int pos, const Array& path);

	void update_property();

	CommonInspectorProperty();
	~CommonInspectorProperty();
};

class DefinitionReference {
public:
	std::variant<
			std::pair<std::vector<NamedSchema>*, size_t>,
			std::unique_ptr<Schema>*>
			data;

	DefinitionReference();
	DefinitionReference(std::vector<NamedSchema>* data, size_t idx);
	DefinitionReference(std::unique_ptr<Schema>* data);

	bool has_name();
	/// Safe to call regardless of the state, returns emtpy string if the definition does not have a name
	String get_name();
	/// Only safe to call when has_name() returns true
	String& get_name_ref();
	std::unique_ptr<Schema>& get_schema();
};

class ResourceSchemaInspectorProperty;

class ResourceSchemaNode : public MarginContainer {
	GODOT_CLASS(ResourceSchemaNode, MarginContainer)
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
	// There might be intermediate layout nodes (VBoxContainer, etc.) between this node and its logical parent ResourceSchemaNode
	ResourceSchemaNode* parent;
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

	void _input(Ref<InputEvent> event);
	void _notification(int what);
	void _type_selected(int id);
	void _add_list_item();
	void _toggle_remove_mode();
	void _field_name_set(const String& field_name);
	void _min_value_set(real_t value);
	void _max_value_set(real_t value);
	void _pattern_set(const String& pattern);
	void _child_clicked(ResourceSchemaNode* child);

	void _enum_name_set(const String& name, Control* child);
	void _enum_id_set(int id, Control* child);

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceSchemaInspectorProperty* root, ResourceSchemaNode* parent, DefinitionReference definition);

	String get_field_name() const;
	void set_field_name(const String& name);

	ResourceSchemaNode* get_child_node(int i);
	Schema* get_schema();

	ResourceSchemaNode();
	~ResourceSchemaNode();
};

class ResourceSchemaInspectorProperty : public EditorProperty {
	GODOT_CLASS(ResourceSchemaInspectorProperty, EditorProperty)
private:
	std::vector<NamedSchema> schemas;
	Button* btn;
	VBoxContainer* properties;
	int selected_idx = -1;
	bool updating = false;

	void _toggle_editor_visibility();
	void _prop_clicked(ResourceSchemaNode* node);

public:
	static void _register_methods();
	void _init();

	ResourceSchemaNode* add_root_property_with(const String& name, std::unique_ptr<Schema> schema);
	ResourceSchemaNode* add_root_property();
	void remove_root_property();
	void emit_something_changed();

	void update_property();

	ResourceSchemaInspectorProperty();
	~ResourceSchemaInspectorProperty();
};

} // namespace godot::structural_inspector
