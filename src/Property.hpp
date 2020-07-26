#pragma once

#include "Plugin.hpp"

#include <Button.hpp>
#include <Control.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <HBoxContainer.hpp>
#include <Label.hpp>
#include <LineEdit.hpp>
#include <MarginContainer.hpp>
#include <OptionButton.hpp>
#include <RegEx.hpp>
#include <Resource.hpp>
#include <SpinBox.hpp>
#include <VBoxContainer.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace godot::structural_editor {

class NXButton : public Button {
	GODOT_CLASS(NXButton, Button)
private:
	String icon_name;

	void _notification(int what);
	void _apply_icon();

public:
	static void _register_methods();
	void _init();
	void _custom_init(const String& icon_name);

	NXButton();
	~NXButton();
};

class CommonInspectorProperty;

class Schema {
public:
	virtual Variant create_value() const = 0;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const = 0;
	virtual void update_edit(Control* edit, const Variant& data) const = 0;
	virtual EditorProperty* create_property() const = 0;
	virtual ~Schema() = default;
};

class StructSchema : public Schema {
public:
	struct Field {
		String name;
		std::unique_ptr<Schema> def;
	};
	std::vector<Field> fields;

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class ArraySchema : public Schema {
public:
	std::unique_ptr<Schema> element_type;
	int min_elements;
	int max_elements;

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class StringSchema : public Schema {
public:
	std::optional<Ref<RegEx>> pattern;

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class EnumSchema : public Schema {
public:
	struct EnumValue {
		String name;
		int id;
	};
	std::vector<EnumValue> elements;

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class IntSchema : public Schema {
public:
	int min_value = std::numeric_limits<int>::min();
	int max_value = std::numeric_limits<int>::max();

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class FloatSchema : public Schema {
public:
	// real_t will be either float32 or float64 depending on the compilation options, so this will work
	real_t min_value = std::numeric_limits<real_t>::min();
	real_t max_value = std::numeric_limits<real_t>::max();

	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

class BoolSchema : public Schema {
public:
	virtual Variant create_value() const override;
	virtual Control* create_edit(const String* name, CommonInspectorProperty* target, const Array& path) const override;
	virtual void update_edit(Control* edit, const Variant& data) const override;
	virtual EditorProperty* create_property() const override;
};

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
	void set_info(const std::unordered_map<String, Schema*>& data);

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

	Schema* schema = nullptr;
	bool owns_schema = true;
	NodeType schema_id = UNKNOWN;

	bool removing_child = false;
	bool mouse_inside = false;

	ResourceSchemaNode* _make_child();
	bool _is_mouse_inside();

	void _input(Ref<InputEvent> event);
	void _notification(int what);
	void _type_selected(int id);
	void _add_list_item();
	void _toggle_remove_mode();
	void _field_name_set(const String& field_name);
	void _min_value_set(real_t value);
	void _max_value_set(real_t value);
	void _pattern_set(const String& pattern);
	void _child_schema_changed(ResourceSchemaNode* child);
	void _child_clicked(ResourceSchemaNode* child);

	void _enum_name_set(const String& name, Control* child);
	void _enum_id_set(int id, Control* child);

public:
	static void _register_methods();
	void _init();
	void _custom_init(ResourceSchemaInspectorProperty* root, ResourceSchemaNode* parent);

	String get_field_name() const;
	void update_with(const Schema* schema);
	Schema* take_schema();

	ResourceSchemaNode();
	~ResourceSchemaNode();
};

class ResourceSchemaInspectorProperty : public EditorProperty {
	GODOT_CLASS(ResourceSchemaInspectorProperty, EditorProperty)
private:
	std::unordered_map<String, Schema*> properties_schema;
	Button* btn;
	VBoxContainer* properties;
	int selected_idx = -1;
	bool updating = false;

	void _toggle_editor_visibility();
	void _prop_schema_changed(ResourceSchemaNode* node);

public:
	static void _register_methods();
	void _init();

	void add_root_property(int pos = -1);
	void remove_root_property(int pos = -1);
	void emit_something_changed();

	void update_property();

	ResourceSchemaInspectorProperty();
	~ResourceSchemaInspectorProperty();
};

} // namespace godot::structural_editor
