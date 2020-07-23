#pragma once

#include "Schema.hpp"
#include <Button.hpp>
#include <Control.hpp>
#include <EditorProperty.hpp>
#include <Godot.hpp>
#include <HBoxContainer.hpp>
#include <Label.hpp>
#include <RegEx.hpp>
#include <VBoxContainer.hpp>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace godot::structural_editor {

class Schema {
public:
	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const = 0;
	virtual ~Schema() = default;
};

class StructSchema : public Schema {
public:
	std::unordered_map<String, std::unique_ptr<Schema>> fields;

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class ArraySchema : public Schema {
public:
	std::unique_ptr<Schema> element_type;

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class StringSchema : public Schema {
public:
	std::optional<Ref<RegEx>> pattern;

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class EnumSchema : public Schema {
public:
	struct EnumValue {
		String name;
		int id;
	};
	std::vector<EnumValue> elements;

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class IntSchema : public Schema {
public:
	int min_value = INT_MIN;
	int max_value = INT_MAX;

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class RealSchema : public Schema {
public:
	// real_t will be either float32 or float64 depending on the compilation options, so this will work
	real_t min_value = std::numeric_limits<real_t>::min();
	real_t max_value = std::numeric_limits<real_t>::max();

	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class BoolSchema : public Schema {
public:
	virtual Control* create_edit(const String* name, const Object* target, const Array& path) const override;
};

class DefaultEditorBehaviors {
protected:
	void init();

public:
};

class StructEditor : public VBoxContainer {
	GODOT_CLASS(StructEditor, VBoxContainer)
private:
	const Object* notif_target;
	const StructSchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	VBoxContainer* fields;

	Array path;

public:
	static void _register_methods();
	void _init();

	bool has_property_name() const;
	String get_property_name() const;
	void set_property_name(const String& name);

	StructEditor();
	StructEditor(const StructSchema* schema, const Object* notif_target);
	~StructEditor();
};

class ArrayEditor : public VBoxContainer {
	GODOT_CLASS(ArrayEditor, VBoxContainer)
private:
	const Object* notif_target;
	const ArraySchema* schema;

	HBoxContainer* toolbar;
	Label* title;
	Button* add;
	Button* remove;
	VBoxContainer* elements;

	Array path;

	int64_t selected_elm = -1;

	void _element_gui_input(Ref<InputEvent> event, Control* element);
	void _add_element();
	void _remove_element();

public:
	static void _register_methods();
	void _init();

	bool has_property_name() const;
	String get_property_name() const;
	void set_property_name(const String& name);

	ArrayEditor();
	ArrayEditor(const ArraySchema* schema, const Object* notif_target);
	~ArrayEditor();
};

class StructProperty : public EditorProperty {
	GODOT_CLASS(StructProperty, EditorProperty)
private:
	Button* btn;
	StructEditor* editor;

public:
	static void _register_methods();
	void _init();

	void update_value(Variant value, const Array& path);
	void add_array_element(int pos, Variant elm, const Array& path);
	void remove_array_element(int idx, const Array& path);

	void update_property();

	StructProperty();
	~StructProperty();
};

class ArrayProperty : public EditorProperty {
	GODOT_CLASS(ArrayProperty, EditorProperty)
private:
	Button* btn;
	ArrayEditor* editor;

public:
	static void _register_methods();
	void _init();

	void update_value(Variant value, const Array& path);
	void add_array_element(int pos, Variant elm, const Array& path);
	void remove_array_element(int idx, const Array& path);

	void update_property();

	ArrayProperty();
	~ArrayProperty();
};

class ResourceSchemaProperty : public EditorProperty {
	GODOT_CLASS(ResourceSchemaProperty, EditorProperty)
private:
	Button* btn;
	ArrayEditor* properties;

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
